#include <iostream>
#include <string>
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <fstream>
#include <cmath>
#include "Queue.h"
#include "Vector.h"
#include "booking.h"
#include "priorityqueue.h"
#include "RouteStructures.h"         
#include "DockingStructures.h"      
#include "InputBoxUI.h"               
#include "ViewBookings.h"            
using namespace sf;
using namespace std;

PortLocation portLocations[] = {
    {"AbuDhabi", 1280, 340},
    {"Alexandria", 1092, 286},
    {"Antwerp", 952, 122},
    {"Athens", 1027, 251},
    {"Busan", 1829, 224},
    {"CapeTown", 1027, 849},
    {"Chittagong", 1553, 370},
    {"Colombo", 1491, 503},
    {"Copenhagen", 987, 82},
    {"Doha", 1310, 358},
    {"Dubai", 1301, 392},
    {"Dublin", 850, 105},
    {"Durban", 1109, 814},
    {"Genoa", 954, 174},
    {"Hamburg", 969, 105},
    {"Helsinki", 1136, 40},
    {"HongKong", 1770, 322},
    {"Istanbul", 1121, 244},
    {"Jakarta", 1697, 605},
    {"Jeddah", 1175, 365},
    {"Karachi", 1380, 338},
    {"Lisbon", 816, 230},
    {"London", 888, 117},
    {"LosAngeles", 150, 350},
    {"Manila", 1824, 472},
    {"Marseille", 990, 178},
    {"Melbourne", 1830, 840},
    {"Montreal", 301, 44},
    {"Mumbai", 1436, 401},
    {"NewYork", 278, 174},
    {"Osaka", 1870, 269},
    {"Oslo", 958, 40},
    {"PortLouis", 1317, 736},
    {"Rotterdam", 923, 109},
    {"Shanghai", 1774, 280},
    {"Singapore", 1674, 555},
    {"Stockholm", 1034, 28},
    {"Sydney", 1870, 870},
    {"Tokyo", 1875, 226},
    {"Vancouver", 53, 105}};
const int numLocations = sizeof(portLocations) / sizeof(portLocations[0]);

PortDockingState globalPortDocking[40];

int getPortDockWaitMinutes(int portIndex) 
{
    if (portIndex < 0 || portIndex >= numLocations) 
        return 0;
    return globalPortDocking[portIndex].calculateWaitTimeMinutes();
}

int getPortDockWaitMinutesByName(const string& portName) 
{
    for (int i = 0; i < numLocations; i++) 
    {
        if (portLocations[i].name == portName) 
        {
            return globalPortDocking[i].calculateWaitTimeMinutes();
        }
    }
    return 0;
}

#include "Dijkstra.h"
#include "AStar.h"
#include "ShipPreferences.h"
#include "LinkedListRoute.h"
#include "MainMenu.h"

SoundBuffer clickSoundBuffer;
Sound clickSound;

SoundBuffer transitionSoundBuffer;
Sound transitionSound;

SoundBuffer hornSoundBuffer;
Sound hornSound;

ShipPreferences shipPrefs;

BookingManager bookingManager;

int menuSelection = 0;

void openShipPreferencesPopup(Font &font);


class Maps
{
public:
    Vector<PortNode> ports;

    
    void addPort(string name, int ch)
    {
        PortNode p;
        p.portName = name;
        p.charge = ch;
        ports.push_back(p);
    }

    
    int findPortIndex(string s)
    {
        for (int i = 0; i < ports.getSize(); i++)
        {
            if (ports[i].portName == s)
            {
                return i;
            }
        }
        return -1;
    }

    
    void addRoute(string start, string dest, string depDate, string depTime, string arrTime, int cost, string comp)
    {
        int s = findPortIndex(start);
        int d = findPortIndex(dest);
        if (s == -1 || d == -1)
        {
            cout << "Cannot add Route: Port not found - " << start << " or " << dest << endl;
            return;
        }
        RouteNode *newroute = new RouteNode(start, dest, depDate, depTime, arrTime, cost, comp);
        newroute->next = ports[s].routeHead;
        ports[s].routeHead = newroute;
    }

    
    string normalizeDate(string date)
    {
        if (date.length() == 10)
        {
            if (date[2] == '/' && date[5] == '/')
            {
                return date;
            }
        }

        size_t s1 = date.find('/');
        if (s1 == string::npos)
        {
            return date;
        }

        size_t s2 = date.find('/', s1 + 1);
        if (s2 == string::npos)
        {
            return date;
        }

        string d = date.substr(0, s1);
        string m = date.substr(s1 + 1, s2 - s1 - 1);
        string y = date.substr(s2 + 1);

        if (d.length() == 1)
        {
            d = "0" + d;
        }

        if (m.length() == 1)
        {
            m = "0" + m;
        }

        return d + "/" + m + "/" + y;
    }

    
    bool isDateGreaterOrEqual(string date1, string date2)
    {
        string normDate1 = normalizeDate(date1);
        string normDate2 = normalizeDate(date2);

        int day1 = stoi(normDate1.substr(0, 2));
        int month1 = stoi(normDate1.substr(3, 2));
        int year1 = stoi(normDate1.substr(6, 4));

        int day2 = stoi(normDate2.substr(0, 2));
        int month2 = stoi(normDate2.substr(3, 2));
        int year2 = stoi(normDate2.substr(6, 4));

        if (year1 != year2)
            return year1 > year2;
        if (month1 != month2)
            return month1 > month2;
        return day1 >= day2;
    }

    

    int convertToMinutes(string t)
    {
        int h = stoi(t.substr(0, 2));
        int m = stoi(t.substr(3, 2));
        return h * 60 + m;
    }

    
    int calculateDaysBetween(string date1, string date2)
    {
        int day1 = stoi(date1.substr(0, 2));
        int month1 = stoi(date1.substr(3, 2));
        int year1 = stoi(date1.substr(6, 4));

        int day2 = stoi(date2.substr(0, 2));
        int month2 = stoi(date2.substr(3, 2));
        int year2 = stoi(date2.substr(6, 4));

        int totalDays1 = year1 * 365 + month1 * 30 + day1;
        int totalDays2 = year2 * 365 + month2 * 30 + day2;
        return totalDays2 - totalDays1;
    }
    
    string formatWaitTime(int minutes)
    {
        if (minutes >= 1440)
        {
            int days = minutes / 1440;
            int hours = (minutes % 1440) / 60;
            return to_string(days) + "d " + to_string(hours) + "h";
        }
        else
        {
            int hours = minutes / 60;
            int mins = minutes % 60;
            return to_string(hours) + "h " + to_string(mins) + "m";
        }
    }
    bool checkDirectRoutes(string origin, string des, string date, Vector<int> &path)
    {
        int originindex = findPortIndex(origin);
        int desindex = findPortIndex(des);

        if (originindex == -1 || desindex == -1)
        {
            return false;
        }

        RouteNode *temp = ports[originindex].routeHead;
        while (temp != nullptr)
        {
            if (temp->destinationPort == des && isDateGreaterOrEqual(temp->departureDate, date))
            {
                path.push_back(originindex);
                path.push_back(desindex);
                return true;
            }
            temp = temp->next;
        }
        return false;
    }
    bool checkConnectingRoutes(string origin, string dest, string date, Vector<int> &path, Vector<string> &waitPorts, Vector<string> &waitDurations)
    {
        const int MIN_DOCKING_DELAY = 120;

        int org_index = findPortIndex(origin);
        int des_index = findPortIndex(dest);
        if (org_index == -1 || des_index == -1)
            return false;

        Queue q;
        
        PathWithDate startPath;
        startPath.path.push_back(org_index);
        startPath.currentDate = date;
        startPath.currentTime = "00:00";
        startPath.waitPorts = Vector<string>();
        startPath.waitDurations = Vector<string>();
        q.enqueue(startPath);

        bool found = false;

        while (!q.isEmpty() && !found)
        {
            PathWithDate currentPathWithDate = q.dequeue();
            Vector<int> currentPath = currentPathWithDate.path;
            string currentDate = currentPathWithDate.currentDate;
            string currentTime = currentPathWithDate.currentTime;
            Vector<string> currentWaitPorts = currentPathWithDate.waitPorts;
            Vector<string> currentWaitDurations = currentPathWithDate.waitDurations;

            int currentPort = currentPath[currentPath.getSize() - 1];

            RouteNode *route = ports[currentPort].routeHead;
            while (route != nullptr && !found)
            {
                int nextIndex = findPortIndex(route->destinationPort);
                if (nextIndex == -1)
                {
                    route = route->next;
                    continue;
                }

                
                if (shipPrefs.filterActive)
                {
                    
                    if (!shipPrefs.isCompanyPreferred(route->shippingCompany))
                    {
                        route = route->next;
                        continue;
                    }
                    
                    
                    if (shipPrefs.isPortAvoided(route->destinationPort))
                    {
                        route = route->next;
                        continue;
                    }
                }

                bool alreadyVisited = false;
                for (int i = 0; i < currentPath.getSize(); i++)
                {
                    if (currentPath[i] == nextIndex)
                    {
                        alreadyVisited = true;
                        break;
                    }
                }
                if (alreadyVisited)
                {
                    route = route->next;
                    continue;
                }

                if (!isDateGreaterOrEqual(route->departureDate, currentDate))
                {
                    route = route->next;
                    continue;
                }

                int waitMinutes = 0;
                bool hasWait = false;

                if (route->departureDate == currentDate)
                {
                    int departureMinutes = convertToMinutes(route->departureTime);
                    int currentMinutes = convertToMinutes(currentTime);

                    if (departureMinutes >= currentMinutes + MIN_DOCKING_DELAY)
                    {
                        waitMinutes = departureMinutes - currentMinutes;
                        hasWait = true;
                    }
                    else
                    {
                        route = route->next;
                        continue;
                    }
                }
                else
                {
                    int daysBetween = calculateDaysBetween(currentDate, route->departureDate);
                    waitMinutes = daysBetween * 1440;
                    hasWait = true;

                    if (daysBetween == 0)
                    {
                        int departureMinutes = convertToMinutes(route->departureTime);
                        int currentMinutes = convertToMinutes(currentTime);
                        waitMinutes = (24 * 60 - currentMinutes) + departureMinutes;
                    }
                }

                PathWithDate newPathWithDate;
                newPathWithDate.path = currentPath;
                newPathWithDate.path.push_back(nextIndex);
                newPathWithDate.currentDate = route->departureDate;
                newPathWithDate.currentTime = route->departureTime;

                newPathWithDate.waitPorts = currentWaitPorts;
                newPathWithDate.waitDurations = currentWaitDurations;

                if (hasWait && currentPath.getSize() > 0)
                {
                    string waitPortName = portLocations[currentPort].name;
                    string waitDuration = formatWaitTime(waitMinutes);
                    newPathWithDate.waitPorts.push_back(waitPortName);
                    newPathWithDate.waitDurations.push_back(waitDuration);
                }

                if (nextIndex == des_index)
                {
                    path = newPathWithDate.path;
                    waitPorts = newPathWithDate.waitPorts;
                    waitDurations = newPathWithDate.waitDurations;
                    found = true;
                    break;
                }
                else
                {
                    q.enqueue(newPathWithDate);
                }

                route = route->next;
            }
        }

        return found;
    }

  
    struct ValidPath {
        Vector<int> path;
        Vector<string> waitPorts;
        Vector<string> waitDurations;
        Vector<RouteNode*> routes;
        int totalCost;
        int totalTime;
        bool isDirect;
        string label;
    };
    
    bool findAllValidPaths(string origin, string dest, string date, Vector<ValidPath> &allPaths, int maxPaths = 5)
    {
        allPaths.clear();
        const int MIN_DOCKING_DELAY = 120;
        const int MAX_PATH_LENGTH = 6;  
        
        int org_index = findPortIndex(origin);
        int des_index = findPortIndex(dest);
        if (org_index == -1 || des_index == -1)
            return false;

        
        RouteNode *directRoute = ports[org_index].routeHead;
        while (directRoute != nullptr && allPaths.getSize() < maxPaths)
        {
            if (directRoute->destinationPort == dest && isDateGreaterOrEqual(directRoute->departureDate, date))
            {
                
                bool passesFilter = true;
                if (shipPrefs.filterActive)
                {
                    if (!shipPrefs.isCompanyPreferred(directRoute->shippingCompany))
                        passesFilter = false;
                    if (shipPrefs.isPortAvoided(dest))
                        passesFilter = false;
                }
                
                if (passesFilter)
                {
                    ValidPath vp;
                    vp.path.push_back(org_index);
                    vp.path.push_back(des_index);
                    vp.isDirect = true;
                    vp.totalCost = directRoute->cost;
                    vp.routes.push_back(directRoute);
                    
                    int travelTime = convertToMinutes(directRoute->arrivalTime) - convertToMinutes(directRoute->departureTime);
                    if (travelTime < 0) travelTime += 24 * 60;
                    vp.totalTime = travelTime;
                    vp.label = "Direct (" + directRoute->departureDate + ")";
                    
                    
                    bool duplicate = false;
                    for (int i = 0; i < allPaths.getSize(); i++) {
                        if (allPaths[i].isDirect && allPaths[i].routes.getSize() > 0 &&
                            allPaths[i].routes[0]->departureDate == directRoute->departureDate &&
                            allPaths[i].routes[0]->departureTime == directRoute->departureTime) {
                            duplicate = true;
                            break;
                        }
                    }
                    if (!duplicate)
                        allPaths.push_back(vp);
                }
            }
            directRoute = directRoute->next;
        }

        
        Queue q;
        PathWithDate startPath;
        startPath.path.push_back(org_index);
        startPath.currentDate = date;
        startPath.currentTime = "00:00";
        startPath.waitPorts = Vector<string>();
        startPath.waitDurations = Vector<string>();
        q.enqueue(startPath);

        int iterations = 0;
        const int MAX_ITERATIONS = 1000;  

        while (!q.isEmpty() && allPaths.getSize() < maxPaths && iterations < MAX_ITERATIONS)
        {
            iterations++;
            PathWithDate currentPathWithDate = q.dequeue();
            Vector<int> currentPath = currentPathWithDate.path;
            string currentDate = currentPathWithDate.currentDate;
            string currentTime = currentPathWithDate.currentTime;
            Vector<string> currentWaitPorts = currentPathWithDate.waitPorts;
            Vector<string> currentWaitDurations = currentPathWithDate.waitDurations;

            
            if (currentPath.getSize() >= MAX_PATH_LENGTH)
                continue;

            int currentPort = currentPath[currentPath.getSize() - 1];

            RouteNode *route = ports[currentPort].routeHead;
            while (route != nullptr && allPaths.getSize() < maxPaths)
            {
                int nextIndex = findPortIndex(route->destinationPort);
                if (nextIndex == -1)
                {
                    route = route->next;
                    continue;
                }

                
                if (shipPrefs.filterActive)
                {
                    if (!shipPrefs.isCompanyPreferred(route->shippingCompany))
                    {
                        route = route->next;
                        continue;
                    }
                    if (shipPrefs.isPortAvoided(route->destinationPort))
                    {
                        route = route->next;
                        continue;
                    }
                }

                
                bool alreadyVisited = false;
                for (int i = 0; i < currentPath.getSize(); i++)
                {
                    if (currentPath[i] == nextIndex)
                    {
                        alreadyVisited = true;
                        break;
                    }
                }
                if (alreadyVisited)
                {
                    route = route->next;
                    continue;
                }

                
                if (!isDateGreaterOrEqual(route->departureDate, currentDate))
                {
                    route = route->next;
                    continue;
                }

                int waitMinutes = 0;
                bool hasWait = false;

                if (route->departureDate == currentDate)
                {
                    int departureMinutes = convertToMinutes(route->departureTime);
                    int currentMinutes = convertToMinutes(currentTime);

                    if (departureMinutes >= currentMinutes + MIN_DOCKING_DELAY)
                    {
                        waitMinutes = departureMinutes - currentMinutes;
                        hasWait = true;
                    }
                    else
                    {
                        route = route->next;
                        continue;
                    }
                }
                else
                {
                    int daysBetween = calculateDaysBetween(currentDate, route->departureDate);
                    waitMinutes = daysBetween * 1440;
                    hasWait = true;

                    if (daysBetween == 0)
                    {
                        int departureMinutes = convertToMinutes(route->departureTime);
                        int currentMinutes = convertToMinutes(currentTime);
                        waitMinutes = (24 * 60 - currentMinutes) + departureMinutes;
                    }
                }

                PathWithDate newPathWithDate;
                newPathWithDate.path = currentPath;
                newPathWithDate.path.push_back(nextIndex);
                newPathWithDate.currentDate = route->departureDate;
                newPathWithDate.currentTime = route->arrivalTime;

                newPathWithDate.waitPorts = currentWaitPorts;
                newPathWithDate.waitDurations = currentWaitDurations;

                if (hasWait && currentPath.getSize() > 1)
                {
                    string waitPortName = portLocations[currentPort].name;
                    string waitDuration = formatWaitTime(waitMinutes);
                    newPathWithDate.waitPorts.push_back(waitPortName);
                    newPathWithDate.waitDurations.push_back(waitDuration);
                }

                if (nextIndex == des_index)
                {
                    
                    if (newPathWithDate.path.getSize() > 2)
                    {
                        ValidPath vp;
                        vp.path = newPathWithDate.path;
                        vp.waitPorts = newPathWithDate.waitPorts;
                        vp.waitDurations = newPathWithDate.waitDurations;
                        vp.isDirect = false;
                        vp.totalCost = 0;
                        vp.totalTime = 0;
                        
                        
                        for (int j = 0; j < vp.path.getSize() - 1; j++) {
                            string fromPort = portLocations[vp.path[j]].name;
                            string toPort = portLocations[vp.path[j + 1]].name;
                            int fromIndex = findPortIndex(fromPort);
                            
                            RouteNode* r = ports[fromIndex].routeHead;
                            while (r != nullptr) {
                                if (r->destinationPort == toPort) {
                                    vp.routes.push_back(r);
                                    vp.totalCost += r->cost;
                                    int travelTime = convertToMinutes(r->arrivalTime) - convertToMinutes(r->departureTime);
                                    if (travelTime < 0) travelTime += 24 * 60;
                                    vp.totalTime += travelTime;
                                    break;
                                }
                                r = r->next;
                            }
                        }
                        
                        
                        string intermediates = "";
                        for (int j = 1; j < vp.path.getSize() - 1; j++) {
                            if (j > 1) intermediates += " → ";
                            intermediates += portLocations[vp.path[j]].name;
                        }
                        vp.label = "Via " + intermediates;
                        
                        
                        bool duplicate = false;
                        for (int i = 0; i < allPaths.getSize(); i++) {
                            if (allPaths[i].path.getSize() == vp.path.getSize()) {
                                bool same = true;
                                for (int j = 0; j < vp.path.getSize(); j++) {
                                    if (allPaths[i].path[j] != vp.path[j]) {
                                        same = false;
                                        break;
                                    }
                                }
                                if (same) {
                                    duplicate = true;
                                    break;
                                }
                            }
                        }
                        if (!duplicate)
                            allPaths.push_back(vp);
                    }
                }
                else
                {
                    q.enqueue(newPathWithDate);
                }

                route = route->next;
            }
        }

        return !allPaths.empty();
    }

    int findPortLocationIndex(const string &portName)
    {
        for (int i = 0; i < numLocations; i++)
        {
            if (portLocations[i].name == portName)
            {
                return i;
            }
        }
        return -1;
    }

    struct RouteResult
    {
        Vector<int> directPath;
        Vector<int> connectingPath;
        Vector<string> waitPorts;
        Vector<string> waitDurations;
        bool directFound;
        bool connectingFound;
        string resultText;
        Color resultColor;
        string errorDetails;
        int totalDirectCost;
        int totalConnectingCost;
        int totalDirectTime; 
        int totalConnectingTime; 
    };

    bool findAndDisplayRoute(string origin, string destination, string date,
                             Vector<int> &currentPath, Vector<string> &waitPorts,
                             Vector<string> &waitDurations, string &resultText, Color &resultColor, string &errorDetails,
                             Vector<int> &directPath, bool &hasBothRoutes)
    {
        currentPath.clear();
        waitPorts.clear();
        waitDurations.clear();
        directPath.clear();
        hasBothRoutes = false;
        errorDetails = "";

        if (origin.empty() || destination.empty() || date.empty())
        {
            resultText = "Input Error!";
            resultColor = Color::Red;
            errorDetails = "Please fill all fields: Departure Port, Destination Port, and Date.";
            return false;
        }

        int originIndex = findPortIndex(origin);
        int destIndex = findPortIndex(destination);

        if (originIndex == -1 && destIndex == -1)
        {
            resultText = "Port Error!";
            resultColor = Color::Red;
            errorDetails = "Both ports not found: '" + origin + "' and '" + destination + "'";
            return false;
        }
        else if (originIndex == -1)
        {
            resultText = "Port Error!";
            resultColor = Color::Red;
            errorDetails = "Departure port not found: '" + origin + "'";
            return false;
        }
        else if (destIndex == -1)
        {
            resultText = "Port Error!";
            resultColor = Color::Red;
            errorDetails = "Destination port not found: '" + destination + "'";
            return false;
        }

        string normalizedDate = normalizeDate(date);

        if (normalizedDate.length() != 10 || normalizedDate[2] != '/' || normalizedDate[5] != '/')
        {
            resultText = "Date Format Error!";
            resultColor = Color::Red;
            errorDetails = "Invalid date format. Please use D/M/YYYY or DD/MM/YYYY (e.g., 9/12/2024 or 09/12/2024)";
            return false;
        }
        
        try
        {
            int day = stoi(normalizedDate.substr(0, 2));
            int month = stoi(normalizedDate.substr(3, 2));
            int year = stoi(normalizedDate.substr(6, 4));

            if (day < 1 || day > 31 || month < 1 || month > 12 || year < 2024)
            {
                resultText = "Date Error!";
                resultColor = Color::Red;
                errorDetails = "Invalid date. Please check day (1-31), month (1-12), and year (>=2024)";
                return false;
            }
        }
        catch (...)
        {
            resultText = "Date Error!";
            resultColor = Color::Red;
            errorDetails = "Invalid date format. Please use numbers for day, month, and year.";
            return false;
        }

        Vector<int> tempDirectPath;

        bool directFound = checkDirectRoutes(origin, destination, normalizedDate, tempDirectPath);
        bool connectingFound = checkConnectingRoutes(origin, destination, normalizedDate, currentPath, waitPorts, waitDurations);
        
        if (directFound && connectingFound)
        {
            directPath = tempDirectPath;
            hasBothRoutes = true;
            resultText = "Multiple routes available!";
            resultColor = Color(100, 200, 255);
            errorDetails = "Found both direct and connecting routes. Displaying both options for comparison.";
            return true;
        }
        else if (directFound)
        {
            currentPath = tempDirectPath;
            resultText = "Direct route found!";
            resultColor = Color::Green;
            errorDetails = "Direct connection available. No intermediate stops.";
            return true;
        }
        else if (connectingFound)
        {
            resultText = "Connecting route found!";
            resultColor = Color::Green;
            errorDetails = "Route with " + to_string(currentPath.getSize() - 2) + " intermediate stops and " +
                           to_string(waitPorts.getSize()) + " wait points.";
            return true;
        }
        else
        {
            resultText = "No route found!";
            resultColor = Color::Red;

            RouteNode *routesFromOrigin = ports[originIndex].routeHead;
            bool hasRoutesFromOrigin = (routesFromOrigin != nullptr);

            if (!hasRoutesFromOrigin)
            {
                errorDetails = "No routes available from " + origin + " on " + date;
            }
            else
            {
                bool hasRoutesOnDate = false;
                RouteNode *temp = routesFromOrigin;
                while (temp != nullptr)
                {
                    if (isDateGreaterOrEqual(temp->departureDate, date))
                    {
                        hasRoutesOnDate = true;
                        break;
                    }
                    temp = temp->next;
                }

                if (!hasRoutesOnDate)
                {
                    errorDetails = "No routes available from " + origin + " on " + date + ". Try different date.";
                }
                else
                {
                    errorDetails = "No connecting route found from " + origin + " to " + destination +
                                   " on " + date + ". The ports may not be connected.";
                }
            }
            return false;
        }
    }

    
    void makeButton(RenderWindow &window, Font &font, string txt, float yPos, float centerX, Vector2f mouse, bool &hovered)
    {
        RectangleShape box({500, 110});
        box.setOrigin(250, 55);
        box.setPosition(centerX, yPos);
        box.setFillColor(Color(0, 60, 110, 200));
        box.setOutlineThickness(4);
        box.setOutlineColor(Color(100, 200, 255));

        if (box.getGlobalBounds().contains(mouse))
        {
            hovered = true;
            box.setFillColor(Color(0, 120, 200, 220));
            box.setScale(1.07f, 1.07f);
        }
        else
            hovered = false;

        Text label(txt, font, 48);
        label.setStyle(Text::Bold);
        FloatRect lb = label.getLocalBounds();
        label.setOrigin(lb.width / 2, lb.height / 2);
        label.setPosition(centerX, yPos - 5);

        window.draw(box);
        window.draw(label);
    }

    
    RouteNode *findRouteBetweenPorts(Maps &graph, string fromPort, string toPort)
    {
        int fromIndex = graph.findPortIndex(fromPort);
        if (fromIndex == -1)
            return nullptr;

        RouteNode *route = graph.ports[fromIndex].routeHead;
        while (route != nullptr)
        {
            if (route->destinationPort == toPort)
            {
                return route;
            }
            route = route->next;
        }
        return nullptr;
    }

    
    int calculateJourneyTime(string departureTime, string arrivalTime)
    {
        int departureInMinutes = convertTimeToMinutes(departureTime);
        int arrivalInMinutes = convertTimeToMinutes(arrivalTime);

        if (arrivalInMinutes >= departureInMinutes)
            return arrivalInMinutes - departureInMinutes;
        else
            return (24 * 60 - departureInMinutes) + arrivalInMinutes;
    }

    
    int convertTimeToMinutes(string timeString)
    {
        int hourValue = (timeString[0] - '0') * 10 + (timeString[1] - '0');
        int minuteValue = (timeString[3] - '0') * 10 + (timeString[4] - '0');
        return hourValue * 60 + minuteValue;
    }

    
    string formatTime(int totalMinutes)
    {
        int days = totalMinutes / (24 * 60);
        int hours = (totalMinutes % (24 * 60)) / 60;
        int minutes = totalMinutes % 60;

        if (days > 0)
        {
            return to_string(days) + "d " + to_string(hours) + "h " + to_string(minutes) + "m";
        }
        else
        {
            return to_string(hours) + "h " + to_string(minutes) + "m";
        }
    }

    
    void drawLabel(RenderWindow &window, Font &font, int idx, Color fillColor, unsigned int size, float alpha = 255.0f)
    {
        Text label;
        label.setFont(font);
        label.setString(portLocations[idx].name);
        label.setCharacterSize(size);
        label.setStyle(Text::Bold);
        label.setOutlineThickness(1.f);
        label.setOutlineColor(Color(0, 0, 0, alpha));

        Color textColor = fillColor;
        textColor.a = alpha;
        label.setFillColor(textColor);

        label.setPosition(portLocations[idx].x, portLocations[idx].y - 25.f);
        window.draw(label);
    }

    
    int openShortestPathMenu(Font &font)
    {
        RenderWindow window(VideoMode(1920, 1080), "Shortest Path - Algorithm Selection", Style::Fullscreen);
        window.setFramerateLimit(60);

        Clock clock;
        float time = 0.0f;
        int selection = 0;

        
        RectangleShape bgGradient1(Vector2f(1920, 360));
        bgGradient1.setFillColor(Color(5, 15, 35));
        bgGradient1.setPosition(0, 0);

        RectangleShape bgGradient2(Vector2f(1920, 360));
        bgGradient2.setFillColor(Color(10, 25, 50));
        bgGradient2.setPosition(0, 360);

        RectangleShape bgGradient3(Vector2f(1920, 360));
        bgGradient3.setFillColor(Color(15, 35, 65));
        bgGradient3.setPosition(0, 720);

        
        Vector<CircleShape> particles;
        Vector<float> particleSpeeds;
        Vector<float> particlePhases;
        for (int i = 0; i < 80; i++)
        {
            CircleShape particle(1.f + rand() % 3);
            int colorChoice = rand() % 3;
            if (colorChoice == 0)
                particle.setFillColor(Color(100, 255, 180, 80 + rand() % 100)); 
            else if (colorChoice == 1)
                particle.setFillColor(Color(255, 215, 100, 80 + rand() % 100)); 
            else
                particle.setFillColor(Color(150, 200, 255, 60 + rand() % 80)); 
            particle.setPosition(rand() % 1920, rand() % 1080);
            particles.push_back(particle);
            particleSpeeds.push_back(0.2f + (rand() % 100) / 200.0f);
            particlePhases.push_back((rand() % 100) / 10.0f);
        }

        
        RectangleShape panelOuterGlow(Vector2f(920, 620));
        panelOuterGlow.setFillColor(Color(50, 150, 200, 30));
        panelOuterGlow.setPosition(500, 230);

        RectangleShape mainPanel(Vector2f(900, 600));
        mainPanel.setFillColor(Color(10, 30, 60, 245));
        mainPanel.setOutlineThickness(3);
        mainPanel.setOutlineColor(Color(80, 180, 220, 200));
        mainPanel.setPosition(510, 240);

        
        RectangleShape topBar(Vector2f(900, 8));
        topBar.setPosition(510, 240);

        
        Text mainTitle;
        mainTitle.setFont(font);
        mainTitle.setString("OCEANROUTE NAVIGATOR");
        mainTitle.setCharacterSize(52);
        mainTitle.setStyle(Text::Bold);
        mainTitle.setFillColor(Color(200, 240, 255));
        mainTitle.setOutlineThickness(2);
        mainTitle.setOutlineColor(Color(0, 50, 100));
        mainTitle.setLetterSpacing(3);
        FloatRect mainTitleBounds = mainTitle.getLocalBounds();
        mainTitle.setPosition((1920 - mainTitleBounds.width) / 2, 270);

        
        Text subtitle;
        subtitle.setFont(font);
        subtitle.setString("Select Your Pathfinding Algorithm");
        subtitle.setCharacterSize(24);
        subtitle.setFillColor(Color(150, 200, 230));
        subtitle.setStyle(Text::Italic);
        FloatRect subtitleBounds = subtitle.getLocalBounds();
        subtitle.setPosition((1920 - subtitleBounds.width) / 2, 340);

        
        RectangleShape divider(Vector2f(400, 2));
        divider.setFillColor(Color(100, 180, 220, 150));
        divider.setPosition(760, 385);

        
        
        RectangleShape dijkstraGlow(Vector2f(380, 320));
        dijkstraGlow.setFillColor(Color(0, 180, 100, 0));
        dijkstraGlow.setPosition(555, 415);

        RectangleShape dijkstraCard(Vector2f(360, 300));
        dijkstraCard.setFillColor(Color(0, 60, 40, 240));
        dijkstraCard.setOutlineThickness(3);
        dijkstraCard.setOutlineColor(Color(0, 200, 120));
        dijkstraCard.setPosition(565, 425);

        
        RectangleShape dijkstraAccent(Vector2f(360, 6));
        dijkstraAccent.setFillColor(Color(0, 255, 150));
        dijkstraAccent.setPosition(565, 425);

        
        CircleShape dijkstraIconOuter(35.f);
        dijkstraIconOuter.setFillColor(Color(0, 200, 120, 60));
        dijkstraIconOuter.setOutlineThickness(3);
        dijkstraIconOuter.setOutlineColor(Color(0, 255, 150));
        dijkstraIconOuter.setOrigin(35, 35);
        dijkstraIconOuter.setPosition(745, 490);

        CircleShape dijkstraIconInner(20.f);
        dijkstraIconInner.setFillColor(Color(0, 255, 150));
        dijkstraIconInner.setOrigin(20, 20);
        dijkstraIconInner.setPosition(745, 490);

        
        RectangleShape dijkstraLine1(Vector2f(25, 3));
        dijkstraLine1.setFillColor(Color(0, 60, 40));
        dijkstraLine1.setOrigin(12.5f, 1.5f);
        dijkstraLine1.setPosition(745, 490);

        RectangleShape dijkstraLine2(Vector2f(25, 3));
        dijkstraLine2.setFillColor(Color(0, 60, 40));
        dijkstraLine2.setOrigin(12.5f, 1.5f);
        dijkstraLine2.setPosition(745, 490);
        dijkstraLine2.setRotation(90);

        Text dijkstraLabel;
        dijkstraLabel.setFont(font);
        dijkstraLabel.setString("DIJKSTRA");
        dijkstraLabel.setCharacterSize(32);
        dijkstraLabel.setFillColor(Color(0, 255, 150));
        dijkstraLabel.setStyle(Text::Bold);
        dijkstraLabel.setLetterSpacing(2);
        FloatRect dijkLabelBounds = dijkstraLabel.getLocalBounds();
        dijkstraLabel.setPosition(745 - dijkLabelBounds.width / 2, 535);

        Text dijkstraSubtitle;
        dijkstraSubtitle.setFont(font);
        dijkstraSubtitle.setString("Classic Algorithm");
        dijkstraSubtitle.setCharacterSize(14);
        dijkstraSubtitle.setFillColor(Color(100, 200, 150));
        FloatRect dijkSubBounds = dijkstraSubtitle.getLocalBounds();
        dijkstraSubtitle.setPosition(745 - dijkSubBounds.width / 2, 575);

        
        RectangleShape dijkstraDescBox(Vector2f(320, 80));
        dijkstraDescBox.setFillColor(Color(0, 40, 30, 200));
        dijkstraDescBox.setOutlineThickness(1);
        dijkstraDescBox.setOutlineColor(Color(0, 150, 100, 150));
        dijkstraDescBox.setPosition(585, 605);

        Text dijkstraDesc;
        dijkstraDesc.setFont(font);
        dijkstraDesc.setString("Guaranteed shortest path\nOptimal for weighted graphs\nBased on edge costs");
        dijkstraDesc.setCharacterSize(14);
        dijkstraDesc.setFillColor(Color(180, 230, 200));
        dijkstraDesc.setLineSpacing(1.3f);
        dijkstraDesc.setPosition(600, 615);

        Text dijkstraClick;
        dijkstraClick.setFont(font);
        dijkstraClick.setString("[ CLICK TO SELECT ]");
        dijkstraClick.setCharacterSize(12);
        dijkstraClick.setFillColor(Color(0, 200, 120, 180));
        FloatRect dijkClickBounds = dijkstraClick.getLocalBounds();
        dijkstraClick.setPosition(745 - dijkClickBounds.width / 2, 700);


        RectangleShape astarGlow(Vector2f(380, 320));
        astarGlow.setFillColor(Color(200, 150, 0, 0));
        astarGlow.setPosition(985, 415);

        RectangleShape astarCard(Vector2f(360, 300));
        astarCard.setFillColor(Color(60, 45, 0, 240));
        astarCard.setOutlineThickness(3);
        astarCard.setOutlineColor(Color(255, 200, 50));
        astarCard.setPosition(995, 425);

        
        RectangleShape astarAccent(Vector2f(360, 6));
        astarAccent.setFillColor(Color(255, 215, 0));
        astarAccent.setPosition(995, 425);

        
        CircleShape astarIconOuter(35.f);
        astarIconOuter.setFillColor(Color(200, 150, 0, 60));
        astarIconOuter.setOutlineThickness(3);
        astarIconOuter.setOutlineColor(Color(255, 215, 0));
        astarIconOuter.setOrigin(35, 35);
        astarIconOuter.setPosition(1175, 490);

        
        CircleShape astarStar(18.f, 5); 
        astarStar.setFillColor(Color(255, 215, 0));
        astarStar.setOrigin(18, 18);
        astarStar.setPosition(1175, 490);

        Text astarLabel;
        astarLabel.setFont(font);
        astarLabel.setString("A* STAR");
        astarLabel.setCharacterSize(32);
        astarLabel.setFillColor(Color(255, 215, 0));
        astarLabel.setStyle(Text::Bold);
        astarLabel.setLetterSpacing(2);
        FloatRect astarLabelBounds = astarLabel.getLocalBounds();
        astarLabel.setPosition(1175 - astarLabelBounds.width / 2, 535);

        Text astarSubtitle;
        astarSubtitle.setFont(font);
        astarSubtitle.setString("Intelligent Search");
        astarSubtitle.setCharacterSize(14);
        astarSubtitle.setFillColor(Color(220, 180, 80));
        FloatRect astarSubBounds = astarSubtitle.getLocalBounds();
        astarSubtitle.setPosition(1175 - astarSubBounds.width / 2, 575);

        
        RectangleShape astarDescBox(Vector2f(320, 80));
        astarDescBox.setFillColor(Color(50, 40, 10, 200));
        astarDescBox.setOutlineThickness(1);
        astarDescBox.setOutlineColor(Color(200, 150, 50, 150));
        astarDescBox.setPosition(1015, 605);

        Text astarDesc;
        astarDesc.setFont(font);
        astarDesc.setString("Uses heuristic guidance\nFaster pathfinding\nIntelligent route planning");
        astarDesc.setCharacterSize(14);
        astarDesc.setFillColor(Color(240, 220, 180));
        astarDesc.setLineSpacing(1.3f);
        astarDesc.setPosition(1030, 615);

        
        Text astarClick;
        astarClick.setFont(font);
        astarClick.setString("[ CLICK TO SELECT ]");
        astarClick.setCharacterSize(12);
        astarClick.setFillColor(Color(255, 200, 50, 180));
        FloatRect astarClickBounds = astarClick.getLocalBounds();
        astarClick.setPosition(1175 - astarClickBounds.width / 2, 700);

        
        Text instruction;
        instruction.setFont(font);
        instruction.setString("Press ESC to return to main menu");
        instruction.setCharacterSize(16);
        instruction.setFillColor(Color(150, 150, 180));
        FloatRect instrBounds = instruction.getLocalBounds();
        instruction.setPosition((1920 - instrBounds.width) / 2, 770);

        
        Text comparisonText;
        comparisonText.setFont(font);
        comparisonText.setString("Dijkstra: Guarantees optimal path  |  A*: Faster with heuristic optimization");
        comparisonText.setCharacterSize(14);
        comparisonText.setFillColor(Color(120, 160, 200));
        FloatRect compBounds = comparisonText.getLocalBounds();
        comparisonText.setPosition((1920 - compBounds.width) / 2, 800);

        bool dijkstraHovered = false;
        bool astarHovered = false;

        while (window.isOpen() && selection == 0)
        {
            Event event;
            while (window.pollEvent(event))
            {
                if (event.type == Event::Closed)
                {
                    window.close();
                    return 0;
                }

                if (event.type == Event::MouseButtonPressed)
                {
                    Vector2f mousePos = window.mapPixelToCoords(Mouse::getPosition(window));
                    if (dijkstraCard.getGlobalBounds().contains(mousePos))
                    {
                        clickSound.play();
                        selection = 1; 
                        window.close();
                    }
                    if (astarCard.getGlobalBounds().contains(mousePos))
                    {
                        clickSound.play();
                        selection = 2; 
                        window.close();
                    }
                }

                if (event.type == Event::KeyPressed && event.key.code == Keyboard::Escape)
                {
                    window.close();
                    return 0;
                }
            }

            float deltaTime = clock.restart().asSeconds();
            time += deltaTime;
            Vector2f mousePos = window.mapPixelToCoords(Mouse::getPosition(window));

            
            for (int i = 0; i < particles.getSize(); i++)
            {
                float newY = particles[i].getPosition().y - particleSpeeds[i];
                float newX = particles[i].getPosition().x + sin(time * 0.5f + particlePhases[i]) * 0.3f;
                if (newY < -10)
                {
                    newY = 1090;
                    newX = rand() % 1920;
                }
                particles[i].setPosition(newX, newY);
            }

            
            int r = 80 + 40 * sin(time * 0.5f);
            int g = 180 + 40 * sin(time * 0.5f + 2.0f);
            int b = 220;
            topBar.setFillColor(Color(r, g, b));

            
            dijkstraHovered = dijkstraCard.getGlobalBounds().contains(mousePos);
            astarHovered = astarCard.getGlobalBounds().contains(mousePos);

            if (dijkstraHovered)
            {
                dijkstraCard.setFillColor(Color(0, 80, 50, 250));
                dijkstraCard.setOutlineThickness(4);
                dijkstraCard.setOutlineColor(Color(0, 255, 150));
                dijkstraGlow.setFillColor(Color(0, 200, 100, 60));
                dijkstraAccent.setFillColor(Color(100, 255, 180));
                dijkstraClick.setFillColor(Color(0, 255, 150));
            }
            else
            {
                dijkstraCard.setFillColor(Color(0, 60, 40, 240));
                dijkstraCard.setOutlineThickness(3);
                dijkstraCard.setOutlineColor(Color(0, 200, 120));
                dijkstraGlow.setFillColor(Color(0, 180, 100, 0));
                dijkstraAccent.setFillColor(Color(0, 255, 150));
                dijkstraClick.setFillColor(Color(0, 200, 120, 180));
            }

            
            if (astarHovered)
            {
                astarCard.setFillColor(Color(80, 60, 0, 250));
                astarCard.setOutlineThickness(4);
                astarCard.setOutlineColor(Color(255, 230, 100));
                astarGlow.setFillColor(Color(255, 200, 0, 60));
                astarAccent.setFillColor(Color(255, 240, 150));
                astarClick.setFillColor(Color(255, 230, 100));
            }
            else
            {
                astarCard.setFillColor(Color(60, 45, 0, 240));
                astarCard.setOutlineThickness(3);
                astarCard.setOutlineColor(Color(255, 200, 50));
                astarGlow.setFillColor(Color(200, 150, 0, 0));
                astarAccent.setFillColor(Color(255, 215, 0));
                astarClick.setFillColor(Color(255, 200, 50, 180));
            }

            
            float pulse = 0.5f + 0.5f * sin(time * 3.0f);
            float slowPulse = 0.5f + 0.5f * sin(time * 1.5f);

            dijkstraIconOuter.setScale(0.9f + 0.15f * pulse, 0.9f + 0.15f * pulse);
            dijkstraIconInner.setScale(0.85f + 0.2f * slowPulse, 0.85f + 0.2f * slowPulse);

            astarIconOuter.setScale(0.9f + 0.15f * pulse, 0.9f + 0.15f * pulse);
            astarStar.setRotation(time * 20.0f); 
            astarStar.setScale(0.85f + 0.2f * slowPulse, 0.85f + 0.2f * slowPulse);

            
            window.clear(Color(5, 15, 30));

            
            window.draw(bgGradient1);
            window.draw(bgGradient2);
            window.draw(bgGradient3);

            
            for (int i = 0; i < particles.getSize(); i++)
            {
                window.draw(particles[i]);
            }

            
            window.draw(panelOuterGlow);
            window.draw(mainPanel);
            window.draw(topBar);
            window.draw(divider);

            
            window.draw(mainTitle);
            window.draw(subtitle);

            
            window.draw(dijkstraGlow);
            window.draw(dijkstraCard);
            window.draw(dijkstraAccent);
            window.draw(dijkstraIconOuter);
            window.draw(dijkstraIconInner);
            window.draw(dijkstraLine1);
            window.draw(dijkstraLine2);
            window.draw(dijkstraLabel);
            window.draw(dijkstraSubtitle);
            window.draw(dijkstraDescBox);
            window.draw(dijkstraDesc);
            window.draw(dijkstraClick);

            
            window.draw(astarGlow);
            window.draw(astarCard);
            window.draw(astarAccent);
            window.draw(astarIconOuter);
            window.draw(astarStar);
            window.draw(astarLabel);
            window.draw(astarSubtitle);
            window.draw(astarDescBox);
            window.draw(astarDesc);
            window.draw(astarClick);

            
            window.draw(instruction);
            window.draw(comparisonText);

            window.display();
        }

        return selection;
    }

    
    void showShortestPathAStar(Maps &graph, Font &font)
    {
        RenderWindow window(VideoMode(1920, 1080), "A* Algorithm - OceanRoute Navigator");
        window.setFramerateLimit(60);
        
        
        View fixedView(FloatRect(0, 0, 1920, 1080));
        window.setView(fixedView);

        Texture mapTexture;
        if (!mapTexture.loadFromFile("pics/map2.png"))
        {
            cout << "Map image not loaded!" << endl;
            return;
        }
        Sprite mapSprite(mapTexture);
        mapSprite.setColor(Color(180, 220, 255, 255));

        if (!font.loadFromFile("Roboto.ttf"))
        {
            cout << "Font load failed!" << endl;
            return;
        }

        struct RouteAnimation
        {
            Vector<int> path;
            Vector<string> waitPorts;
            Vector<string> waitDurations;
            Color routeColor;
            string routeType;
            int currentSegment;
            float segmentProgress;
            bool isPaused;
            float pauseTimer;
            bool isComplete;
            Vector<RouteNode *> pathRoutes;
            Vector<int> waitTimes;
            int totalCost;
            int totalTime;
        };

        Vector<RouteAnimation> activeAnimations;
        float animationSpeed = 0.8f;
        const float SEGMENT_PAUSE = 1.5f;
        string userSearchedDate = "";  

        Clock clock;
        float time = 0.0f;

        
        
        RectangleShape panelGlow(Vector2f(320.f, 440.f));
        panelGlow.setFillColor(Color(255, 180, 0, 40));
        panelGlow.setPosition(Vector2f(10.f, 545.f));

        
        RectangleShape inputPanel(Vector2f(300.f, 420.f));
        inputPanel.setFillColor(Color(15, 25, 45, 245));
        inputPanel.setOutlineThickness(3.f);
        inputPanel.setOutlineColor(Color(255, 200, 0, 200));
        inputPanel.setPosition(Vector2f(20.f, 555.f));

        
        RectangleShape panelTopAccent(Vector2f(300.f, 6.f));
        panelTopAccent.setFillColor(Color(255, 215, 0));
        panelTopAccent.setPosition(Vector2f(20.f, 555.f));

        
        RectangleShape headerBg(Vector2f(300.f, 45.f));
        headerBg.setFillColor(Color(60, 45, 0, 200));
        headerBg.setPosition(Vector2f(20.f, 561.f));

        
        Text panelTitle;
        panelTitle.setFont(font);
        panelTitle.setString("A* PATHFINDER");
        panelTitle.setCharacterSize(20);
        panelTitle.setFillColor(Color(255, 215, 0));
        panelTitle.setStyle(Text::Bold);
        panelTitle.setLetterSpacing(2);
        FloatRect titleBounds = panelTitle.getLocalBounds();
        panelTitle.setPosition(170.f - titleBounds.width / 2.f, 568.f);

        
        CircleShape starIcon(12.f, 5);
        starIcon.setFillColor(Color(255, 215, 0));
        starIcon.setOrigin(12.f, 12.f);
        starIcon.setPosition(50.f, 583.f);

        
        class InputBox
        {
        public:
            RectangleShape box;
            RectangleShape iconBg;
            Text text;
            Text label;
            Text icon;
            string inputString;
            bool isActive;

            InputBox(Font &font, string lbl, string iconChar, Vector2f position)
            {
                label.setFont(font);
                label.setString(lbl);
                label.setCharacterSize(13);
                label.setFillColor(Color(255, 220, 150));
                label.setStyle(Text::Bold);
                label.setPosition(position.x + 5, position.y - 22);

                
                iconBg.setSize(Vector2f(30, 38));
                iconBg.setFillColor(Color(80, 60, 0, 200));
                iconBg.setPosition(position.x, position.y);

                icon.setFont(font);
                icon.setString(iconChar);
                icon.setCharacterSize(14);
                icon.setFillColor(Color(255, 215, 0));
                icon.setStyle(Text::Bold);
                icon.setPosition(position.x + 8, position.y + 10);

                box.setSize(Vector2f(230, 38));
                box.setFillColor(Color(20, 30, 50, 240));
                box.setOutlineThickness(2);
                box.setOutlineColor(Color(180, 140, 50));
                box.setPosition(position.x + 30, position.y);

                text.setFont(font);
                text.setCharacterSize(14);
                text.setFillColor(Color::White);
                text.setPosition(position.x + 40, position.y + 10);

                inputString = "";
                isActive = false;
            }

            void handleEvent(Event &event, RenderWindow &window)
            {
                Vector2f mousePos = window.mapPixelToCoords(Mouse::getPosition(window));

                if (event.type == Event::MouseButtonPressed)
                {
                    if (box.getGlobalBounds().contains(mousePos))
                    {
                        isActive = true;
                        box.setOutlineColor(Color(255, 215, 0));
                        box.setFillColor(Color(30, 40, 60, 255));
                    }
                    else
                    {
                        isActive = false;
                        box.setOutlineColor(Color(180, 140, 50));
                        box.setFillColor(Color(20, 30, 50, 240));
                    }
                }

                if (event.type == Event::TextEntered && isActive)
                {
                    if (event.text.unicode == '\b')
                    {
                        if (!inputString.empty())
                            inputString.pop_back();
                    }
                    else if (event.text.unicode < 128 && event.text.unicode != '\r' && event.text.unicode != '\t')
                    {
                        inputString += static_cast<char>(event.text.unicode);
                    }
                    text.setString(inputString);
                }
            }

            void draw(RenderWindow &window)
            {
                window.draw(iconBg);
                window.draw(icon);
                window.draw(label);
                window.draw(box);
                window.draw(text);
            }

            string getText() { return inputString; }
            void setText(string newText)
            {
                inputString = newText;
                text.setString(inputString);
            }
        };

        
        InputBox departureInput(font, "Departure Port", ">", Vector2f(30, 635));
        InputBox destinationInput(font, "Destination Port", "X", Vector2f(30, 700));
        InputBox dateInput(font, "Date (DD/MM/YYYY)", "#", Vector2f(30, 765));

        
        
        RectangleShape sectionDivider(Vector2f(260, 2));
        sectionDivider.setFillColor(Color(255, 200, 0, 100));
        sectionDivider.setPosition(30, 820);

        Text modeLabel;
        modeLabel.setFont(font);
        modeLabel.setString("OPTIMIZATION MODE");
        modeLabel.setCharacterSize(11);
        modeLabel.setFillColor(Color(200, 180, 100));
        modeLabel.setLetterSpacing(1.5f);
        modeLabel.setPosition(75, 828);

        
        RectangleShape costButton(Vector2f(260, 40));
        costButton.setFillColor(Color(100, 80, 0, 220));
        costButton.setOutlineThickness(2);
        costButton.setOutlineColor(Color(255, 200, 50));
        costButton.setPosition(30, 850);

        CircleShape costIcon(12.f, 6);
        costIcon.setFillColor(Color(255, 215, 0));
        costIcon.setOrigin(12.f, 12.f);
        costIcon.setPosition(55, 870);

        Text costButtonText("CHEAPEST ROUTE", font, 13);
        costButtonText.setFillColor(Color::White);
        costButtonText.setStyle(Text::Bold);
        costButtonText.setPosition(80, 858);

        Text costSubText("Minimize total cost", font, 10);
        costSubText.setFillColor(Color(200, 180, 100));
        costSubText.setPosition(80, 874);

        
        RectangleShape timeButton(Vector2f(260, 40));
        timeButton.setFillColor(Color(80, 60, 0, 220));
        timeButton.setOutlineThickness(2);
        timeButton.setOutlineColor(Color(200, 150, 50));
        timeButton.setPosition(30, 900);

        CircleShape timeIcon(12.f, 3);
        timeIcon.setFillColor(Color(255, 180, 0));
        timeIcon.setOrigin(12.f, 12.f);
        timeIcon.setPosition(55, 920);

        Text timeButtonText("FASTEST ROUTE", font, 13);
        timeButtonText.setFillColor(Color::White);
        timeButtonText.setStyle(Text::Bold);
        timeButtonText.setPosition(80, 908);

        Text timeSubText("Minimize travel time", font, 10);
        timeSubText.setFillColor(Color(180, 150, 80));
        timeSubText.setPosition(80, 924);

        bool useCost = true;

        
        RectangleShape calcButtonGlow(Vector2f(270, 55));
        calcButtonGlow.setFillColor(Color(255, 180, 0, 0));
        calcButtonGlow.setPosition(25, 947);

        RectangleShape calculateButton(Vector2f(260, 50));
        calculateButton.setFillColor(Color(200, 150, 0));
        calculateButton.setOutlineThickness(3);
        calculateButton.setOutlineColor(Color(255, 220, 100));
        calculateButton.setPosition(30, 950);

        
        CircleShape calcStar(8.f, 5);
        calcStar.setFillColor(Color::White);
        calcStar.setOrigin(8.f, 8.f);
        calcStar.setPosition(60, 975);

        Text calculateButtonText("FIND A* PATH", font, 18);
        calculateButtonText.setFillColor(Color::White);
        calculateButtonText.setStyle(Text::Bold);
        calculateButtonText.setLetterSpacing(1.5f);
        calculateButtonText.setPosition(85, 965);

        
        RectangleShape shipPrefsButton(Vector2f(220, 50));
        shipPrefsButton.setFillColor(Color(120, 80, 0, 230));
        shipPrefsButton.setOutlineThickness(3);
        shipPrefsButton.setOutlineColor(Color(255, 180, 50));
        shipPrefsButton.setPosition(1580, 80);

        Text shipPrefsText("SHIP PREFS", font, 16);
        shipPrefsText.setFillColor(Color::White);
        shipPrefsText.setStyle(Text::Bold);
        shipPrefsText.setPosition(1720, 32);

        
        CircleShape prefsIndicator(8);
        prefsIndicator.setPosition(1870, 35);

        
        Text statusMessage("", font, 14);
        statusMessage.setPosition(30, 985);
        statusMessage.setFillColor(Color::Yellow);

        
        RouteNode *hoveredRoute = nullptr;
        Vector<float> portPulse;
        for (int i = 0; i < graph.ports.getSize(); i++)
        {
            portPulse.push_back(0.0f);
        }

        
        auto addRouteAnimation = [&](Vector<int> path, string userDate, Color color, string type)
        {
            RouteAnimation newAnim;
            newAnim.path = path;
            newAnim.waitPorts = Vector<string>();
            newAnim.waitDurations = Vector<string>();
            newAnim.waitTimes = Vector<int>();
            newAnim.routeColor = color;
            newAnim.routeType = type;
            newAnim.currentSegment = 0;
            newAnim.segmentProgress = 0.0f;
            newAnim.isPaused = false;
            newAnim.pauseTimer = 0.0f;
            newAnim.isComplete = false;

            newAnim.pathRoutes.clear();
            newAnim.totalCost = 0;
            newAnim.totalTime = 0;

            if (path.getSize() > 1)
            {
                DijkstraAlgorithm dijkstra; 
                string currentDate = graph.normalizeDate(userDate);
                string currentTime = "00:00";

                for (int i = 0; i < path.getSize() - 1; i++)
                {
                    string fromPort = portLocations[path[i]].name;
                    string toPort = portLocations[path[i + 1]].name;

                    int fromIndex = graph.findPortIndex(fromPort);
                    RouteNode *route = graph.ports[fromIndex].routeHead;

                    RouteNode *bestRoute = nullptr;
                    int minWaitTime = INT_MAX;
                    int bestTravelTime = 0;

                    while (route != nullptr)
                    {
                        if (route->destinationPort == toPort)
                        {
                            if (dijkstra.isDateGreaterOrEqual(route->departureDate, currentDate))
                            {
                                int waitMinutes = 0;

                                if (route->departureDate == currentDate)
                                {
                                    int departureMin = dijkstra.toMinutes(route->departureTime);
                                    int currentMin = dijkstra.toMinutes(currentTime);

                                    if (departureMin >= currentMin + 120)
                                    {
                                        waitMinutes = departureMin - currentMin;
                                    }
                                    else
                                    {
                                        route = route->next;
                                        continue;
                                    }
                                }
                                else
                                {
                                    int daysBetween = dijkstra.calculateDaysBetween(currentDate, route->departureDate);
                                    int departureMin = dijkstra.toMinutes(route->departureTime);
                                    int currentMin = dijkstra.toMinutes(currentTime);

                                    waitMinutes = (24 * 60 - currentMin) +
                                                  (daysBetween - 1) * 24 * 60 +
                                                  departureMin;
                                }

                                int travelTime = dijkstra.calculateTravelTime(route->departureTime, route->arrivalTime);

                                if (useCost)
                                {
                                    if (bestRoute == nullptr || route->cost < bestRoute->cost)
                                    {
                                        bestRoute = route;
                                        minWaitTime = waitMinutes;
                                        bestTravelTime = travelTime;
                                    }
                                }
                                else
                                {
                                    int totalSegmentTime = waitMinutes + travelTime;
                                    if (bestRoute == nullptr || totalSegmentTime < (minWaitTime + bestTravelTime))
                                    {
                                        bestRoute = route;
                                        minWaitTime = waitMinutes;
                                        bestTravelTime = travelTime;
                                    }
                                }
                            }
                        }
                        route = route->next;
                    }

                    if (bestRoute != nullptr)
                    {
                        newAnim.pathRoutes.push_back(bestRoute);
                        newAnim.totalCost += bestRoute->cost;

                        if (minWaitTime > 0)
                        {
                            newAnim.waitTimes.push_back(minWaitTime);
                            newAnim.totalTime += minWaitTime;

                            if (i == 0)
                            {
                                newAnim.waitPorts.push_back(fromPort + " (initial)");
                            }
                            else
                            {
                                newAnim.waitPorts.push_back(fromPort);
                            }
                            newAnim.waitDurations.push_back(graph.formatWaitTime(minWaitTime));
                        }

                        newAnim.totalTime += bestTravelTime;

                        currentDate = bestRoute->departureDate;
                        currentTime = bestRoute->arrivalTime;
                    }
                    else
                    {
                        cout << "Error: No route found from " << fromPort << " to " << toPort << endl;
                        return;
                    }
                }
            }

            activeAnimations.push_back(newAnim);
        };

        while (window.isOpen())
        {
            Event event;
            Vector2f mousePos = window.mapPixelToCoords(Mouse::getPosition(window));
            float deltaTime = clock.restart().asSeconds();
            time += deltaTime;

            while (window.pollEvent(event))
            {
                if (event.type == Event::Closed)
                    window.close();

                if (event.type == Event::KeyPressed && event.key.code == Keyboard::Escape)
                    window.close();

                
                departureInput.handleEvent(event, window);
                destinationInput.handleEvent(event, window);
                dateInput.handleEvent(event, window);

                if (event.type == Event::MouseButtonPressed)
                {
                    Vector2f mousePos = window.mapPixelToCoords(Mouse::getPosition(window));

                    
                    if (costButton.getGlobalBounds().contains(mousePos))
                    {
                        useCost = true;
                        activeAnimations.clear();
                        statusMessage.setString("Mode: Cheapest Route (Minimize Cost)");
                        statusMessage.setFillColor(Color::Yellow);
                    }
                    if (timeButton.getGlobalBounds().contains(mousePos))
                    {
                        useCost = false;
                        activeAnimations.clear();
                        statusMessage.setString("Mode: Fastest Route (Minimize Time)");
                        statusMessage.setFillColor(Color::Cyan);
                    }

                    
                    if (shipPrefsButton.getGlobalBounds().contains(mousePos))
                    {
                        openShipPreferencesPopup(font);
                    }

                    
                    if (calculateButton.getGlobalBounds().contains(mousePos))
                    {
                        string departureText = departureInput.getText();
                        string destinationText = destinationInput.getText();
                        string dateText = dateInput.getText();

                        
                        if (departureText.empty() || destinationText.empty() || dateText.empty())
                        {
                            statusMessage.setString("Error: Please fill all fields!");
                            statusMessage.setFillColor(Color::Red);
                        }
                        else
                        {
                            string normalizedDate = graph.normalizeDate(dateText);

                            
                            if (normalizedDate.length() != 10 || normalizedDate[2] != '/' || normalizedDate[5] != '/')
                            {
                                statusMessage.setString("Error: Invalid date format! Use DD/MM/YYYY");
                                statusMessage.setFillColor(Color::Red);
                            }
                            else
                            {
                                
                                try
                                {
                                    int day = stoi(normalizedDate.substr(0, 2));
                                    int month = stoi(normalizedDate.substr(3, 2));
                                    int year = stoi(normalizedDate.substr(6, 4));

                                    if (day < 1 || day > 31 || month < 1 || month > 12 || year < 2024)
                                    {
                                        statusMessage.setString("Error: Invalid date values!");
                                        statusMessage.setFillColor(Color::Red);
                                    }
                                    else
                                    {
                                        int originIdx = graph.findPortIndex(departureText);
                                        int destIdx = graph.findPortIndex(destinationText);

                                        if (originIdx == -1 && destIdx == -1)
                                        {
                                            statusMessage.setString("Error: Both ports not found!");
                                            statusMessage.setFillColor(Color::Red);
                                        }
                                        else if (originIdx == -1)
                                        {
                                            statusMessage.setString("Error: Departure port not found!");
                                            statusMessage.setFillColor(Color::Red);
                                        }
                                        else if (destIdx == -1)
                                        {
                                            statusMessage.setString("Error: Destination port not found!");
                                            statusMessage.setFillColor(Color::Red);
                                        }
                                        else
                                        {
                                            activeAnimations.clear();
                                            AStar astar;
                                            Vector<int> path;

                                            
                                            if (useCost)
                                            {
                                                path = astar.findAStarPathCost(graph.ports, originIdx, destIdx, normalizedDate, &shipPrefs);
                                                statusMessage.setString("Calculating cheapest A* path...");
                                                statusMessage.setFillColor(Color::Yellow);
                                            }
                                            else
                                            {
                                                path = astar.findAStarPathTime(graph.ports, originIdx, destIdx, normalizedDate, &shipPrefs);
                                                statusMessage.setString("Calculating fastest A* path...");
                                                statusMessage.setFillColor(Color::Cyan);
                                            }

                                            if (path.getSize() > 1)
                                            {
                                                userSearchedDate = normalizedDate;  
                                                Color routeColor = Color(255, 200, 0); 
                                                string routeType = "A* " + string(useCost ? "Cheapest" : "Fastest");
                                                addRouteAnimation(path, normalizedDate, routeColor, routeType);

                                                
                                                string mode = useCost ? "Cheapest" : "Fastest";
                                                statusMessage.setString("A* " + mode + " path found! (" +
                                                                        to_string(path.getSize()) + " ports)");
                                                statusMessage.setFillColor(Color::Green);
                                            }
                                            else
                                            {
                                                statusMessage.setString("No A* path found for given ports and date!");
                                                statusMessage.setFillColor(Color::Red);
                                            }
                                        }
                                    }
                                }
                                catch (...)
                                {
                                    statusMessage.setString("Error: Invalid date values!");
                                    statusMessage.setFillColor(Color::Red);
                                }
                            }
                        }
                    }
                }
            }

            
            for (int i = 0; i < portPulse.getSize(); i++)
            {
                if (portPulse[i] > 0)
                {
                    portPulse[i] -= 0.05f;
                    if (portPulse[i] < 0)
                        portPulse[i] = 0;
                }
            }

            
            for (int animIndex = 0; animIndex < activeAnimations.getSize(); animIndex++)
            {
                RouteAnimation &anim = activeAnimations[animIndex];

                if (!anim.isComplete)
                {
                    if (!anim.isPaused)
                    {
                        anim.segmentProgress += deltaTime * animationSpeed;

                        if (anim.segmentProgress >= 1.0f)
                        {
                            anim.segmentProgress = 1.0f;
                            anim.isPaused = true;
                            anim.pauseTimer = 0.0f;
                        }
                    }
                    else
                    {
                        anim.pauseTimer += deltaTime;
                        if (anim.pauseTimer >= SEGMENT_PAUSE)
                        {
                            anim.currentSegment++;
                            anim.segmentProgress = 0.0f;
                            anim.isPaused = false;

                            if (anim.currentSegment >= anim.path.getSize() - 1)
                            {
                                anim.isComplete = true;
                            }
                        }
                    }
                }
            }

            
            float panelPulse = 0.5f + 0.5f * sin(time * 2.0f);
            panelTopAccent.setFillColor(Color(255, 200 + 55 * panelPulse, 0));
            starIcon.setRotation(time * 30.0f);
            calcStar.setRotation(-time * 50.0f);

            
            window.clear(Color(10, 20, 40));
            window.draw(mapSprite);

            
            window.draw(panelGlow);
            window.draw(inputPanel);
            window.draw(panelTopAccent);
            window.draw(headerBg);
            window.draw(panelTitle);
            window.draw(starIcon);

            
            departureInput.draw(window);
            destinationInput.draw(window);
            dateInput.draw(window);

            
            window.draw(sectionDivider);
            window.draw(modeLabel);

            
            if (useCost)
            {
                costButton.setFillColor(Color(180, 140, 0, 255));
                costButton.setOutlineColor(Color(255, 220, 100));
                costIcon.setFillColor(Color(255, 240, 150));
                timeButton.setFillColor(Color(60, 45, 0, 200));
                timeButton.setOutlineColor(Color(150, 120, 50));
                timeIcon.setFillColor(Color(180, 140, 50));
            }
            else
            {
                costButton.setFillColor(Color(60, 45, 0, 200));
                costButton.setOutlineColor(Color(150, 120, 50));
                costIcon.setFillColor(Color(180, 140, 50));
                timeButton.setFillColor(Color(180, 140, 0, 255));
                timeButton.setOutlineColor(Color(255, 200, 100));
                timeIcon.setFillColor(Color(255, 220, 100));
            }

            
            if (costButton.getGlobalBounds().contains(mousePos))
            {
                costButton.setOutlineColor(Color(255, 255, 200));
                costButton.setFillColor(useCost ? Color(200, 160, 0, 255) : Color(100, 80, 0, 240));
            }

            if (timeButton.getGlobalBounds().contains(mousePos))
            {
                timeButton.setOutlineColor(Color(255, 240, 150));
                timeButton.setFillColor(!useCost ? Color(200, 160, 0, 255) : Color(100, 80, 0, 240));
            }

            window.draw(costButton);
            window.draw(costIcon);
            window.draw(costButtonText);
            window.draw(costSubText);
            window.draw(timeButton);
            window.draw(timeIcon);
            window.draw(timeButtonText);
            window.draw(timeSubText);

            
            if (calculateButton.getGlobalBounds().contains(mousePos))
            {
                calcButtonGlow.setFillColor(Color(255, 200, 0, 60));
                calculateButton.setFillColor(Color(255, 200, 0));
                calculateButton.setOutlineColor(Color(255, 240, 150));
            }
            else
            {
                calcButtonGlow.setFillColor(Color(255, 180, 0, 0));
                calculateButton.setFillColor(Color(200, 150, 0));
                calculateButton.setOutlineColor(Color(255, 220, 100));
            }
            window.draw(calcButtonGlow);
            window.draw(calculateButton);
            window.draw(calcStar);
            window.draw(calculateButtonText);

            
            window.draw(statusMessage);

            
            
            for (int i = 0; i < numLocations; i++)
            {
                
                string currentPortName = portLocations[i].name;
                bool isAvoided = shipPrefs.isPortAvoided(currentPortName);
                bool isInSubgraph = !isAvoided; 
                
                
                float fadeFactor = 1.0f;
                if (shipPrefs.filterActive && isAvoided) {
                    fadeFactor = 0.1f; 
                }

                
                if (shipPrefs.filterActive && isAvoided) {
                    CircleShape avoidCircle(25.f);
                    avoidCircle.setOrigin(25, 25);
                    avoidCircle.setFillColor(Color(255, 0, 0, 40)); 
                    avoidCircle.setOutlineThickness(3);
                    avoidCircle.setOutlineColor(Color(255, 50, 50, 200)); 
                    avoidCircle.setPosition(portLocations[i].x, portLocations[i].y);
                    window.draw(avoidCircle);
                }

                CircleShape glow(16.f);
                glow.setOrigin(16, 16);
                
                glow.setFillColor(Color(100, 180, 255, static_cast<Uint8>(100 * fadeFactor)));
                glow.setPosition(portLocations[i].x, portLocations[i].y);
                window.draw(glow);

                float pulseScale = 1.0f + 0.2f * portPulse[i];
                CircleShape dot(8.f * pulseScale);
                dot.setOrigin(8.f * pulseScale, 8.f * pulseScale);

                bool inPath = false;
                for (int animIndex = 0; animIndex < activeAnimations.getSize(); animIndex++)
                {
                    RouteAnimation &anim = activeAnimations[animIndex];
                    for (int j = 0; j < anim.path.getSize(); j++)
                    {
                        if (anim.path[j] == i)
                        {
                            inPath = true;
                            break;
                        }
                    }
                    if (inPath)
                        break;
                }

                if (inPath)
                {
                    dot.setFillColor(Color(255, 200, 0)); 
                }
                else if (isAvoided && shipPrefs.filterActive)
                {
                    
                    dot.setFillColor(Color(150, 50, 50, 80));
                }
                else
                {
                    float gentlePulse = 0.5f + 0.5f * sin(time * 2.0f + i);
                    dot.setFillColor(Color(0, static_cast<Uint8>(150 * gentlePulse * fadeFactor), static_cast<Uint8>(255 * fadeFactor)));
                }

                dot.setOutlineColor(Color(255, 255, 255, static_cast<Uint8>(255 * fadeFactor)));
                dot.setOutlineThickness(1.5f);
                dot.setPosition(portLocations[i].x, portLocations[i].y);
                window.draw(dot);

                
                if (isAvoided && shipPrefs.filterActive) {
                    
                    Text bigX("X", font, 28);
                    bigX.setFillColor(Color(255, 50, 50, 255)); 
                    bigX.setStyle(Text::Bold);
                    bigX.setPosition(portLocations[i].x - 10, portLocations[i].y - 18);
                    window.draw(bigX);
                    
                    
                    Text avoidedLabel("AVOIDED", font, 10);
                    avoidedLabel.setFillColor(Color(255, 100, 100, 220));
                    avoidedLabel.setStyle(Text::Bold);
                    avoidedLabel.setPosition(portLocations[i].x - 22, portLocations[i].y + 12);
                    window.draw(avoidedLabel);
                }

                
                PortDockingState& portDock = globalPortDocking[i];
                int dockedCount = portDock.getOccupiedDocks();
                int queueCount = portDock.getQueueLength();
                
                if (dockedCount > 0 || queueCount > 0) {
                    float baseX = portLocations[i].x + 15;
                    float baseY = portLocations[i].y - 10;
                    
                    
                    for (int slot = 0; slot < portDock.maxDocks; slot++) {
                        RectangleShape miniSlot(Vector2f(6, 6));
                        miniSlot.setPosition(baseX + slot * 8, baseY);
                        
                        
                        bool occupied = false;
                        for (int d = 0; d < dockedCount; d++) {
                            if (portDock.dockedShips[d].dockSlot == slot) {
                                occupied = true;
                                break;
                            }
                        }
                        
                        if (occupied) {
                            miniSlot.setFillColor(Color(0, 200, 100)); 
                        } else {
                            miniSlot.setFillColor(Color(60, 60, 80)); 
                        }
                        miniSlot.setOutlineThickness(1);
                        miniSlot.setOutlineColor(Color(100, 100, 120));
                        window.draw(miniSlot);
                    }
                    
                    
                    if (queueCount > 0) {
                        float queueStartX = baseX + portDock.maxDocks * 8 + 4;
                        float dashLen = 3.f;
                        for (int d = 0; d < 4; d++) {
                            RectangleShape dash(Vector2f(dashLen, 2));
                            dash.setPosition(queueStartX + d * (dashLen + 2), baseY + 2);
                            dash.setFillColor(Color(255, 180, 0, 180));
                            window.draw(dash);
                        }
                        
                        
                        CircleShape qBadge(7);
                        qBadge.setFillColor(Color(255, 100, 50));
                        qBadge.setOutlineThickness(1);
                        qBadge.setOutlineColor(Color::White);
                        qBadge.setPosition(queueStartX + 20, baseY - 3);
                        window.draw(qBadge);
                        
                        Text qText(to_string(queueCount), font, 10);
                        qText.setFillColor(Color::White);
                        qText.setStyle(Text::Bold);
                        qText.setPosition(queueStartX + 23, baseY - 2);
                        window.draw(qText);
                    }
                    
                    
                    if (dockedCount > 0) {
                        CircleShape dBadge(7);
                        dBadge.setFillColor(Color(0, 150, 100));
                        dBadge.setOutlineThickness(1);
                        dBadge.setOutlineColor(Color::White);
                        dBadge.setPosition(baseX - 12, baseY - 3);
                        window.draw(dBadge);
                        
                        Text dText(to_string(dockedCount), font, 10);
                        dText.setFillColor(Color::White);
                        dText.setStyle(Text::Bold);
                        dText.setPosition(baseX - 9, baseY - 2);
                        window.draw(dText);
                    }
                }
            }

            
            for (int animIndex = 0; animIndex < activeAnimations.getSize(); animIndex++)
            {
                RouteAnimation &anim = activeAnimations[animIndex];

                if (anim.path.getSize() > 1)
                {
                    for (int i = 0; i < anim.path.getSize() - 1; i++)
                    {
                        int startIdx = anim.path[i];
                        int endIdx = anim.path[i + 1];
                        Vector2f start(portLocations[startIdx].x, portLocations[startIdx].y);
                        Vector2f end(portLocations[endIdx].x, portLocations[endIdx].y);

                        float length = sqrt(pow(end.x - start.x, 2) + pow(end.y - start.y, 2));
                        float angle = atan2(end.y - start.y, end.x - start.x) * 180 / 3.14159f;

                        if (i == anim.currentSegment && !anim.isComplete)
                        {
                            float currentLength = length * anim.segmentProgress;

                            RectangleShape currentLine(Vector2f(currentLength, 6.0f));
                            currentLine.setPosition(start);
                            currentLine.setRotation(angle);
                            currentLine.setFillColor(anim.routeColor);
                            window.draw(currentLine);

                            RectangleShape currentGlow(Vector2f(currentLength, 12.0f));
                            currentGlow.setPosition(start);
                            currentGlow.setRotation(angle);
                            currentGlow.setFillColor(Color(anim.routeColor.r, anim.routeColor.g, anim.routeColor.b, 100));
                            window.draw(currentGlow);

                            if (!anim.isComplete)
                            {
                                Vector2f currentPos = start + (end - start) * anim.segmentProgress;
                                float pulse = 0.7f + 0.3f * sin(time * 8.0f);

                                CircleShape endGlow(15.f * pulse);
                                endGlow.setFillColor(Color(anim.routeColor.r, anim.routeColor.g, anim.routeColor.b, 150));
                                endGlow.setOrigin(15.f * pulse, 15.f * pulse);
                                endGlow.setPosition(currentPos);
                                window.draw(endGlow);

                                CircleShape endDot(8.f);
                                endDot.setFillColor(Color::Yellow);
                                endDot.setOutlineColor(Color::White);
                                endDot.setOutlineThickness(2);
                                endDot.setOrigin(8.f, 8.f);
                                endDot.setPosition(currentPos);
                                window.draw(endDot);
                            }
                        }
                        else if (i < anim.currentSegment || anim.isComplete)
                        {
                            RectangleShape completedLine(Vector2f(length, 5.0f));
                            completedLine.setPosition(start);
                            completedLine.setRotation(angle);
                            completedLine.setFillColor(anim.routeColor);
                            window.draw(completedLine);
                        }
                        else
                        {
                            RectangleShape upcomingLine(Vector2f(length, 3.0f));
                            upcomingLine.setPosition(start);
                            upcomingLine.setRotation(angle);
                            upcomingLine.setFillColor(Color(anim.routeColor.r, anim.routeColor.g, anim.routeColor.b, 100));
                            window.draw(upcomingLine);
                        }
                    }

                    
                    if (animIndex == 0)
                    {
                        RectangleShape routeInfoBox(Vector2f(500, 220));
                        routeInfoBox.setFillColor(Color(0, 30, 60, 200));
                        routeInfoBox.setOutlineThickness(3);
                        routeInfoBox.setOutlineColor(anim.routeColor);
                        routeInfoBox.setPosition(250, 850);
                        window.draw(routeInfoBox);

                        Text routeTitle;
                        routeTitle.setFont(font);
                        routeTitle.setString(anim.routeType + " - $" + to_string(anim.totalCost) +
                                             " - " + graph.formatTime(anim.totalTime));
                        routeTitle.setCharacterSize(18);
                        routeTitle.setFillColor(anim.routeColor);
                        routeTitle.setStyle(Text::Bold);
                        routeTitle.setPosition(260, 860);
                        window.draw(routeTitle);

                        string pathStr = "";
                        for (int i = 0; i < anim.path.getSize(); i++)
                        {
                            pathStr += portLocations[anim.path[i]].name;
                            if (i < anim.path.getSize() - 1)
                                pathStr += " → ";
                        }

                        Text pathText;
                        pathText.setFont(font);
                        pathText.setString(pathStr);
                        pathText.setCharacterSize(14);
                        pathText.setFillColor(Color::White);
                        pathText.setPosition(260, 890);
                        window.draw(pathText);

                        
                        if (anim.waitPorts.getSize() > 0)
                        {
                            Text waitText;
                            waitText.setFont(font);
                            waitText.setCharacterSize(12);
                            waitText.setFillColor(Color::Yellow);
                            waitText.setPosition(260, 920);

                            string waitInfo = "Waiting at: ";
                            for (int i = 0; i < anim.waitPorts.getSize(); i++)
                            {
                                if (i > 0)
                                    waitInfo += ", ";
                                waitInfo += anim.waitPorts[i] + "(" + anim.waitDurations[i] + ")";
                            }
                            waitText.setString(waitInfo);
                            window.draw(waitText);
                        }

                        if (anim.isComplete)
                        {
                            Text completeText;
                            completeText.setFont(font);
                            completeText.setString("A* Route Complete!");
                            completeText.setCharacterSize(14);
                            completeText.setFillColor(Color::Green);
                            completeText.setPosition(260, 945);
                            window.draw(completeText);
                        }
                    }
                }
            }

            
            hoveredRoute = nullptr;
            int hoveredSegment = -1;
            int hoveredAnimIndex = -1;

            if (activeAnimations.getSize() > 0)
            {
                for (int animIndex = 0; animIndex < activeAnimations.getSize(); animIndex++)
                {
                    RouteAnimation &anim = activeAnimations[animIndex];
                    for (int i = 0; i < anim.path.getSize() - 1; i++)
                    {
                        Vector2f start(portLocations[anim.path[i]].x, portLocations[anim.path[i]].y);
                        Vector2f end(portLocations[anim.path[i + 1]].x, portLocations[anim.path[i + 1]].y);

                        float length = sqrt(pow(end.x - start.x, 2) + pow(end.y - start.y, 2));

                        float lineLength = length;
                        Vector2f lineDir = (end - start) / lineLength;
                        Vector2f toMouse = mousePos - start;
                        float projection = toMouse.x * lineDir.x + toMouse.y * lineDir.y;
                        projection = max(0.0f, min(lineLength, projection));
                        Vector2f closestPoint = start + lineDir * projection;

                        float distance = sqrt(pow(mousePos.x - closestPoint.x, 2) + pow(mousePos.y - closestPoint.y, 2));

                        if (distance < 15.0f && i < anim.pathRoutes.getSize())
                        {
                            hoveredRoute = anim.pathRoutes[i];
                            hoveredSegment = i;
                            hoveredAnimIndex = animIndex;
                            break;
                        }
                    }
                    if (hoveredRoute != nullptr)
                        break;
                }
            }

            
            if (hoveredRoute != nullptr && hoveredAnimIndex != -1)
            {
                RouteAnimation &anim = activeAnimations[hoveredAnimIndex];

                float boxWidth = 480.f, boxHeight = 420.f;
                float boxX = mousePos.x + 20.f;
                float boxY = mousePos.y - 350.f;
                if (boxX + boxWidth > 1920)
                    boxX = 1920 - boxWidth - 20;
                if (boxY < 0)
                    boxY = 20;

                RectangleShape infoBox(Vector2f(boxWidth, boxHeight));
                infoBox.setFillColor(Color(0, 20, 60, 240));
                infoBox.setOutlineThickness(3.f);
                infoBox.setOutlineColor(Color(255, 200, 0)); 
                infoBox.setPosition(boxX, boxY);

                VertexArray gradient(Quads, 4);
                gradient[0].position = Vector2f(boxX, boxY);
                gradient[1].position = Vector2f(boxX + boxWidth, boxY);
                gradient[2].position = Vector2f(boxX + boxWidth, boxY + boxHeight);
                gradient[3].position = Vector2f(boxX, boxY + boxHeight);

                gradient[0].color = Color(0, 30, 80, 240);
                gradient[1].color = Color(0, 20, 60, 240);
                gradient[2].color = Color(0, 10, 40, 240);
                gradient[3].color = Color(0, 30, 80, 240);

                window.draw(gradient);
                window.draw(infoBox);

                Text header;
                header.setFont(font);
                header.setString("A* ROUTE DETAILS");
                header.setCharacterSize(18);
                header.setFillColor(Color(255, 220, 100)); 
                header.setStyle(Text::Bold);
                header.setPosition(boxX + 15.f, boxY + 10.f);
                window.draw(header);

                Text details;
                details.setFont(font);
                details.setCharacterSize(13);
                details.setFillColor(Color(200, 230, 255));
                details.setStyle(Text::Bold);
                details.setPosition(boxX + 15.f, boxY + 35.f);

                DijkstraAlgorithm dijkstra;
                int travelTime = dijkstra.calculateTravelTime(hoveredRoute->departureTime, hoveredRoute->arrivalTime);

                
                int destDockWait = getPortDockWaitMinutesByName(hoveredRoute->destinationPort);
                
                
                int baseCost = hoveredRoute->cost;
                
                
                float portCharge = 100.0f;
                float layoverCost = 0.0f;
                for (int p = 0; p < numLocations; p++) {
                    if (ports[p].portName == hoveredRoute->destinationPort) {
                        portCharge = ports[p].charge;
                        layoverCost = portCharge * (destDockWait / 1440.0f);
                        break;
                    }
                }
                
                
                int totalPathCost = anim.totalCost;
                int totalPathTime = anim.totalTime;
                
                
                int basePathCost = 0;
                int totalDockingWait = 0;
                for (int r = 0; r < anim.pathRoutes.getSize(); r++) {
                    if (anim.pathRoutes[r] != nullptr) {
                        basePathCost += anim.pathRoutes[r]->cost;
                    }
                }
                for (int pi = 0; pi < anim.path.getSize(); pi++) {
                    totalDockingWait += getPortDockWaitMinutes(anim.path[pi]);
                }
                
                
                int dockingPenalty = 0;
                for (int pi = 0; pi < anim.path.getSize(); pi++) {
                    int portIdx = anim.path[pi];
                    int waitMin = getPortDockWaitMinutes(portIdx);
                    if (waitMin > 0) {
                        
                        string portName = portLocations[portIdx].name;
                        int graphIdx = graph.findPortIndex(portName);
                        if (graphIdx != -1) {
                            float charge = static_cast<float>(graph.ports[graphIdx].charge);
                            dockingPenalty += static_cast<int>(charge * (waitMin / 1440.0f));
                        }
                    }
                }
                int displayedTotalCost = basePathCost + dockingPenalty;

                string infoText = "From: " + hoveredRoute->startingPort + "\n" +
                                  "To: " + hoveredRoute->destinationPort + "\n" +
                                  "Searched Date: " + userSearchedDate + "\n" +
                                  "Route Valid On: " + hoveredRoute->departureDate + "\n" +
                                  "Departure: " + hoveredRoute->departureTime + "  Arrival: " + hoveredRoute->arrivalTime + "\n" +
                                  "Company: " + hoveredRoute->shippingCompany;

                
                if (hoveredSegment > 0 && hoveredSegment - 1 < anim.waitTimes.getSize())
                {
                    int waitTime = anim.waitTimes[hoveredSegment - 1];
                    infoText += "\nWait at Port: " + graph.formatWaitTime(waitTime);
                }

                details.setString(infoText);
                window.draw(details);
                
                
                RectangleShape costBox(Vector2f(boxWidth - 30, 90));
                costBox.setFillColor(Color(0, 15, 40, 200));
                costBox.setOutlineThickness(1);
                costBox.setOutlineColor(Color(200, 150, 50));
                costBox.setPosition(boxX + 15, boxY + 130);
                window.draw(costBox);
                
                Text costHeader;
                costHeader.setFont(font);
                costHeader.setString("TOTAL PATH COST BREAKDOWN");
                costHeader.setCharacterSize(12);
                costHeader.setFillColor(Color(255, 200, 100));
                costHeader.setStyle(Text::Bold);
                costHeader.setPosition(boxX + 25, boxY + 133);
                window.draw(costHeader);
                
                Text costDetails;
                costDetails.setFont(font);
                costDetails.setCharacterSize(11);
                costDetails.setPosition(boxX + 25, boxY + 150);
                
                string costText = "Base Route Cost:     $" + to_string(basePathCost) + "\n" +
                                  "Layover Penalty:     $" + to_string(dockingPenalty) + " (charge x hrs/24)\n" +
                                  "TOTAL PATH COST:     $" + to_string(displayedTotalCost);
                costDetails.setString(costText);
                costDetails.setFillColor(Color(100, 255, 150));
                window.draw(costDetails);
                
                
                RectangleShape timeBox(Vector2f(boxWidth - 30, 90));
                timeBox.setFillColor(Color(0, 15, 40, 200));
                timeBox.setOutlineThickness(1);
                timeBox.setOutlineColor(Color(100, 150, 200));
                timeBox.setPosition(boxX + 15, boxY + 230);
                window.draw(timeBox);
                
                Text timeHeader;
                timeHeader.setFont(font);
                timeHeader.setString("TOTAL PATH TIME BREAKDOWN");
                timeHeader.setCharacterSize(12);
                timeHeader.setFillColor(Color(100, 180, 255));
                timeHeader.setStyle(Text::Bold);
                timeHeader.setPosition(boxX + 25, boxY + 233);
                window.draw(timeHeader);
                
                
                
                
                
                for (int p = 0; p < numLocations; p++) {
                    if (ports[p].portName == hoveredRoute->destinationPort) {
                        portCharge = ports[p].charge;
                        layoverCost = portCharge * (destDockWait / 1440.0f);
                        break;
                    }
                }
                
                
                int baseTravelMinutes = totalPathTime - totalDockingWait;
                if (baseTravelMinutes < 0) baseTravelMinutes = totalPathTime;

                
                int baseDays = baseTravelMinutes / (24 * 60);
                int baseHrs = (baseTravelMinutes % (24 * 60)) / 60;
                int baseMins = baseTravelMinutes % 60;
                string baseTimeStr = to_string(baseDays) + "d " + to_string(baseHrs) + "h " + to_string(baseMins) + "m";

                int dockDays = totalDockingWait / (24 * 60);
                int dockHrs = (totalDockingWait % (24 * 60)) / 60;
                int dockMins = totalDockingWait % 60;
                string dockTimeStr = to_string(dockDays) + "d " + to_string(dockHrs) + "h " + to_string(dockMins) + "m";

                int totalDays = totalPathTime / (24 * 60);
                int totalHrs = (totalPathTime % (24 * 60)) / 60;
                int totalMins = totalPathTime % 60;
                string totalTimeStr = to_string(totalDays) + "d " + to_string(totalHrs) + "h " + to_string(totalMins) + "m";
                
                Text timeDetails;
                timeDetails.setFont(font);
                timeDetails.setCharacterSize(11);
                timeDetails.setPosition(boxX + 25, boxY + 250);
                
                string timeText = "Travel Time:         " + baseTimeStr + "\n" +
                                  "Docking Wait:        " + dockTimeStr + "\n" +
                                  "TOTAL JOURNEY:       " + totalTimeStr;
                timeDetails.setString(timeText);
                timeDetails.setFillColor(Color(100, 200, 255));
                window.draw(timeDetails);
                
                
                Text algoInfo;
                algoInfo.setFont(font);
                algoInfo.setString("Algorithm: A* (" + string(useCost ? "Cheapest" : "Fastest") + ")  |  Segments: " + to_string(anim.pathRoutes.getSize()));
                algoInfo.setCharacterSize(11);
                algoInfo.setFillColor(Color(180, 150, 255));
                algoInfo.setPosition(boxX + 25, boxY + 330);
                window.draw(algoInfo);

                static float boxPulse = 0.0f;
                boxPulse += 0.1f;
                float pulseValue = 0.5f + 0.5f * sin(boxPulse);
                infoBox.setOutlineColor(Color(255, 200, 0, 150 + 105 * pulseValue));
            }

            
            Text mainTitle("", font, 24);
            mainTitle.setString("A* Algorithm - " + string(useCost ? "Cheapest Route" : "Fastest Route"));
            mainTitle.setFillColor(Color(255, 220, 100)); 
            mainTitle.setStyle(Text::Bold);
            mainTitle.setPosition(Vector2f(400.f, 20.f));
            window.draw(mainTitle);

            

            Text escInstruction("", font, 12);
            escInstruction.setString("ESC to return to main menu");
            escInstruction.setFillColor(Color(255, 220, 150)); 
            escInstruction.setPosition(Vector2f(1750.f, 1050.f));
            window.draw(escInstruction);

            
            
            if (shipPrefsButton.getGlobalBounds().contains(mousePos))
            {
                shipPrefsButton.setFillColor(Color(180, 120, 0, 250));
                shipPrefsButton.setOutlineColor(Color(255, 220, 100));
            }
            else
            {
                shipPrefsButton.setFillColor(Color(120, 80, 0, 230));
                shipPrefsButton.setOutlineColor(Color(255, 180, 50));
            }
            window.draw(shipPrefsButton);
            window.draw(shipPrefsText);

            
            if (shipPrefs.filterActive)
            {
                prefsIndicator.setFillColor(Color(50, 255, 100));
            }
            else
            {
                prefsIndicator.setFillColor(Color(255, 100, 100));
            }
            window.draw(prefsIndicator);

            
            
            if (shipPrefs.filterActive)
            {
                
                int totalPorts = graph.ports.getSize();
                int avoidedCount = 0;
                for (int i = 0; i < totalPorts; i++)
                {
                    if (shipPrefs.isPortAvoided(graph.ports[i].portName))
                    {
                        avoidedCount++;
                    }
                }
                int activeCount = totalPorts - avoidedCount;

                
                RectangleShape subgraphPanel(Vector2f(280, 100));
                subgraphPanel.setFillColor(Color(30, 30, 60, 220));
                subgraphPanel.setOutlineThickness(2);
                subgraphPanel.setOutlineColor(Color(255, 180, 50));
                subgraphPanel.setPosition(20, 950);
                window.draw(subgraphPanel);

                
                Text subgraphTitle("SUBGRAPH VIEW ACTIVE", font, 16);
                subgraphTitle.setFillColor(Color(255, 220, 100));
                subgraphTitle.setStyle(Text::Bold);
                subgraphTitle.setPosition(35, 958);
                window.draw(subgraphTitle);

                
                Text activeText("Active Ports: " + to_string(activeCount), font, 14);
                activeText.setFillColor(Color(100, 255, 100));
                activeText.setPosition(35, 985);
                window.draw(activeText);

                
                Text avoidedText("Avoided Ports: " + to_string(avoidedCount) + " (faded)", font, 14);
                avoidedText.setFillColor(Color(255, 100, 100));
                avoidedText.setPosition(35, 1010);
                window.draw(avoidedText);

                
                CircleShape legendActive(6);
                legendActive.setFillColor(Color(255, 100, 50));
                legendActive.setPosition(240, 988);
                window.draw(legendActive);

                CircleShape legendFaded(6);
                legendFaded.setFillColor(Color(100, 50, 30, 100));  
                legendFaded.setPosition(240, 1013);
                window.draw(legendFaded);
            }

            window.display();
        }
    }

    
    void showShortestPathDijkstra(Maps &graph, Font &font)
    {
        RenderWindow window(VideoMode(1920, 1080), "Dijkstra Algorithm - OceanRoute Navigator");
        window.setFramerateLimit(60);
        
        
        View fixedView(FloatRect(0, 0, 1920, 1080));
        window.setView(fixedView);

        Texture mapTexture;
        if (!mapTexture.loadFromFile("pics/map2.png"))
        {
            cout << "Map image not loaded!" << endl;
            return;
        }
        Sprite mapSprite(mapTexture);
        mapSprite.setColor(Color(180, 220, 255, 255));

        struct RouteAnimation
        {
            Vector<int> path;
            Vector<string> waitPorts;
            Vector<string> waitDurations;
            Color routeColor;
            string routeType;
            int currentSegment;
            float segmentProgress;
            bool isPaused;
            float pauseTimer;
            bool isComplete;
            Vector<RouteNode *> pathRoutes;
            Vector<int> waitTimes;
            int totalCost;
            int totalTime;
        };

        Vector<RouteAnimation> activeAnimations;
        float animationSpeed = 0.8f;
        const float SEGMENT_PAUSE = 1.5f;
        string userSearchedDate = "";  

        Clock clock;
        float time = 0.0f;

        
        
        RectangleShape panelGlow(Vector2f(320.f, 440.f));
        panelGlow.setFillColor(Color(0, 200, 100, 40));
        panelGlow.setPosition(Vector2f(10.f, 545.f));

        
        RectangleShape inputPanel(Vector2f(300.f, 420.f));
        inputPanel.setFillColor(Color(10, 30, 25, 245));
        inputPanel.setOutlineThickness(3.f);
        inputPanel.setOutlineColor(Color(0, 200, 120, 200));
        inputPanel.setPosition(Vector2f(20.f, 555.f));

        
        RectangleShape panelTopAccent(Vector2f(300.f, 6.f));
        panelTopAccent.setFillColor(Color(0, 255, 150));
        panelTopAccent.setPosition(Vector2f(20.f, 555.f));

        
        RectangleShape headerBg(Vector2f(300.f, 45.f));
        headerBg.setFillColor(Color(0, 60, 40, 200));
        headerBg.setPosition(Vector2f(20.f, 561.f));

        
        Text panelTitle;
        panelTitle.setFont(font);
        panelTitle.setString("DIJKSTRA NAVIGATOR");
        panelTitle.setCharacterSize(18);
        panelTitle.setFillColor(Color(0, 255, 150));
        panelTitle.setStyle(Text::Bold);
        panelTitle.setLetterSpacing(2);
        FloatRect titleBounds = panelTitle.getLocalBounds();
        panelTitle.setPosition(170.f - titleBounds.width / 2.f, 570.f);

        
        CircleShape networkIcon(12.f);
        networkIcon.setFillColor(Color(0, 255, 150, 150));
        networkIcon.setOutlineThickness(2);
        networkIcon.setOutlineColor(Color(0, 255, 150));
        networkIcon.setOrigin(12.f, 12.f);
        networkIcon.setPosition(45.f, 583.f);

        CircleShape networkInner(6.f);
        networkInner.setFillColor(Color(0, 255, 150));
        networkInner.setOrigin(6.f, 6.f);
        networkInner.setPosition(45.f, 583.f);

        
        class InputBox
        {
        public:
            RectangleShape box;
            RectangleShape iconBg;
            Text text;
            Text label;
            Text icon;
            string inputString;
            bool isActive;

            InputBox(Font &font, string lbl, string iconChar, Vector2f position)
            {
                label.setFont(font);
                label.setString(lbl);
                label.setCharacterSize(13);
                label.setFillColor(Color(150, 255, 200));
                label.setStyle(Text::Bold);
                label.setPosition(position.x + 5, position.y - 22);

                
                iconBg.setSize(Vector2f(30, 38));
                iconBg.setFillColor(Color(0, 80, 50, 200));
                iconBg.setPosition(position.x, position.y);

                icon.setFont(font);
                icon.setString(iconChar);
                icon.setCharacterSize(14);
                icon.setFillColor(Color(0, 255, 150));
                icon.setStyle(Text::Bold);
                icon.setPosition(position.x + 8, position.y + 10);

                box.setSize(Vector2f(230, 38));
                box.setFillColor(Color(10, 35, 30, 240));
                box.setOutlineThickness(2);
                box.setOutlineColor(Color(0, 150, 100));
                box.setPosition(position.x + 30, position.y);

                text.setFont(font);
                text.setCharacterSize(14);
                text.setFillColor(Color::White);
                text.setPosition(position.x + 40, position.y + 10);

                inputString = "";
                isActive = false;
            }

            void handleEvent(Event &event, RenderWindow &window)
            {
                Vector2f mousePos = window.mapPixelToCoords(Mouse::getPosition(window));

                if (event.type == Event::MouseButtonPressed)
                {
                    if (box.getGlobalBounds().contains(mousePos))
                    {
                        isActive = true;
                        box.setOutlineColor(Color(0, 255, 150));
                        box.setFillColor(Color(15, 45, 40, 255));
                    }
                    else
                    {
                        isActive = false;
                        box.setOutlineColor(Color(0, 150, 100));
                        box.setFillColor(Color(10, 35, 30, 240));
                    }
                }

                if (event.type == Event::TextEntered && isActive)
                {
                    if (event.text.unicode == '\b')
                    {
                        if (!inputString.empty())
                            inputString.pop_back();
                    }
                    else if (event.text.unicode < 128 && event.text.unicode != '\r' && event.text.unicode != '\t')
                    {
                        inputString += static_cast<char>(event.text.unicode);
                    }
                    text.setString(inputString);
                }
            }

            void draw(RenderWindow &window)
            {
                window.draw(iconBg);
                window.draw(icon);
                window.draw(label);
                window.draw(box);
                window.draw(text);
            }

            string getText() { return inputString; }
            void setText(string newText)
            {
                inputString = newText;
                text.setString(inputString);
            }
        };

        
        InputBox departureInput(font, "Departure Port", ">", Vector2f(30, 635));
        InputBox destinationInput(font, "Destination Port", "X", Vector2f(30, 700));
        InputBox dateInput(font, "Date (DD/MM/YYYY)", "#", Vector2f(30, 765));

        
        
        RectangleShape sectionDivider(Vector2f(260, 2));
        sectionDivider.setFillColor(Color(0, 200, 120, 100));
        sectionDivider.setPosition(30, 820);

        Text modeLabel;
        modeLabel.setFont(font);
        modeLabel.setString("OPTIMIZATION MODE");
        modeLabel.setCharacterSize(11);
        modeLabel.setFillColor(Color(100, 200, 150));
        modeLabel.setLetterSpacing(1.5f);
        modeLabel.setPosition(75, 828);

        
        RectangleShape costButton(Vector2f(260, 40));
        costButton.setFillColor(Color(0, 100, 60, 220));
        costButton.setOutlineThickness(2);
        costButton.setOutlineColor(Color(0, 255, 150));
        costButton.setPosition(30, 850);

        CircleShape costIcon(12.f, 6);
        costIcon.setFillColor(Color(0, 255, 150));
        costIcon.setOrigin(12.f, 12.f);
        costIcon.setPosition(55, 870);

        Text costButtonText("CHEAPEST ROUTE", font, 13);
        costButtonText.setFillColor(Color::White);
        costButtonText.setStyle(Text::Bold);
        costButtonText.setPosition(80, 858);

        Text costSubText("Minimize total cost", font, 10);
        costSubText.setFillColor(Color(100, 200, 150));
        costSubText.setPosition(80, 874);

        
        RectangleShape timeButton(Vector2f(260, 40));
        timeButton.setFillColor(Color(0, 60, 40, 220));
        timeButton.setOutlineThickness(2);
        timeButton.setOutlineColor(Color(0, 150, 100));
        timeButton.setPosition(30, 900);

        CircleShape timeIcon(12.f, 3);
        timeIcon.setFillColor(Color(0, 200, 150));
        timeIcon.setOrigin(12.f, 12.f);
        timeIcon.setPosition(55, 920);

        Text timeButtonText("FASTEST ROUTE", font, 13);
        timeButtonText.setFillColor(Color::White);
        timeButtonText.setStyle(Text::Bold);
        timeButtonText.setPosition(80, 908);

        Text timeSubText("Minimize travel time", font, 10);
        timeSubText.setFillColor(Color(80, 180, 130));
        timeSubText.setPosition(80, 924);

        bool useCost = true;

        
        RectangleShape calcButtonGlow(Vector2f(270, 55));
        calcButtonGlow.setFillColor(Color(0, 200, 100, 0));
        calcButtonGlow.setPosition(25, 947);

        RectangleShape calculateButton(Vector2f(260, 50));
        calculateButton.setFillColor(Color(0, 150, 100));
        calculateButton.setOutlineThickness(3);
        calculateButton.setOutlineColor(Color(0, 255, 150));
        calculateButton.setPosition(30, 950);

        
        CircleShape calcCircle(8.f);
        calcCircle.setFillColor(Color::White);
        calcCircle.setOrigin(8.f, 8.f);
        calcCircle.setPosition(55, 975);

        Text calculateButtonText("FIND OPTIMAL PATH", font, 16);
        calculateButtonText.setFillColor(Color::White);
        calculateButtonText.setStyle(Text::Bold);
        calculateButtonText.setLetterSpacing(1.2f);
        calculateButtonText.setPosition(78, 967);

        
        RectangleShape shipPrefsButton(Vector2f(220, 50));
        shipPrefsButton.setFillColor(Color(80, 40, 120, 230));
        shipPrefsButton.setOutlineThickness(3);
        shipPrefsButton.setOutlineColor(Color(180, 100, 255));
        shipPrefsButton.setPosition(1680, 20);

        Text shipPrefsText("SHIP PREFS", font, 16);
        shipPrefsText.setFillColor(Color::White);
        shipPrefsText.setStyle(Text::Bold);
        shipPrefsText.setPosition(1720, 32);

        
        CircleShape prefsIndicator(8);
        prefsIndicator.setPosition(1870, 35);

        
        Text statusMessage("", font, 14);
        statusMessage.setPosition(30, 985);
        statusMessage.setFillColor(Color::Yellow);

        
        RouteNode *hoveredRoute = nullptr;
        Vector<float> portPulse;
        for (int i = 0; i < graph.ports.getSize(); i++)
        {
            portPulse.push_back(0.0f);
        }

        

        auto addRouteAnimation = [&](Vector<int> path, string userDate, Color color, string type)
        {
            RouteAnimation newAnim;
            newAnim.path = path;
            newAnim.waitPorts = Vector<string>();
            newAnim.waitDurations = Vector<string>();
            newAnim.routeColor = color;
            newAnim.routeType = type;
            newAnim.currentSegment = 0;
            newAnim.segmentProgress = 0.0f;
            newAnim.isPaused = false;
            newAnim.pauseTimer = 0.0f;
            newAnim.isComplete = false;

            newAnim.pathRoutes.clear();
            newAnim.totalCost = 0;
            newAnim.totalTime = 0;

            if (path.getSize() > 1)
            {
                DijkstraAlgorithm dijkstra;
                string currentDate = graph.normalizeDate(userDate);
                string currentTime = "00:00";

                for (int i = 0; i < path.getSize() - 1; i++)
                {
                    string fromPort = portLocations[path[i]].name;
                    string toPort = portLocations[path[i + 1]].name;

                    int fromIndex = graph.findPortIndex(fromPort);
                    RouteNode *route = graph.ports[fromIndex].routeHead;

                    RouteNode *bestRoute = nullptr;
                    int minWaitTime = INT_MAX;
                    int bestTravelTime = 0;

                    bool isCostMode = useCost; 

                    while (route != nullptr)
                    {
                        if (route->destinationPort == toPort)
                        {
                            if (dijkstra.isDateGreaterOrEqual(route->departureDate, currentDate))
                            {
                                int waitMinutes = 0;

                                if (route->departureDate == currentDate)
                                {
                                    int departureMin = dijkstra.toMinutes(route->departureTime);
                                    int currentMin = dijkstra.toMinutes(currentTime);

                                    if (departureMin >= currentMin + 120)
                                    {
                                        waitMinutes = departureMin - currentMin;
                                    }
                                    else
                                    {
                                        route = route->next;
                                        continue;
                                    }
                                }
                                else
                                {
                                    int daysBetween = dijkstra.calculateDaysBetween(currentDate, route->departureDate);
                                    int departureMin = dijkstra.toMinutes(route->departureTime);
                                    int currentMin = dijkstra.toMinutes(currentTime);

                                    waitMinutes = (24 * 60 - currentMin) +
                                                  (daysBetween - 1) * 24 * 60 +
                                                  departureMin;
                                }

                                int travelTime = dijkstra.calculateTravelTime(route->departureTime, route->arrivalTime);

                                if (isCostMode)
                                {
                                    if (bestRoute == nullptr || route->cost < bestRoute->cost)
                                    {
                                        bestRoute = route;
                                        minWaitTime = waitMinutes;
                                        bestTravelTime = travelTime;
                                    }
                                }
                                else
                                {
                                    int totalSegmentTime = waitMinutes + travelTime;
                                    if (bestRoute == nullptr || totalSegmentTime < (minWaitTime + bestTravelTime))
                                    {
                                        bestRoute = route;
                                        minWaitTime = waitMinutes;
                                        bestTravelTime = travelTime;
                                    }
                                }
                            }
                        }
                        route = route->next;
                    }

                    if (bestRoute != nullptr)
                    {
                        newAnim.pathRoutes.push_back(bestRoute);
                        newAnim.totalCost += bestRoute->cost;

                        if (minWaitTime > 0)
                        {
                            newAnim.totalTime += minWaitTime;

                            if (i == 0)
                            {
                                newAnim.waitPorts.push_back(fromPort + " (initial)");
                            }
                            else
                            {
                                newAnim.waitPorts.push_back(fromPort);
                            }
                            newAnim.waitDurations.push_back(graph.formatWaitTime(minWaitTime));
                        }

                        newAnim.totalTime += bestTravelTime;

                        currentDate = bestRoute->departureDate;
                        currentTime = bestRoute->arrivalTime;
                    }
                    else
                    {
                        cout << "Error: No route found from " << fromPort << " to " << toPort << endl;
                        return;
                    }
                }
            }

            activeAnimations.push_back(newAnim);
        };
        while (window.isOpen())
        {
            Event event;
            Vector2f mousePos = window.mapPixelToCoords(Mouse::getPosition(window));
            float deltaTime = clock.restart().asSeconds();
            time += deltaTime;

            while (window.pollEvent(event))
            {
                if (event.type == Event::Closed)
                    window.close();

                if (event.type == Event::KeyPressed && event.key.code == Keyboard::Escape)
                    window.close();

                
                departureInput.handleEvent(event, window);
                destinationInput.handleEvent(event, window);
                dateInput.handleEvent(event, window);

                if (event.type == Event::MouseButtonPressed)
                {
                    Vector2f mousePos = window.mapPixelToCoords(Mouse::getPosition(window));

                    
                    if (costButton.getGlobalBounds().contains(mousePos))
                    {
                        useCost = true;
                        activeAnimations.clear();
                        statusMessage.setString("Mode: Cheapest Route (Minimize Cost)");
                        statusMessage.setFillColor(Color::Green);
                    }
                    if (timeButton.getGlobalBounds().contains(mousePos))
                    {
                        useCost = false;
                        activeAnimations.clear();
                        statusMessage.setString("Mode: Fastest Route (Minimize Time)");
                        statusMessage.setFillColor(Color::Cyan);
                    }

                    
                    if (shipPrefsButton.getGlobalBounds().contains(mousePos))
                    {
                        openShipPreferencesPopup(font);
                    }

                    
                    if (calculateButton.getGlobalBounds().contains(mousePos))
                    {
                        string departureText = departureInput.getText();
                        string destinationText = destinationInput.getText();
                        string dateText = dateInput.getText();

                        
                        if (departureText.empty() || destinationText.empty() || dateText.empty())
                        {
                            statusMessage.setString("Error: Please fill all fields!");
                            statusMessage.setFillColor(Color::Red);
                        }
                        else
                        {
                            string normalizedDate = graph.normalizeDate(dateText);

                            
                            if (normalizedDate.length() != 10 || normalizedDate[2] != '/' || normalizedDate[5] != '/')
                            {
                                statusMessage.setString("Error: Invalid date format! Use DD/MM/YYYY");
                                statusMessage.setFillColor(Color::Red);
                            }
                            else
                            {
                                
                                try
                                {
                                    int day = stoi(normalizedDate.substr(0, 2));
                                    int month = stoi(normalizedDate.substr(3, 2));
                                    int year = stoi(normalizedDate.substr(6, 4));

                                    if (day < 1 || day > 31 || month < 1 || month > 12 || year < 2024)
                                    {
                                        statusMessage.setString("Error: Invalid date values!");
                                        statusMessage.setFillColor(Color::Red);
                                    }
                                    else
                                    {
                                        int originIdx = graph.findPortIndex(departureText);
                                        int destIdx = graph.findPortIndex(destinationText);

                                        if (originIdx == -1 && destIdx == -1)
                                        {
                                            statusMessage.setString("Error: Both ports not found!");
                                            statusMessage.setFillColor(Color::Red);
                                        }
                                        else if (originIdx == -1)
                                        {
                                            statusMessage.setString("Error: Departure port not found!");
                                            statusMessage.setFillColor(Color::Red);
                                        }
                                        else if (destIdx == -1)
                                        {
                                            statusMessage.setString("Error: Destination port not found!");
                                            statusMessage.setFillColor(Color::Red);
                                        }
                                        else
                                        {
                                            activeAnimations.clear();
                                            DijkstraAlgorithm dijkstra;
                                            Vector<int> path;

                                            if (useCost)
                                            {
                                                path = dijkstra.findCheapestPath(graph.ports, originIdx, destIdx, normalizedDate, &shipPrefs);
                                                statusMessage.setString("Calculating cheapest path...");
                                                statusMessage.setFillColor(Color::Green);
                                            }
                                            else
                                            {
                                                path = dijkstra.findFastestPath(graph.ports, originIdx, destIdx, normalizedDate, &shipPrefs);
                                                statusMessage.setString("Calculating fastest path...");
                                                statusMessage.setFillColor(Color::Cyan);
                                            }

                                            if (path.getSize() > 1)
                                            {
                                                userSearchedDate = normalizedDate;  
                                                Color routeColor = useCost ? Color::Green : Color::Cyan;
                                                string routeType = useCost ? "Dijkstra Cheapest" : "Dijkstra Fastest";
                                                addRouteAnimation(path, normalizedDate, routeColor, routeType);

                                                
                                                string mode = useCost ? "Cheapest" : "Fastest";
                                                statusMessage.setString(" " + mode + " path found! (" +
                                                                        to_string(path.getSize()) + " ports)");
                                                statusMessage.setFillColor(Color::Green);
                                            }
                                            else
                                            {
                                                statusMessage.setString("No path found for given ports and date!");
                                                statusMessage.setFillColor(Color::Red);
                                            }
                                        }
                                    }
                                }
                                catch (...)
                                {
                                    statusMessage.setString("Error: Invalid date values!");
                                    statusMessage.setFillColor(Color::Red);
                                }
                            }
                        }
                    }
                }
            }

            
            for (int i = 0; i < portPulse.getSize(); i++)
            {
                if (portPulse[i] > 0)
                {
                    portPulse[i] -= 0.05f;
                    if (portPulse[i] < 0)
                        portPulse[i] = 0;
                }
            }

            
            for (int animIndex = 0; animIndex < activeAnimations.getSize(); animIndex++)
            {
                RouteAnimation &anim = activeAnimations[animIndex];

                if (!anim.isComplete)
                {
                    if (!anim.isPaused)
                    {
                        anim.segmentProgress += deltaTime * animationSpeed;

                        if (anim.segmentProgress >= 1.0f)
                        {
                            anim.segmentProgress = 1.0f;
                            anim.isPaused = true;
                            anim.pauseTimer = 0.0f;
                        }
                    }
                    else
                    {
                        anim.pauseTimer += deltaTime;
                        if (anim.pauseTimer >= SEGMENT_PAUSE)
                        {
                            anim.currentSegment++;
                            anim.segmentProgress = 0.0f;
                            anim.isPaused = false;

                            if (anim.currentSegment >= anim.path.getSize() - 1)
                            {
                                anim.isComplete = true;
                            }
                        }
                    }
                }
            }

            
            float panelPulse = 0.5f + 0.5f * sin(time * 2.0f);
            panelTopAccent.setFillColor(Color(0, 200 + 55 * panelPulse, 100 + 50 * panelPulse));
            networkIcon.setScale(0.9f + 0.15f * panelPulse, 0.9f + 0.15f * panelPulse);
            calcCircle.setScale(0.9f + 0.15f * panelPulse, 0.9f + 0.15f * panelPulse);

            
            window.clear(Color(10, 20, 40));
            window.draw(mapSprite);

            
            window.draw(panelGlow);
            window.draw(inputPanel);
            window.draw(panelTopAccent);
            window.draw(headerBg);
            window.draw(panelTitle);
            window.draw(networkIcon);
            window.draw(networkInner);

            
            departureInput.draw(window);
            destinationInput.draw(window);
            dateInput.draw(window);

            
            window.draw(sectionDivider);
            window.draw(modeLabel);

            
            if (useCost)
            {
                costButton.setFillColor(Color(0, 140, 90, 255));
                costButton.setOutlineColor(Color(0, 255, 150));
                costIcon.setFillColor(Color(100, 255, 200));
                timeButton.setFillColor(Color(0, 50, 35, 200));
                timeButton.setOutlineColor(Color(0, 120, 80));
                timeIcon.setFillColor(Color(0, 150, 100));
            }
            else
            {
                costButton.setFillColor(Color(0, 50, 35, 200));
                costButton.setOutlineColor(Color(0, 120, 80));
                costIcon.setFillColor(Color(0, 150, 100));
                timeButton.setFillColor(Color(0, 140, 90, 255));
                timeButton.setOutlineColor(Color(0, 255, 150));
                timeIcon.setFillColor(Color(100, 255, 200));
            }

            
            if (costButton.getGlobalBounds().contains(mousePos))
            {
                costButton.setOutlineColor(Color(150, 255, 200));
                costButton.setFillColor(useCost ? Color(0, 160, 110, 255) : Color(0, 80, 55, 240));
            }

            if (timeButton.getGlobalBounds().contains(mousePos))
            {
                timeButton.setOutlineColor(Color(150, 255, 200));
                timeButton.setFillColor(!useCost ? Color(0, 160, 110, 255) : Color(0, 80, 55, 240));
            }

            window.draw(costButton);
            window.draw(costIcon);
            window.draw(costButtonText);
            window.draw(costSubText);
            window.draw(timeButton);
            window.draw(timeIcon);
            window.draw(timeButtonText);
            window.draw(timeSubText);

            
            if (calculateButton.getGlobalBounds().contains(mousePos))
            {
                calcButtonGlow.setFillColor(Color(0, 200, 100, 60));
                calculateButton.setFillColor(Color(0, 200, 130));
                calculateButton.setOutlineColor(Color(100, 255, 200));
            }
            else
            {
                calcButtonGlow.setFillColor(Color(0, 200, 100, 0));
                calculateButton.setFillColor(Color(0, 150, 100));
                calculateButton.setOutlineColor(Color(0, 255, 150));
            }
            window.draw(calcButtonGlow);
            window.draw(calculateButton);
            window.draw(calcCircle);
            window.draw(calculateButtonText);

            
            window.draw(statusMessage);

            
            
            for (int i = 0; i < numLocations; i++)
            {
                
                string currentPortName = portLocations[i].name;
                bool isAvoided = shipPrefs.isPortAvoided(currentPortName);
                bool isInSubgraph = !isAvoided; 
                
                
                float fadeFactor = 1.0f;
                if (shipPrefs.filterActive && isAvoided) {
                    fadeFactor = 0.1f; 
                }

                
                if (shipPrefs.filterActive && isAvoided) {
                    CircleShape avoidCircle(25.f);
                    avoidCircle.setOrigin(25, 25);
                    avoidCircle.setFillColor(Color(255, 0, 0, 40)); 
                    avoidCircle.setOutlineThickness(3);
                    avoidCircle.setOutlineColor(Color(255, 50, 50, 200)); 
                    avoidCircle.setPosition(portLocations[i].x, portLocations[i].y);
                    window.draw(avoidCircle);
                }

                CircleShape glow(16.f);
                glow.setOrigin(16, 16);
                
                glow.setFillColor(Color(100, 180, 255, static_cast<Uint8>(100 * fadeFactor)));
                glow.setPosition(portLocations[i].x, portLocations[i].y);
                window.draw(glow);

                float pulseScale = 1.0f + 0.2f * portPulse[i];
                CircleShape dot(8.f * pulseScale);
                dot.setOrigin(8.f * pulseScale, 8.f * pulseScale);

                bool inPath = false;
                for (int animIndex = 0; animIndex < activeAnimations.getSize(); animIndex++)
                {
                    RouteAnimation &anim = activeAnimations[animIndex];
                    for (int j = 0; j < anim.path.getSize(); j++)
                    {
                        if (anim.path[j] == i)
                        {
                            inPath = true;
                            break;
                        }
                    }
                    if (inPath)
                        break;
                }

                if (inPath)
                {
                    dot.setFillColor(useCost ? Color::Green : Color::Cyan);
                }
                else if (isAvoided && shipPrefs.filterActive)
                {
                    
                    dot.setFillColor(Color(150, 50, 50, 80));
                }
                else
                {
                    float gentlePulse = 0.5f + 0.5f * sin(time * 2.0f + i);
                    dot.setFillColor(Color(0, static_cast<Uint8>(150 * gentlePulse * fadeFactor), static_cast<Uint8>(255 * fadeFactor)));
                }

                dot.setOutlineColor(Color(255, 255, 255, static_cast<Uint8>(255 * fadeFactor)));
                dot.setOutlineThickness(1.5f);
                dot.setPosition(portLocations[i].x, portLocations[i].y);
                window.draw(dot);

                
                if (isAvoided && shipPrefs.filterActive) {
                    
                    Text bigX("X", font, 28);
                    bigX.setFillColor(Color(255, 50, 50, 255)); 
                    bigX.setStyle(Text::Bold);
                    bigX.setPosition(portLocations[i].x - 10, portLocations[i].y - 18);
                    window.draw(bigX);
                    
                    
                    Text avoidedLabel("AVOIDED", font, 10);
                    avoidedLabel.setFillColor(Color(255, 100, 100, 220));
                    avoidedLabel.setStyle(Text::Bold);
                    avoidedLabel.setPosition(portLocations[i].x - 22, portLocations[i].y + 12);
                    window.draw(avoidedLabel);
                }

                
                PortDockingState& portDock = globalPortDocking[i];
                int dockedCount = portDock.getOccupiedDocks();
                int queueCount = portDock.getQueueLength();
                
                if (dockedCount > 0 || queueCount > 0) {
                    float baseX = portLocations[i].x + 15;
                    float baseY = portLocations[i].y - 10;
                    
                    
                    for (int slot = 0; slot < portDock.maxDocks; slot++) {
                        RectangleShape miniSlot(Vector2f(6, 6));
                        miniSlot.setPosition(baseX + slot * 8, baseY);
                        
                        
                        bool occupied = false;
                        for (int d = 0; d < dockedCount; d++) {
                            if (portDock.dockedShips[d].dockSlot == slot) {
                                occupied = true;
                                break;
                            }
                        }
                        
                        if (occupied) {
                            miniSlot.setFillColor(Color(0, 200, 100)); 
                        } else {
                            miniSlot.setFillColor(Color(60, 60, 80)); 
                        }
                        miniSlot.setOutlineThickness(1);
                        miniSlot.setOutlineColor(Color(100, 100, 120));
                        window.draw(miniSlot);
                    }
                    
                    
                    if (queueCount > 0) {
                        float queueStartX = baseX + portDock.maxDocks * 8 + 4;
                        float dashLen = 3.f;
                        for (int d = 0; d < 4; d++) {
                            RectangleShape dash(Vector2f(dashLen, 2));
                            dash.setPosition(queueStartX + d * (dashLen + 2), baseY + 2);
                            dash.setFillColor(Color(255, 180, 0, 180));
                            window.draw(dash);
                        }
                        
                        
                        CircleShape qBadge(7);
                        qBadge.setFillColor(Color(255, 100, 50));
                        qBadge.setOutlineThickness(1);
                        qBadge.setOutlineColor(Color::White);
                        qBadge.setPosition(queueStartX + 20, baseY - 3);
                        window.draw(qBadge);
                        
                        Text qText(to_string(queueCount), font, 10);
                        qText.setFillColor(Color::White);
                        qText.setStyle(Text::Bold);
                        qText.setPosition(queueStartX + 23, baseY - 2);
                        window.draw(qText);
                    }
                    
                    
                    if (dockedCount > 0) {
                        CircleShape dBadge(7);
                        dBadge.setFillColor(Color(0, 150, 100));
                        dBadge.setOutlineThickness(1);
                        dBadge.setOutlineColor(Color::White);
                        dBadge.setPosition(baseX - 12, baseY - 3);
                        window.draw(dBadge);
                        
                        Text dText(to_string(dockedCount), font, 10);
                        dText.setFillColor(Color::White);
                        dText.setStyle(Text::Bold);
                        dText.setPosition(baseX - 9, baseY - 2);
                        window.draw(dText);
                    }
                }
            }

            
            for (int animIndex = 0; animIndex < activeAnimations.getSize(); animIndex++)
            {
                RouteAnimation &anim = activeAnimations[animIndex];

                if (anim.path.getSize() > 1)
                {
                    for (int i = 0; i < anim.path.getSize() - 1; i++)
                    {
                        int startIdx = anim.path[i];
                        int endIdx = anim.path[i + 1];
                        Vector2f start(portLocations[startIdx].x, portLocations[startIdx].y);
                        Vector2f end(portLocations[endIdx].x, portLocations[endIdx].y);

                        float length = sqrt(pow(end.x - start.x, 2) + pow(end.y - start.y, 2));
                        float angle = atan2(end.y - start.y, end.x - start.x) * 180 / 3.14159f;

                        if (i == anim.currentSegment && !anim.isComplete)
                        {
                            float currentLength = length * anim.segmentProgress;

                            RectangleShape currentLine(Vector2f(currentLength, 6.0f));
                            currentLine.setPosition(start);
                            currentLine.setRotation(angle);
                            currentLine.setFillColor(anim.routeColor);
                            window.draw(currentLine);

                            RectangleShape currentGlow(Vector2f(currentLength, 12.0f));
                            currentGlow.setPosition(start);
                            currentGlow.setRotation(angle);
                            currentGlow.setFillColor(Color(anim.routeColor.r, anim.routeColor.g, anim.routeColor.b, 100));
                            window.draw(currentGlow);

                            if (!anim.isComplete)
                            {
                                Vector2f currentPos = start + (end - start) * anim.segmentProgress;
                                float pulse = 0.7f + 0.3f * sin(time * 8.0f);

                                CircleShape endGlow(15.f * pulse);
                                endGlow.setFillColor(Color(anim.routeColor.r, anim.routeColor.g, anim.routeColor.b, 150));
                                endGlow.setOrigin(15.f * pulse, 15.f * pulse);
                                endGlow.setPosition(currentPos);
                                window.draw(endGlow);

                                CircleShape endDot(8.f);
                                endDot.setFillColor(Color::Yellow);
                                endDot.setOutlineColor(Color::White);
                                endDot.setOutlineThickness(2);
                                endDot.setOrigin(8.f, 8.f);
                                endDot.setPosition(currentPos);
                                window.draw(endDot);
                            }
                        }
                        else if (i < anim.currentSegment || anim.isComplete)
                        {
                            RectangleShape completedLine(Vector2f(length, 5.0f));
                            completedLine.setPosition(start);
                            completedLine.setRotation(angle);
                            completedLine.setFillColor(anim.routeColor);
                            window.draw(completedLine);
                        }
                        else
                        {
                            RectangleShape upcomingLine(Vector2f(length, 3.0f));
                            upcomingLine.setPosition(start);
                            upcomingLine.setRotation(angle);
                            upcomingLine.setFillColor(Color(anim.routeColor.r, anim.routeColor.g, anim.routeColor.b, 100));
                            window.draw(upcomingLine);
                        }
                    }

                    
                    if (animIndex == 0)
                    {
                        RectangleShape routeInfoBox(Vector2f(500, 220));
                        routeInfoBox.setFillColor(Color(0, 30, 60, 200));
                        routeInfoBox.setOutlineThickness(3);
                        routeInfoBox.setOutlineColor(anim.routeColor);
                        routeInfoBox.setPosition(250, 850);
                        window.draw(routeInfoBox);

                        Text routeTitle;
                        routeTitle.setFont(font);
                        routeTitle.setString(anim.routeType + " - $" + to_string(anim.totalCost) +
                                             " - " + graph.formatTime(anim.totalTime));
                        routeTitle.setCharacterSize(18);
                        routeTitle.setFillColor(anim.routeColor);
                        routeTitle.setStyle(Text::Bold);
                        routeTitle.setPosition(260, 860);
                        window.draw(routeTitle);

                        string pathStr = "";
                        for (int i = 0; i < anim.path.getSize(); i++)
                        {
                            pathStr += portLocations[anim.path[i]].name;
                            if (i < anim.path.getSize() - 1)
                                pathStr += " → ";
                        }

                        Text pathText;
                        pathText.setFont(font);
                        pathText.setString(pathStr);
                        pathText.setCharacterSize(14);
                        pathText.setFillColor(Color::White);
                        pathText.setPosition(260, 890);
                        window.draw(pathText);

                        
                        if (anim.waitPorts.getSize() > 0)
                        {
                            Text waitText;
                            waitText.setFont(font);
                            waitText.setCharacterSize(12);
                            waitText.setFillColor(Color::Yellow);
                            waitText.setPosition(260, 920);

                            string waitInfo = "Waiting at: ";
                            for (int i = 0; i < anim.waitPorts.getSize(); i++)
                            {
                                if (i > 0)
                                    waitInfo += ", ";
                                waitInfo += anim.waitPorts[i] + "(" + anim.waitDurations[i] + ")";
                            }
                            waitText.setString(waitInfo);
                            window.draw(waitText);
                        }

                        if (anim.isComplete)
                        {
                            Text completeText;
                            completeText.setFont(font);
                            completeText.setString(" Route Complete");
                            completeText.setCharacterSize(14);
                            completeText.setFillColor(Color::Green);
                            completeText.setPosition(260, 945);
                            window.draw(completeText);
                        }
                    }
                }
            }

            
            hoveredRoute = nullptr;
            int hoveredSegment = -1;
            int hoveredAnimIndex = -1;

            if (activeAnimations.getSize() > 0)
            {
                for (int animIndex = 0; animIndex < activeAnimations.getSize(); animIndex++)
                {
                    RouteAnimation &anim = activeAnimations[animIndex];
                    for (int i = 0; i < anim.path.getSize() - 1; i++)
                    {
                        Vector2f start(portLocations[anim.path[i]].x, portLocations[anim.path[i]].y);
                        Vector2f end(portLocations[anim.path[i + 1]].x, portLocations[anim.path[i + 1]].y);

                        float length = sqrt(pow(end.x - start.x, 2) + pow(end.y - start.y, 2));

                        float lineLength = length;
                        Vector2f lineDir = (end - start) / lineLength;
                        Vector2f toMouse = mousePos - start;
                        float projection = toMouse.x * lineDir.x + toMouse.y * lineDir.y;
                        projection = max(0.0f, min(lineLength, projection));
                        Vector2f closestPoint = start + lineDir * projection;

                        float distance = sqrt(pow(mousePos.x - closestPoint.x, 2) + pow(mousePos.y - closestPoint.y, 2));

                        if (distance < 15.0f && i < anim.pathRoutes.getSize())
                        {
                            hoveredRoute = anim.pathRoutes[i];
                            hoveredSegment = i;
                            hoveredAnimIndex = animIndex;
                            break;
                        }
                    }
                    if (hoveredRoute != nullptr)
                        break;
                }
            }

            
            if (hoveredRoute != nullptr && hoveredAnimIndex != -1)
            {
                RouteAnimation &anim = activeAnimations[hoveredAnimIndex];

                float boxWidth = 480.f, boxHeight = 420.f;
                float boxX = mousePos.x + 20.f;
                float boxY = mousePos.y - 350.f;
                if (boxX + boxWidth > 1920)
                    boxX = 1920 - boxWidth - 20;
                if (boxY < 0)
                    boxY = 20;

                RectangleShape infoBox(Vector2f(boxWidth, boxHeight));
                infoBox.setFillColor(Color(0, 20, 60, 240));
                infoBox.setOutlineThickness(3.f);
                infoBox.setOutlineColor(anim.routeColor);
                infoBox.setPosition(boxX, boxY);

                VertexArray gradient(Quads, 4);
                gradient[0].position = Vector2f(boxX, boxY);
                gradient[1].position = Vector2f(boxX + boxWidth, boxY);
                gradient[2].position = Vector2f(boxX + boxWidth, boxY + boxHeight);
                gradient[3].position = Vector2f(boxX, boxY + boxHeight);

                gradient[0].color = Color(0, 30, 80, 240);
                gradient[1].color = Color(0, 20, 60, 240);
                gradient[2].color = Color(0, 10, 40, 240);
                gradient[3].color = Color(0, 30, 80, 240);

                window.draw(gradient);
                window.draw(infoBox);

                Text header;
                header.setFont(font);
                
                string headerText = anim.routeType.empty() ? "ROUTE DETAILS" : (anim.routeType + " ROUTE");
                header.setString(headerText);
                header.setCharacterSize(18);
                header.setFillColor(anim.routeColor);
                header.setStyle(Text::Bold);
                header.setPosition(boxX + 15.f, boxY + 10.f);
                window.draw(header);

                Text details;
                details.setFont(font);
                details.setCharacterSize(13);
                details.setFillColor(Color(200, 230, 255));
                details.setStyle(Text::Bold);
                details.setPosition(boxX + 15.f, boxY + 35.f);

                DijkstraAlgorithm dijkstra;
                int travelTime = dijkstra.calculateTravelTime(hoveredRoute->departureTime, hoveredRoute->arrivalTime);

                
                int destDockWait = getPortDockWaitMinutesByName(hoveredRoute->destinationPort);
                
                
                int baseCost = hoveredRoute->cost;
                
                
                float portCharge = 100.0f;
                float layoverCost = 0.0f;
                for (int p = 0; p < numLocations; p++) {
                    if (ports[p].portName == hoveredRoute->destinationPort) {
                        portCharge = ports[p].charge;
                        layoverCost = portCharge * (destDockWait / 1440.0f);
                        break;
                    }
                }
                
                
                int totalPathCost = anim.totalCost;
                int totalPathTime = anim.totalTime;
                
                
                int basePathCost = 0;
                int totalDockingWait = 0;
                for (int r = 0; r < anim.pathRoutes.getSize(); r++) {
                    if (anim.pathRoutes[r] != nullptr) {
                        basePathCost += anim.pathRoutes[r]->cost;
                    }
                }
                for (int pi = 0; pi < anim.path.getSize(); pi++) {
                    totalDockingWait += getPortDockWaitMinutes(anim.path[pi]);
                }
                
                
                int dockingPenalty = 0;
                for (int pi = 0; pi < anim.path.getSize(); pi++) {
                    int portIdx = anim.path[pi];
                    int waitMin = getPortDockWaitMinutes(portIdx);
                    if (waitMin > 0) {
                        string portName = portLocations[portIdx].name;
                        int graphIdx = graph.findPortIndex(portName);
                        if (graphIdx != -1) {
                            float charge = static_cast<float>(graph.ports[graphIdx].charge);
                            dockingPenalty += static_cast<int>(charge * (waitMin / 1440.0f));
                        }
                    }
                }
                int displayedTotalCost = basePathCost + dockingPenalty;

                string infoText = "From: " + hoveredRoute->startingPort + "\n" +
                                  "To: " + hoveredRoute->destinationPort + "\n" +
                                  "Searched Date: " + userSearchedDate + "\n" +
                                  "Route Valid On: " + hoveredRoute->departureDate + "\n" +
                                  "Departure: " + hoveredRoute->departureTime + "  Arrival: " + hoveredRoute->arrivalTime + "\n" +
                                  "Company: " + hoveredRoute->shippingCompany;

                
                if (hoveredSegment > 0 && hoveredSegment - 1 < anim.waitTimes.getSize())
                {
                    int waitTime = anim.waitTimes[hoveredSegment - 1];
                    infoText += "\nWait at Port: " + graph.formatWaitTime(waitTime);
                }

                details.setString(infoText);
                window.draw(details);
                
                
                RectangleShape costBox(Vector2f(boxWidth - 30, 90));
                costBox.setFillColor(Color(0, 15, 40, 200));
                costBox.setOutlineThickness(1);
                costBox.setOutlineColor(Color(100, 80, 150));
                costBox.setPosition(boxX + 15, boxY + 130);
                window.draw(costBox);
                
                Text costHeader;
                costHeader.setFont(font);
                costHeader.setString("TOTAL PATH COST BREAKDOWN");
                costHeader.setCharacterSize(12);
                costHeader.setFillColor(anim.routeColor);
                costHeader.setStyle(Text::Bold);
                costHeader.setPosition(boxX + 25, boxY + 133);
                window.draw(costHeader);
                
                Text costDetails;
                costDetails.setFont(font);
                costDetails.setCharacterSize(11);
                costDetails.setPosition(boxX + 25, boxY + 150);
                
                string costText = "Base Route Cost:     $" + to_string(basePathCost) + "\n" +
                                  "Layover Penalty:     $" + to_string(dockingPenalty) + " (charge x hrs/24)\n" +
                                  "TOTAL PATH COST:     $" + to_string(displayedTotalCost);
                costDetails.setString(costText);
                costDetails.setFillColor(Color(100, 255, 150));
                window.draw(costDetails);
                
                
                RectangleShape timeBox(Vector2f(boxWidth - 30, 90));
                timeBox.setFillColor(Color(0, 15, 40, 200));
                timeBox.setOutlineThickness(1);
                timeBox.setOutlineColor(Color(100, 150, 200));
                timeBox.setPosition(boxX + 15, boxY + 230);
                window.draw(timeBox);
                
                Text timeHeader;
                timeHeader.setFont(font);
                timeHeader.setString("TOTAL PATH TIME BREAKDOWN");
                timeHeader.setCharacterSize(12);
                timeHeader.setFillColor(Color(100, 180, 255));
                timeHeader.setStyle(Text::Bold);
                timeHeader.setPosition(boxX + 25, boxY + 233);
                window.draw(timeHeader);
                
                
                int baseTravelMinutes = totalPathTime - totalDockingWait;
                if (baseTravelMinutes < 0) baseTravelMinutes = totalPathTime;
                
                int baseDays = baseTravelMinutes / (24 * 60);
                int baseHrs = (baseTravelMinutes % (24 * 60)) / 60;
                int baseMins = baseTravelMinutes % 60;
                string baseTimeStr = to_string(baseDays) + "d " + to_string(baseHrs) + "h " + to_string(baseMins) + "m";
                
                int dockDays = totalDockingWait / (24 * 60);
                int dockHrs = (totalDockingWait % (24 * 60)) / 60;
                int dockMins = totalDockingWait % 60;
                string dockTimeStr = to_string(dockDays) + "d " + to_string(dockHrs) + "h " + to_string(dockMins) + "m";
                
                int totalDays = totalPathTime / (24 * 60);
                int totalHrs = (totalPathTime % (24 * 60)) / 60;
                int totalMins = totalPathTime % 60;
                string totalTimeStr = to_string(totalDays) + "d " + to_string(totalHrs) + "h " + to_string(totalMins) + "m";
                
                Text timeDetails;
                timeDetails.setFont(font);
                timeDetails.setCharacterSize(11);
                timeDetails.setPosition(boxX + 25, boxY + 250);
                
                string timeText = "Travel Time:         " + baseTimeStr + "\n" +
                                  "Docking Wait:        " + dockTimeStr + "\n" +
                                  "TOTAL JOURNEY:       " + totalTimeStr;
                timeDetails.setString(timeText);
                timeDetails.setFillColor(Color(100, 200, 255));
                window.draw(timeDetails);
                
                
                string algoType = anim.routeType.empty() ? "Dijkstra" : anim.routeType;
                Text algoInfo;
                algoInfo.setFont(font);
                algoInfo.setString("Algorithm: " + algoType + "  |  Segments: " + to_string(anim.pathRoutes.getSize()));
                algoInfo.setCharacterSize(11);
                algoInfo.setFillColor(Color(180, 150, 255));
                algoInfo.setPosition(boxX + 25, boxY + 330);
                window.draw(algoInfo);

                static float boxPulse = 0.0f;
                boxPulse += 0.1f;
                float pulseValue = 0.5f + 0.5f * sin(boxPulse);
                infoBox.setOutlineColor(Color(anim.routeColor.r, anim.routeColor.g, anim.routeColor.b, 150 + 105 * pulseValue));
            }

            
            Text mainTitle("", font, 24);
            mainTitle.setString("Dijkstra Algorithm - Optimal Pathfinding");
            mainTitle.setFillColor(Color(180, 150, 255)); 
            mainTitle.setStyle(Text::Bold);
            mainTitle.setPosition(Vector2f(400.f, 20.f));
            window.draw(mainTitle);

            Text escInstruction("", font, 12);
            escInstruction.setString("ESC to return to main menu");
            escInstruction.setFillColor(Color(200, 180, 255));
            escInstruction.setPosition(Vector2f(1750.f, 1050.f));
            window.draw(escInstruction);

            
            
            if (shipPrefsButton.getGlobalBounds().contains(mousePos))
            {
                shipPrefsButton.setFillColor(Color(120, 60, 180, 250));
                shipPrefsButton.setOutlineColor(Color(220, 150, 255));
            }
            else
            {
                shipPrefsButton.setFillColor(Color(80, 40, 120, 230));
                shipPrefsButton.setOutlineColor(Color(180, 100, 255));
            }
            window.draw(shipPrefsButton);
            window.draw(shipPrefsText);

            
            if (shipPrefs.filterActive)
            {
                prefsIndicator.setFillColor(Color(50, 255, 100));
            }
            else
            {
                prefsIndicator.setFillColor(Color(255, 100, 100));
            }
            window.draw(prefsIndicator);

            
            
            if (shipPrefs.filterActive)
            {
                
                int totalPorts = graph.ports.getSize();
                int avoidedCount = 0;
                for (int i = 0; i < totalPorts; i++)
                {
                    if (shipPrefs.isPortAvoided(graph.ports[i].portName))
                    {
                        avoidedCount++;
                    }
                }
                int activeCount = totalPorts - avoidedCount;

                
                RectangleShape subgraphPanel(Vector2f(280, 100));
                subgraphPanel.setFillColor(Color(30, 30, 60, 220));
                subgraphPanel.setOutlineThickness(2);
                subgraphPanel.setOutlineColor(Color(180, 100, 255));  
                subgraphPanel.setPosition(20, 950);
                window.draw(subgraphPanel);

                
                Text subgraphTitle("SUBGRAPH VIEW ACTIVE", font, 16);
                subgraphTitle.setFillColor(Color(200, 150, 255));  
                subgraphTitle.setStyle(Text::Bold);
                subgraphTitle.setPosition(35, 958);
                window.draw(subgraphTitle);

                
                Text activeText("Active Ports: " + to_string(activeCount), font, 14);
                activeText.setFillColor(Color(100, 255, 100));
                activeText.setPosition(35, 985);
                window.draw(activeText);

                
                Text avoidedText("Avoided Ports: " + to_string(avoidedCount) + " (faded)", font, 14);
                avoidedText.setFillColor(Color(255, 100, 100));
                avoidedText.setPosition(35, 1010);
                window.draw(avoidedText);

                
                CircleShape legendActive(6);
                legendActive.setFillColor(Color(100, 50, 200));  
                legendActive.setPosition(240, 988);
                window.draw(legendActive);

                CircleShape legendFaded(6);
                legendFaded.setFillColor(Color(50, 30, 100, 100));  
                legendFaded.setPosition(240, 1013);
                window.draw(legendFaded);
            }

            window.display();
        }
    }

    
    void openShipPreferencesPopup(Font &font)
    {
        RenderWindow prefWindow(VideoMode(1400, 900), "Ship Preferences - Filter Routes", Style::Close);
        prefWindow.setFramerateLimit(60);

        
        Vector<string> companies;
        companies.push_back("MaerskLine");
        companies.push_back("MSC");
        companies.push_back("COSCO");
        companies.push_back("CMA_CGM");
        companies.push_back("Evergreen");
        companies.push_back("HapagLloyd");
        companies.push_back("ONE");
        companies.push_back("YangMing");
        companies.push_back("ZIM");
        companies.push_back("PIL");

        
        Color companyColors[] = {
            Color(0, 91, 187),  
            Color(255, 204, 0), 
            Color(0, 51, 102),  
            Color(0, 114, 198), 
            Color(0, 128, 0),   
            Color(255, 102, 0), 
            Color(255, 0, 127), 
            Color(255, 165, 0), 
            Color(0, 102, 153), 
            Color(153, 0, 0)    
        };

        
        Vector<string> allPorts;
        for (int i = 0; i < numLocations; i++)
        {
            allPorts.push_back(portLocations[i].name);
        }

        
        int portScrollOffset = 0;
        const int maxVisiblePorts = 10;  
        const int portItemHeight = 48;

        
        string maxTimeInput = "";
        if (shipPrefs.maxVoyageMinutes > 0)
            maxTimeInput = to_string(shipPrefs.maxVoyageMinutes / 60);
        bool maxTimeActive = false;

        Clock animClock;

        
        struct Particle
        {
            float x, y, speed, size;
            Color color;
        };
        Vector<Particle> particles;
        for (int i = 0; i < 60; i++)
        {
            Particle p;
            p.x = rand() % 1400;
            p.y = rand() % 900;
            p.speed = 0.2f + (rand() % 100) / 200.0f;
            p.size = 1 + rand() % 3;
            p.color = Color(100 + rand() % 100, 150 + rand() % 100, 255, 80 + rand() % 80);
            particles.push_back(p);
        }

        while (prefWindow.isOpen())
        {
            float time = animClock.getElapsedTime().asSeconds();
            Vector2i mousePos = Mouse::getPosition(prefWindow);
            Vector2f mouse(mousePos.x, mousePos.y);

            Event e;
            while (prefWindow.pollEvent(e))
            {
                if (e.type == Event::Closed)
                    prefWindow.close();

                if (e.type == Event::KeyPressed)
                {
                    if (e.key.code == Keyboard::Escape)
                        prefWindow.close();
                }

                
                if (e.type == Event::MouseWheelScrolled)
                {
                    
                    if (mouse.x >= 460 && mouse.x <= 860 && mouse.y >= 140 && mouse.y <= 820)
                    {
                        portScrollOffset -= (int)(e.mouseWheelScroll.delta * 2);
                        
                        int maxScroll = max(0, allPorts.getSize() - maxVisiblePorts);
                        if (portScrollOffset < 0) portScrollOffset = 0;
                        if (portScrollOffset > maxScroll) portScrollOffset = maxScroll;
                    }
                }

                
                if (e.type == Event::TextEntered && maxTimeActive)
                {
                    if (e.text.unicode == '\b' && maxTimeInput.length() > 0)
                    {
                        maxTimeInput.pop_back();
                    }
                    else if (e.text.unicode >= '0' && e.text.unicode <= '9' && maxTimeInput.length() < 5)
                    {
                        maxTimeInput += static_cast<char>(e.text.unicode);
                    }
                    
                    if (!maxTimeInput.empty())
                        shipPrefs.maxVoyageMinutes = stoi(maxTimeInput) * 60;
                    else
                        shipPrefs.maxVoyageMinutes = 0;
                }

                
                if (e.type == Event::MouseButtonPressed && e.mouseButton.button == Mouse::Button::Left)
                {
                    
                    for (int c = 0; c < companies.getSize(); c++)
                    {
                        FloatRect companyBounds(50, 180 + c * 55, 350, 50);
                        if (companyBounds.contains(mouse))
                        {
                            string company = companies[c];
                            bool isSelected = false;
                            for (int i = 0; i < shipPrefs.preferredCompanies.getSize(); i++)
                            {
                                if (shipPrefs.preferredCompanies[i] == company)
                                {
                                    isSelected = true;
                                    break;
                                }
                            }

                            if (isSelected)
                            {
                                shipPrefs.removePreferredCompany(company);
                                if (shipPrefs.preferredCompanies.getSize() == 0)
                                    shipPrefs.filterActive = false;
                            }
                            else
                            {
                                shipPrefs.addPreferredCompany(company);
                                shipPrefs.filterActive = true;
                            }
                            break;
                        }
                    }

                    
                    
                    if (mouse.x >= 460 && mouse.x <= 860 && mouse.y >= 195 && mouse.y <= 680)
                    {
                        for (int p = 0; p < maxVisiblePorts && (p + portScrollOffset) < allPorts.getSize(); p++)
                        {
                            int actualIndex = p + portScrollOffset;
                            FloatRect portBounds(480, 195 + p * portItemHeight, 350, portItemHeight - 3);
                            if (portBounds.contains(mouse))
                            {
                                string port = allPorts[actualIndex];
                                bool isAvoided = false;
                                for (int i = 0; i < shipPrefs.avoidedPorts.getSize(); i++)
                                {
                                    if (shipPrefs.avoidedPorts[i] == port)
                                    {
                                        isAvoided = true;
                                        break;
                                    }
                                }

                                if (isAvoided)
                                {
                                    shipPrefs.removeAvoidedPort(port);
                                }
                                else
                                {
                                    shipPrefs.addAvoidedPort(port);
                                    shipPrefs.filterActive = true;
                                }
                                break;
                            }
                        }
                    }

                    
                    FloatRect timeInputBounds(950, 250, 200, 50);
                    maxTimeActive = timeInputBounds.contains(mouse);

                    
                    FloatRect clearBounds(950, 700, 180, 50);
                    if (clearBounds.contains(mouse))
                    {
                        clickSound.play();
                        
                        while (shipPrefs.preferredCompanies.getSize() > 0)
                            shipPrefs.preferredCompanies.pop();
                        while (shipPrefs.avoidedPorts.getSize() > 0)
                            shipPrefs.avoidedPorts.pop();
                        shipPrefs.maxVoyageMinutes = 0;
                        shipPrefs.filterActive = false;
                        maxTimeInput = "";
                    }

                    
                    FloatRect applyBounds(1150, 700, 180, 50);
                    if (applyBounds.contains(mouse))
                    {
                        clickSound.play();
                        prefWindow.close();
                    }
                }
            }

            
            RectangleShape bgTop(Vector2f(1400, 450));
            bgTop.setFillColor(Color(8, 20, 45));
            bgTop.setPosition(0, 0);
            prefWindow.draw(bgTop);

            RectangleShape bgBottom(Vector2f(1400, 450));
            bgBottom.setFillColor(Color(15, 35, 70));
            bgBottom.setPosition(0, 450);
            prefWindow.draw(bgBottom);

            
            for (int i = 0; i < particles.getSize(); i++)
            {
                particles[i].y -= particles[i].speed;
                if (particles[i].y < -10)
                {
                    particles[i].y = 910;
                    particles[i].x = rand() % 1400;
                }

                float twinkle = 0.5f + 0.5f * sin(time * 3.0f + i);
                CircleShape star(particles[i].size);
                star.setFillColor(Color(
                    particles[i].color.r,
                    particles[i].color.g,
                    particles[i].color.b,
                    (Uint8)(particles[i].color.a * twinkle)));
                star.setPosition(particles[i].x, particles[i].y);
                prefWindow.draw(star);
            }

            
            for (int w = 0; w < 3; w++)
            {
                for (int i = 0; i < 30; i++)
                {
                    float waveY = 850 + w * 15 + sin(time * 1.5f + i * 0.3f + w) * 10;
                    CircleShape wave(5 - w);
                    wave.setFillColor(Color(30 + w * 15, 80 + w * 20, 150 + w * 25, 80 - w * 20));
                    wave.setPosition(i * 50 + fmod(time * 15, 50.0f), waveY);
                    prefWindow.draw(wave);
                }
            }

            
            
            RectangleShape headerBg(Vector2f(1400, 80));
            headerBg.setFillColor(Color(10, 30, 60, 230));
            headerBg.setPosition(0, 0);
            prefWindow.draw(headerBg);

            
            Text title("SHIP PREFERENCES", font, 36);
            title.setStyle(Text::Bold);
            title.setFillColor(Color(100, 200, 255));
            title.setLetterSpacing(3);
            FloatRect titleBounds = title.getLocalBounds();
            title.setPosition(700 - titleBounds.width / 2, 15);
            prefWindow.draw(title);

            Text subtitle("Filter routes by company, avoid ports, set time limits", font, 16);
            subtitle.setFillColor(Color(150, 180, 220));
            FloatRect subBounds = subtitle.getLocalBounds();
            subtitle.setPosition(700 - subBounds.width / 2, 55);
            prefWindow.draw(subtitle);

            
            RectangleShape statusBox(Vector2f(200, 35));
            statusBox.setFillColor(shipPrefs.filterActive ? Color(0, 80, 50, 200) : Color(80, 40, 40, 200));
            statusBox.setOutlineThickness(2);
            statusBox.setOutlineColor(shipPrefs.filterActive ? Color(0, 255, 150) : Color(255, 100, 100));
            statusBox.setPosition(1180, 95);
            prefWindow.draw(statusBox);

            Text statusText(shipPrefs.filterActive ? "FILTER ACTIVE" : "NO FILTER", font, 14);
            statusText.setFillColor(shipPrefs.filterActive ? Color(100, 255, 150) : Color(255, 150, 150));
            statusText.setStyle(Text::Bold);
            statusText.setPosition(1210, 102);
            prefWindow.draw(statusText);

            
            RectangleShape companyPanel(Vector2f(380, 680));
            companyPanel.setFillColor(Color(15, 30, 55, 220));
            companyPanel.setOutlineThickness(2);
            companyPanel.setOutlineColor(Color(0, 150, 200, 150));
            companyPanel.setPosition(30, 140);
            prefWindow.draw(companyPanel);

            
            RectangleShape companyHeader(Vector2f(380, 45));
            companyHeader.setFillColor(Color(0, 80, 120, 200));
            companyHeader.setPosition(30, 140);
            prefWindow.draw(companyHeader);

            
            CircleShape shipIcon(15, 3);
            shipIcon.setFillColor(Color(100, 200, 255));
            shipIcon.setRotation(90);
            shipIcon.setPosition(70, 148);
            prefWindow.draw(shipIcon);

            Text companyTitle("PREFERRED COMPANIES", font, 16);
            companyTitle.setFillColor(Color(200, 240, 255));
            companyTitle.setStyle(Text::Bold);
            companyTitle.setPosition(95, 152);
            prefWindow.draw(companyTitle);

            
            for (int c = 0; c < companies.getSize(); c++)
            {
                bool isSelected = false;
                for (int i = 0; i < shipPrefs.preferredCompanies.getSize(); i++)
                {
                    if (shipPrefs.preferredCompanies[i] == companies[c])
                    {
                        isSelected = true;
                        break;
                    }
                }

                FloatRect bounds(50, 195 + c * 55, 340, 48);
                bool isHovered = bounds.contains(mouse);

                
                RectangleShape companyCard(Vector2f(340, 48));
                if (isSelected)
                {
                    companyCard.setFillColor(Color(companyColors[c].r / 3, companyColors[c].g / 3, companyColors[c].b / 3, 220));
                    companyCard.setOutlineColor(companyColors[c]);
                }
                else if (isHovered)
                {
                    companyCard.setFillColor(Color(40, 60, 90, 200));
                    companyCard.setOutlineColor(Color(100, 150, 200));
                }
                else
                {
                    companyCard.setFillColor(Color(25, 40, 65, 180));
                    companyCard.setOutlineColor(Color(60, 80, 110));
                }
                companyCard.setOutlineThickness(2);
                companyCard.setPosition(50, 195 + c * 55);
                prefWindow.draw(companyCard);

                
                RectangleShape colorBar(Vector2f(8, 48));
                colorBar.setFillColor(companyColors[c]);
                colorBar.setPosition(50, 195 + c * 55);
                prefWindow.draw(colorBar);

                
                RectangleShape checkbox(Vector2f(24, 24));
                checkbox.setFillColor(isSelected ? companyColors[c] : Color(40, 50, 70));
                checkbox.setOutlineThickness(2);
                checkbox.setOutlineColor(isSelected ? Color::White : Color(100, 120, 150));
                checkbox.setPosition(70, 207 + c * 55);
                prefWindow.draw(checkbox);

                if (isSelected)
                {
                    Text checkmark("v", font, 18);
                    checkmark.setFillColor(Color::White);
                    checkmark.setStyle(Text::Bold);
                    checkmark.setPosition(75, 203 + c * 55);
                    prefWindow.draw(checkmark);
                }

                
                Text companyName(companies[c], font, 16);
                companyName.setFillColor(isSelected ? Color::White : Color(180, 200, 220));
                companyName.setStyle(isSelected ? Text::Bold : Text::Regular);
                companyName.setPosition(110, 208 + c * 55);
                prefWindow.draw(companyName);
            }

            
            RectangleShape portPanel(Vector2f(380, 680));
            portPanel.setFillColor(Color(15, 30, 55, 220));
            portPanel.setOutlineThickness(2);
            portPanel.setOutlineColor(Color(200, 100, 50, 150));
            portPanel.setPosition(460, 140);
            prefWindow.draw(portPanel);

            
            RectangleShape portHeader(Vector2f(380, 45));
            portHeader.setFillColor(Color(120, 60, 30, 200));
            portHeader.setPosition(460, 140);
            prefWindow.draw(portHeader);

            
            CircleShape warnIcon(12, 3);
            warnIcon.setFillColor(Color(255, 180, 80));
            warnIcon.setPosition(495, 150);
            prefWindow.draw(warnIcon);

            Text portTitle("PORTS TO AVOID", font, 16);
            portTitle.setFillColor(Color(255, 220, 180));
            portTitle.setStyle(Text::Bold);
            portTitle.setPosition(525, 152);
            prefWindow.draw(portTitle);

            
            Text scrollInfo("Scroll: " + to_string(portScrollOffset + 1) + "-" + 
                           to_string(min(portScrollOffset + maxVisiblePorts, (int)allPorts.getSize())) + 
                           " of " + to_string(allPorts.getSize()), font, 11);
            scrollInfo.setFillColor(Color(180, 150, 120));
            scrollInfo.setPosition(700, 155);
            prefWindow.draw(scrollInfo);

            
            for (int p = 0; p < maxVisiblePorts && (p + portScrollOffset) < allPorts.getSize(); p++)
            {
                int actualIndex = p + portScrollOffset;
                
                bool isAvoided = false;
                for (int i = 0; i < shipPrefs.avoidedPorts.getSize(); i++)
                {
                    if (shipPrefs.avoidedPorts[i] == allPorts[actualIndex])
                    {
                        isAvoided = true;
                        break;
                    }
                }

                FloatRect bounds(480, 195 + p * portItemHeight, 340, portItemHeight - 3);
                bool isHovered = bounds.contains(mouse);

                
                RectangleShape portCard(Vector2f(340, portItemHeight - 3));
                if (isAvoided)
                {
                    portCard.setFillColor(Color(100, 40, 40, 220));
                    portCard.setOutlineColor(Color(255, 100, 100));
                }
                else if (isHovered)
                {
                    portCard.setFillColor(Color(60, 50, 40, 200));
                    portCard.setOutlineColor(Color(200, 150, 100));
                }
                else
                {
                    portCard.setFillColor(Color(35, 35, 50, 180));
                    portCard.setOutlineColor(Color(80, 70, 60));
                }
                portCard.setOutlineThickness(2);
                portCard.setPosition(480, 195 + p * portItemHeight);
                prefWindow.draw(portCard);

                
                RectangleShape portCheckbox(Vector2f(22, 22));
                portCheckbox.setFillColor(isAvoided ? Color(200, 50, 50) : Color(50, 50, 60));
                portCheckbox.setOutlineThickness(2);
                portCheckbox.setOutlineColor(isAvoided ? Color(255, 150, 150) : Color(100, 100, 120));
                portCheckbox.setPosition(495, 205 + p * portItemHeight);
                prefWindow.draw(portCheckbox);

                if (isAvoided)
                {
                    Text xMark("X", font, 14);
                    xMark.setFillColor(Color::White);
                    xMark.setStyle(Text::Bold);
                    xMark.setPosition(500, 203 + p * portItemHeight);
                    prefWindow.draw(xMark);
                }

                
                Text portName(allPorts[actualIndex], font, 14);
                portName.setFillColor(isAvoided ? Color(255, 180, 180) : Color(200, 200, 210));
                portName.setStyle(isAvoided ? Text::Bold : Text::Regular);
                portName.setPosition(530, 207 + p * portItemHeight);
                prefWindow.draw(portName);
            }

            
            if (allPorts.getSize() > maxVisiblePorts)
            {
                
                RectangleShape scrollTrack(Vector2f(8, 480));
                scrollTrack.setFillColor(Color(40, 40, 60, 150));
                scrollTrack.setPosition(822, 195);
                prefWindow.draw(scrollTrack);

                
                float thumbHeight = 480.0f * maxVisiblePorts / allPorts.getSize();
                float thumbPos = 195 + (480.0f - thumbHeight) * portScrollOffset / (allPorts.getSize() - maxVisiblePorts);
                RectangleShape scrollThumb(Vector2f(8, thumbHeight));
                scrollThumb.setFillColor(Color(200, 100, 50, 200));
                scrollThumb.setPosition(822, thumbPos);
                prefWindow.draw(scrollThumb);
            }

            
            Text scrollHint("Use mouse wheel to scroll", font, 10);
            scrollHint.setFillColor(Color(150, 120, 100));
            scrollHint.setPosition(530, 690);
            prefWindow.draw(scrollHint);

            
            RectangleShape timePanel(Vector2f(440, 320));
            timePanel.setFillColor(Color(15, 30, 55, 220));
            timePanel.setOutlineThickness(2);
            timePanel.setOutlineColor(Color(100, 200, 150, 150));
            timePanel.setPosition(910, 140);
            prefWindow.draw(timePanel);

            
            RectangleShape timeHeader(Vector2f(440, 45));
            timeHeader.setFillColor(Color(30, 100, 70, 200));
            timeHeader.setPosition(910, 140);
            prefWindow.draw(timeHeader);

            
            CircleShape clockIcon(12);
            clockIcon.setFillColor(Color(100, 255, 180));
            clockIcon.setOutlineThickness(2);
            clockIcon.setOutlineColor(Color(200, 255, 220));
            clockIcon.setPosition(940, 150);
            prefWindow.draw(clockIcon);

            Text timeTitle("MAX VOYAGE TIME", font, 16);
            timeTitle.setFillColor(Color(180, 255, 220));
            timeTitle.setStyle(Text::Bold);
            timeTitle.setPosition(975, 152);
            prefWindow.draw(timeTitle);

            
            Text timeDesc("Set maximum total travel time (hours):", font, 14);
            timeDesc.setFillColor(Color(150, 200, 180));
            timeDesc.setPosition(940, 210);
            prefWindow.draw(timeDesc);

            
            RectangleShape timeInputBox(Vector2f(200, 50));
            timeInputBox.setFillColor(Color(20, 40, 60, 240));
            timeInputBox.setOutlineThickness(3);
            timeInputBox.setOutlineColor(maxTimeActive ? Color(100, 255, 180) : Color(60, 120, 100));
            timeInputBox.setPosition(940, 250);
            prefWindow.draw(timeInputBox);

            Text timeInputText(maxTimeInput.empty() ? "No limit" : maxTimeInput + " hours", font, 20);
            timeInputText.setFillColor(maxTimeInput.empty() ? Color(100, 120, 140) : Color::White);
            timeInputText.setPosition(960, 260);
            prefWindow.draw(timeInputText);

            
            if (maxTimeActive)
            {
                float blink = fmod(time, 1.0f);
                if (blink < 0.5f)
                {
                    RectangleShape cursor(Vector2f(2, 30));
                    cursor.setFillColor(Color(100, 255, 180));
                    cursor.setPosition(960 + timeInputText.getLocalBounds().width + 5, 260);
                    prefWindow.draw(cursor);
                }
            }

            
            Text helperText("Leave empty for unlimited voyage time", font, 12);
            helperText.setFillColor(Color(100, 140, 130));
            helperText.setPosition(940, 320);
            prefWindow.draw(helperText);

            
            if (shipPrefs.maxVoyageMinutes > 0)
            {
                int hours = shipPrefs.maxVoyageMinutes / 60;
                int mins = shipPrefs.maxVoyageMinutes % 60;
                string timeStr = to_string(hours) + "h " + to_string(mins) + "m max";

                Text currentTime("Current: " + timeStr, font, 14);
                currentTime.setFillColor(Color(100, 255, 180));
                currentTime.setStyle(Text::Bold);
                currentTime.setPosition(940, 360);
                prefWindow.draw(currentTime);
            }

            
            RectangleShape summaryPanel(Vector2f(440, 310));
            summaryPanel.setFillColor(Color(20, 35, 60, 220));
            summaryPanel.setOutlineThickness(2);
            summaryPanel.setOutlineColor(Color(150, 150, 200, 150));
            summaryPanel.setPosition(910, 480);
            prefWindow.draw(summaryPanel);

            Text summaryTitle("ACTIVE FILTERS SUMMARY", font, 14);
            summaryTitle.setFillColor(Color(180, 180, 220));
            summaryTitle.setStyle(Text::Bold);
            summaryTitle.setPosition(940, 495);
            prefWindow.draw(summaryTitle);

            int summaryY = 530;

            
            Text compSummary("Companies: ", font, 13);
            compSummary.setFillColor(Color(100, 200, 255));
            compSummary.setPosition(930, summaryY);
            prefWindow.draw(compSummary);

            string compList = "";
            if (shipPrefs.preferredCompanies.getSize() == 0)
                compList = "All (no filter)";
            else
            {
                for (int i = 0; i < shipPrefs.preferredCompanies.getSize() && i < 3; i++)
                {
                    if (i > 0)
                        compList += ", ";
                    compList += shipPrefs.preferredCompanies[i];
                }
                if (shipPrefs.preferredCompanies.getSize() > 3)
                    compList += " +" + to_string(shipPrefs.preferredCompanies.getSize() - 3) + " more";
            }
            Text compListText(compList, font, 12);
            compListText.setFillColor(Color(150, 180, 200));
            compListText.setPosition(930, summaryY + 20);
            prefWindow.draw(compListText);

            
            Text portSummary("Avoided Ports: ", font, 13);
            portSummary.setFillColor(Color(255, 150, 100));
            portSummary.setPosition(930, summaryY + 55);
            prefWindow.draw(portSummary);

            string portList = "";
            if (shipPrefs.avoidedPorts.getSize() == 0)
                portList = "None";
            else
            {
                for (int i = 0; i < shipPrefs.avoidedPorts.getSize() && i < 3; i++)
                {
                    if (i > 0)
                        portList += ", ";
                    portList += shipPrefs.avoidedPorts[i];
                }
                if (shipPrefs.avoidedPorts.getSize() > 3)
                    portList += " +" + to_string(shipPrefs.avoidedPorts.getSize() - 3) + " more";
            }
            Text portListText(portList, font, 12);
            portListText.setFillColor(Color(200, 150, 130));
            portListText.setPosition(930, summaryY + 75);
            prefWindow.draw(portListText);

            
            Text timeSummary("Time Limit: ", font, 13);
            timeSummary.setFillColor(Color(100, 255, 180));
            timeSummary.setPosition(930, summaryY + 110);
            prefWindow.draw(timeSummary);

            string timeLimit = shipPrefs.maxVoyageMinutes > 0 ? to_string(shipPrefs.maxVoyageMinutes / 60) + " hours" : "No limit";
            Text timeLimitText(timeLimit, font, 12);
            timeLimitText.setFillColor(Color(150, 200, 170));
            timeLimitText.setPosition(930, summaryY + 130);
            prefWindow.draw(timeLimitText);

            
            
            FloatRect clearBounds(950, 700, 180, 50);
            bool clearHover = clearBounds.contains(mouse);

            RectangleShape clearButton(Vector2f(180, 50));
            clearButton.setFillColor(clearHover ? Color(180, 80, 80) : Color(120, 50, 50));
            clearButton.setOutlineThickness(2);
            clearButton.setOutlineColor(clearHover ? Color(255, 150, 150) : Color(200, 100, 100));
            clearButton.setPosition(950, 700);
            prefWindow.draw(clearButton);

            Text clearText("CLEAR ALL", font, 16);
            clearText.setFillColor(Color::White);
            clearText.setStyle(Text::Bold);
            clearText.setPosition(985, 712);
            prefWindow.draw(clearText);

            
            FloatRect applyBounds(1150, 700, 180, 50);
            bool applyHover = applyBounds.contains(mouse);

            RectangleShape applyButton(Vector2f(180, 50));
            applyButton.setFillColor(applyHover ? Color(50, 180, 120) : Color(30, 120, 80));
            applyButton.setOutlineThickness(2);
            applyButton.setOutlineColor(applyHover ? Color(150, 255, 200) : Color(100, 200, 150));
            applyButton.setPosition(1150, 700);
            prefWindow.draw(applyButton);

            Text applyText("APPLY & CLOSE", font, 14);
            applyText.setFillColor(Color::White);
            applyText.setStyle(Text::Bold);
            applyText.setPosition(1175, 715);
            prefWindow.draw(applyText);

            
            Text footerText("Press ESC or click APPLY & CLOSE to return | Filters are applied to route calculations", font, 12);
            footerText.setFillColor(Color(100, 130, 160));
            FloatRect footerBounds = footerText.getLocalBounds();
            footerText.setPosition(700 - footerBounds.width / 2, 860);
            prefWindow.draw(footerText);

            prefWindow.display();
        }
    }

    
    void showMap(Maps &graph)
    {
        
        VideoMode desktop = VideoMode::getDesktopMode();
        RenderWindow window(VideoMode(1920, 1080), "OceanRoute Navigator");
        window.setFramerateLimit(60);
        
        
        View fixedView(FloatRect(0, 0, 1920, 1080));
        window.setView(fixedView);
        
        
        
        
        {
            
            int sgIdx = -1;
            for (int p = 0; p < numLocations; p++) {
                if (string(portLocations[p].name) == "Singapore") { sgIdx = p; break; }
            }
            if (sgIdx >= 0 && globalPortDocking[sgIdx].dockedShips.getSize() == 0) {
                
                GlobalDockShipInfo ship1 = {"ZIM-2812", "ZIM", 3600, 7200, 0};
                GlobalDockShipInfo ship2 = {"COSCO-0612", "COSCO", 5400, 7200, 1};
                GlobalDockShipInfo ship3 = {"PIL-1912", "PIL", 1800, 7200, 2}; 
                globalPortDocking[sgIdx].dockedShips.push_back(ship1);
                globalPortDocking[sgIdx].dockedShips.push_back(ship2);
                globalPortDocking[sgIdx].dockedShips.push_back(ship3);
                
                GlobalQueueShipInfo q1 = {"EVG-1212", "Evergreen", 1};
                GlobalQueueShipInfo q2 = {"CMA-0812", "CMA_CGM", 2};
                globalPortDocking[sgIdx].queuedShips.push_back(q1);
                globalPortDocking[sgIdx].queuedShips.push_back(q2);
            }
            
            
            int dbIdx = -1;
            for (int p = 0; p < numLocations; p++) {
                if (string(portLocations[p].name) == "Dubai") { dbIdx = p; break; }
            }
            if (dbIdx >= 0 && globalPortDocking[dbIdx].dockedShips.getSize() == 0) {
                GlobalDockShipInfo dship1 = {"MSC-0912", "MSC", 4200, 7200, 0};
                GlobalDockShipInfo dship2 = {"HPL-2712", "HapagLloyd", 2100, 7200, 1};
                globalPortDocking[dbIdx].dockedShips.push_back(dship1);
                globalPortDocking[dbIdx].dockedShips.push_back(dship2);
                GlobalQueueShipInfo dq1 = {"COSCO-1312", "COSCO", 1};
                globalPortDocking[dbIdx].queuedShips.push_back(dq1);
            }
            
            
            int jdIdx = -1;
            for (int p = 0; p < numLocations; p++) {
                if (string(portLocations[p].name) == "Jeddah") { jdIdx = p; break; }
            }
            if (jdIdx >= 0 && globalPortDocking[jdIdx].dockedShips.getSize() == 0) {
                GlobalDockShipInfo jship1 = {"EVG-2212", "Evergreen", 2000, 7200, 0};
                GlobalDockShipInfo jship2 = {"YM-1512", "YangMing", 4000, 7200, 1};
                GlobalDockShipInfo jship3 = {"YM-0512", "YangMing", 3500, 7200, 2};
                GlobalDockShipInfo jship4 = {"EVG-2512", "Evergreen", 1000, 7200, 3}; 
                globalPortDocking[jdIdx].dockedShips.push_back(jship1);
                globalPortDocking[jdIdx].dockedShips.push_back(jship2);
                globalPortDocking[jdIdx].dockedShips.push_back(jship3);
                globalPortDocking[jdIdx].dockedShips.push_back(jship4);
                GlobalQueueShipInfo jq1 = {"PIL-0212", "PIL", 1};  
                globalPortDocking[jdIdx].queuedShips.push_back(jq1);
            }
            
            
            int hbIdx = -1;
            for (int p = 0; p < numLocations; p++) {
                if (string(portLocations[p].name) == "Hamburg") { hbIdx = p; break; }
            }
            if (hbIdx >= 0 && globalPortDocking[hbIdx].dockedShips.getSize() == 0) {
                GlobalDockShipInfo hship1 = {"HPL-1412", "HapagLloyd", 5000, 7200, 0};
                GlobalDockShipInfo hship2 = {"YM-0612", "YangMing", 3000, 7200, 1};
                globalPortDocking[hbIdx].dockedShips.push_back(hship1);
                globalPortDocking[hbIdx].dockedShips.push_back(hship2);
            }
            
            
            int mlIdx = -1;
            for (int p = 0; p < numLocations; p++) {
                if (string(portLocations[p].name) == "Melbourne") { mlIdx = p; break; }
            }
            if (mlIdx >= 0 && globalPortDocking[mlIdx].dockedShips.getSize() == 0) {
                GlobalDockShipInfo mship1 = {"MSK-0912", "MaerskLine", 4500, 7200, 0};
                GlobalDockShipInfo mship2 = {"CMA-0912", "CMA_CGM", 2500, 7200, 1};
                GlobalDockShipInfo mship3 = {"ZIM-1712", "ZIM", 3200, 7200, 2}; 
                globalPortDocking[mlIdx].dockedShips.push_back(mship1);
                globalPortDocking[mlIdx].dockedShips.push_back(mship2);
                globalPortDocking[mlIdx].dockedShips.push_back(mship3);
                GlobalQueueShipInfo mq1 = {"ZIM-2212", "ZIM", 1}; 
                globalPortDocking[mlIdx].queuedShips.push_back(mq1);
            }
        }

        Texture mapTexture;
        if (!mapTexture.loadFromFile("pics/map2.png"))
        {
            cout << "Map image not loaded!" << endl;
            return;
        }
        Sprite mapSprite(mapTexture);
        mapSprite.setColor(Color(140, 180, 220, 180));  

        Font font;
        if (!font.loadFromFile("Roboto.ttf"))
        {
            cout << "Font load failed!" << endl;
            return;
        }
        
        
        Vector<int> foundPath;
        Vector<RouteNode*> foundPathRoutes;
        Vector<string> foundPathWaitPorts;
        Vector<string> foundPathWaitDurations;
        Vector<int> foundPathWaitTimes;
        Color foundPathColor;
        string foundPathLabel;
        int foundPathCost = 0;
        int foundPathTime = 0;
        string userSearchedDate = "";  

        struct RouteAnimation
        {
            Vector<int> path;
            Vector<string> waitPorts;
            Vector<string> waitDurations;
            Color routeColor;
            string routeType;
            int currentSegment;
            float segmentProgress;
            bool isPaused;
            float pauseTimer;
            bool isComplete;
            Vector<RouteNode *> pathRoutes;
            Vector<int> waitTimes;
            int totalCost;
            int totalTime;
        };

        Vector<RouteAnimation> activeAnimations;
        float animationSpeed = 0.8f;
        const float SEGMENT_PAUSE = 1.5f;

        Clock clock;
        bool useCost = true;

        
        RectangleShape searchButton(Vector2f(220, 60));
        searchButton.setFillColor(Color(80, 20, 30, 230));
        searchButton.setOutlineThickness(3);
        searchButton.setOutlineColor(Color(200, 50, 50));
        searchButton.setPosition(40, 950);

        CircleShape buttonGlow(40);
        buttonGlow.setFillColor(Color(200, 50, 50, 60));
        buttonGlow.setPosition(10, 1040);

        Text searchButtonText;
        searchButtonText.setFont(font);
        searchButtonText.setString("Search Route");
        searchButtonText.setCharacterSize(22);
        searchButtonText.setFillColor(Color(255, 200, 200));
        searchButtonText.setStyle(Text::Bold);
        searchButtonText.setPosition(55, 965);

        
        RectangleShape dockingButton(Vector2f(220, 60));
        dockingButton.setFillColor(Color(60, 15, 25, 230));
        dockingButton.setOutlineThickness(3);
        dockingButton.setOutlineColor(Color(180, 60, 60));
        dockingButton.setPosition(280, 950);

        Text dockingButtonText;
        dockingButtonText.setFont(font);
        dockingButtonText.setString("Port Docking");
        dockingButtonText.setCharacterSize(22);
        dockingButtonText.setFillColor(Color(255, 180, 180));
        dockingButtonText.setStyle(Text::Bold);
        dockingButtonText.setPosition(305, 965);

        
        RectangleShape shortestPathButton(Vector2f(220, 60));
        shortestPathButton.setFillColor(Color(100, 25, 35, 230));
        shortestPathButton.setOutlineThickness(3);
        shortestPathButton.setOutlineColor(Color(220, 80, 80));
        shortestPathButton.setPosition(520, 950);

        Text shortestPathButtonText;
        shortestPathButtonText.setFont(font);
        shortestPathButtonText.setString("Shortest Path");
        shortestPathButtonText.setCharacterSize(22);
        shortestPathButtonText.setFillColor(Color(255, 200, 200));
        shortestPathButtonText.setStyle(Text::Bold);
        shortestPathButtonText.setPosition(540, 965);

        
        RectangleShape shipPrefsButton(Vector2f(220, 50));
        shipPrefsButton.setFillColor(Color(100, 30, 40, 230));
        shipPrefsButton.setOutlineThickness(3);
        shipPrefsButton.setOutlineColor(Color(220, 100, 100));
        shipPrefsButton.setPosition(1680, 20);

        Text shipPrefsButtonText;
        shipPrefsButtonText.setFont(font);
        shipPrefsButtonText.setString("Ship Preferences");
        shipPrefsButtonText.setCharacterSize(18);
        shipPrefsButtonText.setFillColor(Color(255, 200, 200));
        shipPrefsButtonText.setStyle(Text::Bold);
        shipPrefsButtonText.setPosition(1705, 32);

        
        bool showShortestPathMode = false;
        bool showAlgorithmSelection = true;  
        int selectedAlgorithm = 0;  
        bool useCostMode = true;  
        
        
        bool showShortestPathInput = false;
        string spOrigin = "", spDestination = "", spDate = "";
        bool spOriginActive = false, spDestActive = false, spDateActive = false;
        
        
        Vector<int> shortestPath;
        Vector<string> spWaitPorts, spWaitDurations;
        int spTotalCost = 0, spTotalTime = 0;
        bool spPathFound = false;
        string spResultText = "";
        Color spResultColor = Color::White;

        
        bool showDockingMode = false;
        int selectedDockingPort = -1;  
        bool showPortSelection = true; 
        
        
        struct DockShipInfo {
            string shipId;
            string company;
            float dockTimer;
            float maxTime;
            float animProgress;
            bool isArriving;
            bool isDeparting;
            int dockSlot;
        };
        
        struct QueueShipInfo {
            string shipId;
            string company;
            int queuePosition;
            float waitAnimOffset;
        };
        
        
        Vector<DockShipInfo> arrivingShips;
        Vector<DockShipInfo> departingShips;
        
        int dockShipCounter = 1000;
        const int MAX_DOCKS = 4;
        const int MAX_QUEUE = 6;
        float dockTimeMultiplier = 1.0f;
        
        string dockCompanies[] = {"Maersk", "MSC", "COSCO", "CMA-CGM", "Evergreen", "Hapag", "ONE", "YangMing"};
        Color dockCompanyColors[] = {
            Color(0, 100, 200), Color(255, 200, 0), Color(200, 50, 50), Color(0, 150, 100),
            Color(50, 180, 50), Color(255, 100, 0), Color(200, 0, 150), Color(100, 100, 200)
        };
        
        
        bool showAddShipDialog = false;
        string dockInputShipName = "";
        string dockInputCompany = "";
        string dockInputTime = "30";
        bool dockNameActive = false, dockCompanyActive = false, dockTimeActive = false;
        bool dockAddBtnClicked = false, dockConfirmClicked = false, dockCancelClicked = false;
        
        
        float portScrollOffset = 0.0f;
        
        
        int hoveredPortIdx = -1;  

        bool showInputWindow = false;
        
        
        RectangleShape inputWindowGlow(Vector2f(520, 570));
        inputWindowGlow.setFillColor(Color(180, 30, 30, 80));
        inputWindowGlow.setPosition(700, 255);
        
        RectangleShape inputWindow(Vector2f(500, 550));
        inputWindow.setFillColor(Color(15, 5, 8, 250));
        inputWindow.setOutlineThickness(4);
        inputWindow.setOutlineColor(Color(200, 40, 40));
        inputWindow.setPosition(710, 265);
        
        
        RectangleShape inputWindowInner(Vector2f(490, 540));
        inputWindowInner.setFillColor(Color::Transparent);
        inputWindowInner.setOutlineThickness(1);
        inputWindowInner.setOutlineColor(Color(255, 80, 80, 100));
        inputWindowInner.setPosition(715, 270);

        Text windowTitle;
        windowTitle.setFont(font);
        windowTitle.setString("ROUTE SEARCH");
        windowTitle.setCharacterSize(32);
        windowTitle.setFillColor(Color(255, 60, 60));
        windowTitle.setStyle(Text::Bold);
        windowTitle.setOutlineThickness(3);
        windowTitle.setOutlineColor(Color(100, 0, 0));
        windowTitle.setLetterSpacing(1.5f);
        windowTitle.setPosition(800, 280);

        class EnhancedInputBox
        {
        public:
            RectangleShape box;
            Text text;
            Text label;
            string inputString;
            bool isActive;
            float time;

            EnhancedInputBox(Font &font, string lbl, Vector2f position)
            {
                label.setFont(font);
                label.setString(lbl);
                label.setCharacterSize(16);
                label.setFillColor(Color(255, 120, 120));
                label.setStyle(Text::Bold);
                label.setLetterSpacing(1.2f);
                label.setPosition(position.x, position.y - 28);

                box.setSize(Vector2f(460, 50));
                box.setFillColor(Color(20, 8, 12, 250));
                box.setOutlineThickness(3);
                box.setOutlineColor(Color(150, 50, 50));
                box.setPosition(position.x, position.y);

                text.setFont(font);
                text.setCharacterSize(20);
                text.setFillColor(Color(255, 200, 200));
                text.setPosition(position.x + 15, position.y + 12);

                inputString = "";
                isActive = false;
                time = 0.0f;
            }

            void handleEvent(Event &event, RenderWindow &window)
            {
                Vector2f mousePos = window.mapPixelToCoords(Mouse::getPosition(window));

                if (event.type == Event::MouseButtonPressed)
                {
                    if (box.getGlobalBounds().contains(mousePos))
                    {
                        isActive = true;
                        box.setOutlineColor(Color(255, 50, 50));
                        box.setOutlineThickness(4);
                        box.setFillColor(Color(30, 10, 15, 255));
                    }
                    else
                    {
                        isActive = false;
                        box.setOutlineColor(Color(150, 50, 50));
                        box.setOutlineThickness(3);
                        box.setFillColor(Color(20, 8, 12, 250));
                    }
                }

                if (event.type == Event::TextEntered && isActive)
                {
                    if (event.text.unicode == '\b')
                    {
                        if (!inputString.empty())
                        {
                            inputString.pop_back();
                        }
                    }
                    else if (event.text.unicode == 13)
                    {
                    }
                    else if (event.text.unicode < 128 && event.text.unicode != '\r' && event.text.unicode != '\t')
                    {
                        if (inputString.length() < 50)
                        {
                            inputString += static_cast<char>(event.text.unicode);
                        }
                    }
                    text.setString(inputString + (isActive ? "|" : ""));
                }
            }

            void update(float deltaTime)
            {
                time += deltaTime;
                if (isActive)
                {
                    float pulse = sin(time * 6.0f) * 0.4f + 0.6f;
                    int glowIntensity = 200 + (int)(55 * pulse);
                    box.setOutlineColor(Color(255, 40, 40, glowIntensity));
                    box.setOutlineThickness(3 + pulse * 2);
                }
            }

            void draw(RenderWindow &window)
            {
                window.draw(label);
                window.draw(box);

                string displayText = inputString;
                if (isActive && fmod(time, 1.0f) > 0.5f)
                {
                    displayText += "|";
                }
                text.setString(displayText);
                window.draw(text);
            }

            string getText() { return inputString; }
            void setText(string newText)
            {
                inputString = newText;
                text.setString(inputString);
            }
        };

        EnhancedInputBox originInput(font, "Departure Port", Vector2f(730, 350));
        EnhancedInputBox destinationInput(font, "Destination Port", Vector2f(730, 440));
        EnhancedInputBox dateInput(font, "Date (D/M/YYYY or DD/MM/YYYY)", Vector2f(730, 530));

        
        RectangleShape popupSearchButtonGlow(Vector2f(190, 60));
        popupSearchButtonGlow.setFillColor(Color(200, 30, 30, 60));
        popupSearchButtonGlow.setPosition(745, 625);
        
        RectangleShape popupSearchButton(Vector2f(180, 50));
        popupSearchButton.setFillColor(Color(150, 20, 30, 250));
        popupSearchButton.setOutlineThickness(3);
        popupSearchButton.setOutlineColor(Color(255, 60, 60));
        popupSearchButton.setPosition(750, 630);

        Text popupSearchText;
        popupSearchText.setFont(font);
        popupSearchText.setString("FIND ROUTE");
        popupSearchText.setCharacterSize(18);
        popupSearchText.setFillColor(Color(255, 180, 180));
        popupSearchText.setStyle(Text::Bold);
        popupSearchText.setLetterSpacing(1.3f);
        popupSearchText.setPosition(775, 643);

        
        RectangleShape closeButtonGlow(Vector2f(190, 60));
        closeButtonGlow.setFillColor(Color(100, 20, 20, 40));
        closeButtonGlow.setPosition(945, 625);
        
        RectangleShape closeButton(Vector2f(180, 50));
        closeButton.setFillColor(Color(40, 15, 20, 250));
        closeButton.setOutlineThickness(3);
        closeButton.setOutlineColor(Color(120, 40, 40));
        closeButton.setPosition(950, 630);

        Text closeText;
        closeText.setFont(font);
        closeText.setString("CANCEL");
        closeText.setCharacterSize(18);
        closeText.setFillColor(Color(180, 120, 120));
        closeText.setStyle(Text::Bold);
        closeText.setLetterSpacing(1.3f);
        closeText.setPosition(995, 643);

        
        CircleShape searchButtonGlow(30);
        searchButtonGlow.setFillColor(Color::Transparent);
        searchButtonGlow.setPosition(745, 625);

        CircleShape closeButtonGlowCircle(30);
        closeButtonGlowCircle.setFillColor(Color::Transparent);
        closeButtonGlowCircle.setPosition(945, 625);

        int selectedPortIndex = -1;
        RouteNode *hoveredRoute = nullptr;
        bool showRoutes = false;
        float time = 0.0f;
        Vector<float> portPulse;

        
        float mapZoom = 1.0f;
        float targetZoom = 1.0f;
        float mapOffsetX = 0.0f, mapOffsetY = 0.0f;
        float targetOffsetX = 0.0f, targetOffsetY = 0.0f;
        bool isPanning = false;
        Vector2f panStart;
        const float MIN_ZOOM = 0.5f;
        const float MAX_ZOOM = 2.5f;
        const float ZOOM_SPEED = 0.1f;
        const float ZOOM_SMOOTH = 0.12f;

        Vector<int> currentPath;
        Vector<string> waitPorts;
        Vector<string> waitDurations;
        Vector<RouteNode *> pathRoutes;

        bool showSearchResult = false;
        string searchResultText = "";
        Color searchResultColor = Color::White;
        string errorDetails = "";

        Vector<RectangleShape> waterRipples;
        for (int i = 0; i < 20; i++)
        {
            RectangleShape ripple(Vector2f(rand() % 200 + 50, rand() % 200 + 50));
            ripple.setFillColor(Color(30, 60, 120, 30));
            ripple.setPosition(rand() % 1920, rand() % 1080);
            waterRipples.push_back(ripple);
        }

        for (int i = 0; i < graph.ports.getSize(); i++)
        {
            portPulse.push_back(0.0f);
        }

        auto addRouteAnimation = [&](Vector<int> path, Vector<string> waitPorts, Vector<string> waitDurations, Color color, string type)
        {
            RouteAnimation newAnim;
            newAnim.path = path;
            newAnim.waitPorts = waitPorts;
            newAnim.waitDurations = waitDurations;
            newAnim.waitTimes = Vector<int>();
            newAnim.routeColor = color;
            newAnim.routeType = type;
            newAnim.currentSegment = 0;
            newAnim.segmentProgress = 0.0f;
            newAnim.isPaused = false;
            newAnim.pauseTimer = 0.0f;
            newAnim.isComplete = false;

            newAnim.pathRoutes.clear();
            newAnim.totalCost = 0;
            newAnim.totalTime = 0;

            if (path.getSize() > 1)
            {
                for (int i = 0; i < path.getSize() - 1; i++)
                {
                    string fromPort = portLocations[path[i]].name;
                    string toPort = portLocations[path[i + 1]].name;

                    int fromIndex = graph.findPortIndex(fromPort);
                    RouteNode *route = graph.ports[fromIndex].routeHead;

                    while (route != nullptr)
                    {
                        if (route->destinationPort == toPort)
                        {
                            newAnim.pathRoutes.push_back(route);
                            newAnim.totalCost += route->cost;

                            int travelTime = graph.convertToMinutes(route->arrivalTime) -
                                             graph.convertToMinutes(route->departureTime);
                            if (travelTime < 0)
                                travelTime += 24 * 60;
                            newAnim.totalTime += travelTime;
                            break;
                        }
                        route = route->next;
                    }
                }
            }

            activeAnimations.push_back(newAnim);
        };

        
        auto displayFoundPath = [&]() {
            if (foundPath.getSize() == 0)
                return;

            activeAnimations.clear();

            RouteAnimation newAnim;
            newAnim.path = foundPath;
            newAnim.waitPorts = foundPathWaitPorts;
            newAnim.waitDurations = foundPathWaitDurations;
            newAnim.waitTimes = foundPathWaitTimes;
            newAnim.routeColor = foundPathColor;
            newAnim.routeType = foundPathLabel;
            newAnim.currentSegment = 0;
            newAnim.segmentProgress = 0.0f;
            newAnim.isPaused = false;
            newAnim.pauseTimer = 0.0f;
            newAnim.isComplete = false;
            newAnim.pathRoutes = foundPathRoutes;
            newAnim.totalCost = foundPathCost;
            newAnim.totalTime = foundPathTime;

            activeAnimations.push_back(newAnim);
        };

        while (window.isOpen())
        {
            Event event;
            Vector2f mousePos = window.mapPixelToCoords(Mouse::getPosition(window));
            float deltaTime = 0.016f;
            time += deltaTime;
            
            
            hoveredPortIdx = -1;
            View currentView = window.getView();
            Vector2f mapMousePos = window.mapPixelToCoords(Mouse::getPosition(window), currentView);
            for (int i = 0; i < numLocations; i++) {
                float px = portLocations[i].x;
                float py = portLocations[i].y;
                float dist = sqrt(pow(mapMousePos.x - px, 2) + pow(mapMousePos.y - py, 2));
                if (dist < 15) {
                    hoveredPortIdx = i;
                    break;
                }
            }

            while (window.pollEvent(event))
            {
                if (event.type == Event::Closed)
                    window.close();
                
                
                if (event.type == Event::MouseWheelScrolled && showDockingMode && showPortSelection)
                {
                    portScrollOffset -= event.mouseWheelScroll.delta * 40.0f;
                    if (portScrollOffset < 0) portScrollOffset = 0;
                    float maxScroll = (numLocations / 6 + 1) * 105.0f - 700.0f;
                    if (portScrollOffset > maxScroll) portScrollOffset = maxScroll;
                }
                
                
                if (event.type == Event::MouseWheelScrolled && !showDockingMode && !showInputWindow && !showShortestPathMode)
                {
                    float zoomDelta = event.mouseWheelScroll.delta * ZOOM_SPEED;
                    targetZoom += zoomDelta;
                    if (targetZoom < MIN_ZOOM) targetZoom = MIN_ZOOM;
                    if (targetZoom > MAX_ZOOM) targetZoom = MAX_ZOOM;
                }
                
                
                if (event.type == Event::MouseButtonPressed && event.mouseButton.button == Mouse::Middle)
                {
                    if (!showDockingMode && !showInputWindow && !showShortestPathMode) {
                        isPanning = true;
                        panStart = mousePos;
                    }
                }
                if (event.type == Event::MouseButtonReleased && event.mouseButton.button == Mouse::Middle)
                {
                    isPanning = false;
                }
                
                
                if (event.type == Event::TextEntered && showShortestPathMode && showShortestPathInput)
                {
                    string* activeStr = nullptr;
                    if (spOriginActive) activeStr = &spOrigin;
                    else if (spDestActive) activeStr = &spDestination;
                    else if (spDateActive) activeStr = &spDate;
                    
                    if (activeStr) {
                        if (event.text.unicode == '\b') {
                            if (!activeStr->empty()) activeStr->pop_back();
                        }
                        else if (event.text.unicode < 128 && event.text.unicode != '\r' && event.text.unicode != '\t') {
                            if (activeStr->length() < 30) {
                                *activeStr += static_cast<char>(event.text.unicode);
                            }
                        }
                    }
                }

                if (event.type == Event::MouseButtonPressed && event.mouseButton.button == Mouse::Button::Left)
                {
                    Vector2i clickPos = Mouse::getPosition(window);
                    
                    View mapView = window.getView();
                    Vector2f mapMousePos = window.mapPixelToCoords(clickPos, mapView);

                    if (searchButton.getGlobalBounds().contains(mousePos) && !showInputWindow && !showDockingMode)
                    {
                        clickSound.play();
                        transitionSound.play();
                        showInputWindow = true;
                        originInput.setText("");
                        destinationInput.setText("");
                        dateInput.setText("");
                        showSearchResult = false;
                        activeAnimations.clear();
                        originInput.isActive = true;
                        destinationInput.isActive = false;
                        dateInput.isActive = false;
                        errorDetails = "";
                    }
                    
                    
                    if (dockingButton.getGlobalBounds().contains(mousePos) && !showInputWindow && !showDockingMode && !showShortestPathMode)
                    {
                        clickSound.play();
                        transitionSound.play();
                        showDockingMode = true;
                        showPortSelection = true;
                        selectedDockingPort = -1;
                        showSearchResult = false;
                        activeAnimations.clear();
                        arrivingShips.clear();
                        departingShips.clear();
                        portScrollOffset = 0.0f;
                    }

                    
                    if (shortestPathButton.getGlobalBounds().contains(mousePos) && !showInputWindow && !showDockingMode && !showShortestPathMode)
                    {
                        clickSound.play();
                        transitionSound.play();
                        showShortestPathMode = true;
                        showAlgorithmSelection = true;
                        selectedAlgorithm = 0;
                        showShortestPathInput = false;
                        showSearchResult = false;
                        activeAnimations.clear();
                        spOrigin = "";
                        spDestination = "";
                        spDate = "";
                        spPathFound = false;
                        spResultText = "";
                    }

                    
                    if (shipPrefsButton.getGlobalBounds().contains(mousePos) && !showInputWindow && !showDockingMode && !showShortestPathMode)
                    {
                        clickSound.play();
                        openShipPreferencesPopup(font);
                    }

                    if (showInputWindow)
                    {
                        originInput.handleEvent(event, window);
                        destinationInput.handleEvent(event, window);
                        dateInput.handleEvent(event, window);

                        if (popupSearchButton.getGlobalBounds().contains(mousePos))
                        {
                            clickSound.play();
                            string origin = originInput.getText();
                            string destination = destinationInput.getText();
                            string date = dateInput.getText();

                            activeAnimations.clear();
                            waitPorts.clear();
                            waitDurations.clear();
                            pathRoutes.clear();
                            errorDetails = "";
                            
                            
                            foundPath.clear();
                            foundPathRoutes.clear();
                            foundPathWaitPorts.clear();
                            foundPathWaitDurations.clear();
                            foundPathWaitTimes.clear();
                            foundPathCost = 0;
                            foundPathTime = 0;

                            if (origin.empty() || destination.empty() || date.empty())
                            {
                                searchResultText = "Input Error!";
                                searchResultColor = Color::Red;
                                errorDetails = "Please fill all fields (Departure Port, Destination Port, and Date).";
                                showSearchResult = true;
                            }
                            else
                            {
                                
                                string normalizedDate = graph.normalizeDate(date);
                                
                                if (normalizedDate.length() != 10 || normalizedDate[2] != '/' || normalizedDate[5] != '/')
                                {
                                    searchResultText = "Invalid Date Format!";
                                    searchResultColor = Color::Red;
                                    errorDetails = "Please use DD/MM/YYYY format (e.g., 15/12/2024)";
                                    showSearchResult = true;
                                }
                                else
                                {
                                    
                                    bool dateValid = true;
                                    try
                                    {
                                        int day = stoi(normalizedDate.substr(0, 2));
                                        int month = stoi(normalizedDate.substr(3, 2));
                                        int year = stoi(normalizedDate.substr(6, 4));

                                        if (day < 1 || day > 31 || month < 1 || month > 12 || year < 2024)
                                        {
                                            searchResultText = "Invalid Date Values!";
                                            searchResultColor = Color::Red;
                                            errorDetails = "Day must be 1-31, month 1-12, year >= 2024";
                                            showSearchResult = true;
                                            dateValid = false;
                                        }
                                    }
                                    catch (...)
                                    {
                                        searchResultText = "Invalid Date!";
                                        searchResultColor = Color::Red;
                                        errorDetails = "Date must contain valid numbers";
                                        showSearchResult = true;
                                        dateValid = false;
                                    }
                                    
                                    if (dateValid)
                                    {
                                
                                Vector<Maps::ValidPath> allValidPaths;
                                bool pathsFound = graph.findAllValidPaths(origin, destination, normalizedDate, allValidPaths, 5);
                                
                                userSearchedDate = normalizedDate;
                                
                                
                                Color pathColors[] = {
                                    Color(255, 100, 100),   
                                    Color(100, 255, 150),   
                                    Color(255, 200, 50),    
                                    Color(100, 200, 255),   
                                    Color(255, 150, 200),   
                                };

                                if (pathsFound && allValidPaths.getSize() > 0)
                                {
                                    activeAnimations.clear();
                                    
                                    
                                    for (int p = 0; p < allValidPaths.getSize(); p++)
                                    {
                                        Maps::ValidPath& vp = allValidPaths[p];
                                        
                                        RouteAnimation newAnim;
                                        newAnim.path = vp.path;
                                        newAnim.waitPorts = vp.waitPorts;
                                        newAnim.waitDurations = vp.waitDurations;
                                        newAnim.pathRoutes = vp.routes;
                                        newAnim.routeColor = pathColors[p % 5];
                                        newAnim.routeType = vp.label + " ($" + to_string(vp.totalCost) + ")";
                                        newAnim.currentSegment = 0;
                                        newAnim.segmentProgress = 0.0f;
                                        newAnim.isPaused = false;
                                        newAnim.pauseTimer = 0.0f;
                                        newAnim.isComplete = false;
                                        newAnim.totalCost = vp.totalCost;
                                        newAnim.totalTime = vp.totalTime;
                                        
                                        
                                        newAnim.segmentProgress = -0.2f * p;
                                        
                                        activeAnimations.push_back(newAnim);
                                    }
                                    
                                    
                                    foundPath = allValidPaths[0].path;
                                    foundPathRoutes = allValidPaths[0].routes;
                                    foundPathWaitPorts = allValidPaths[0].waitPorts;
                                    foundPathWaitDurations = allValidPaths[0].waitDurations;
                                    foundPathCost = allValidPaths[0].totalCost;
                                    foundPathTime = allValidPaths[0].totalTime;
                                    foundPathColor = pathColors[0];
                                    foundPathLabel = allValidPaths[0].label;
                                    
                                    searchResultText = to_string(allValidPaths.getSize()) + " Route(s) Found!";
                                    searchResultColor = Color::Green;
                                    errorDetails = "Showing all valid paths from " + origin + " to " + destination;
                                    showSearchResult = true;
                                    showRoutes = true;
                                }
                                else
                                {
                                    searchResultText = "No routes found!";
                                    searchResultColor = Color::Red;
                                    errorDetails = "No routes available for the given ports and date.";
                                }

                                showSearchResult = true;
                                showRoutes = true;
                                    }
                                }
                            }
                        }
                        if (closeButton.getGlobalBounds().contains(mousePos))
                        {
                            clickSound.play();
                            showInputWindow = false;
                            showSearchResult = false;
                            errorDetails = "";
                        }
                    }

                    if (!showInputWindow)
                    {
                        for (int i = 0; i < numLocations; i++)
                        {
                            float px = portLocations[i].x;
                            float py = portLocations[i].y;
                            float dist = sqrt(pow(mapMousePos.x - px, 2) + pow(mapMousePos.y - py, 2));
                            if (dist < 12)
                            {
                                selectedPortIndex = i;
                                portPulse[i] = 1.0f;
                                showSearchResult = false;
                                errorDetails = "";

                                
                                activeAnimations.clear();
                                showRoutes = true;

                                
                                string clickedPortName = portLocations[i].name;
                                int portIndex = graph.findPortIndex(clickedPortName);

                                if (portIndex != -1)
                                {
                                    
                                    RouteNode *route = graph.ports[portIndex].routeHead;
                                    int routeCount = 0;

                                    
                                    Color routeColors[] = {
                                        Color(0, 255, 150),   
                                        Color(255, 200, 0),   
                                        Color(255, 100, 150), 
                                        Color(100, 200, 255), 
                                        Color(255, 150, 50),  
                                        Color(200, 100, 255), 
                                        Color(100, 255, 255), 
                                        Color(255, 255, 100), 
                                        Color(150, 255, 150), 
                                        Color(255, 180, 180)  
                                    };

                                    while (route != nullptr && routeCount < 10)
                                    {
                                        
                                        int destLocationIdx = -1;
                                        for (int j = 0; j < numLocations; j++)
                                        {
                                            if (portLocations[j].name == route->destinationPort)
                                            {
                                                destLocationIdx = j;
                                                break;
                                            }
                                        }

                                        if (destLocationIdx != -1)
                                        {
                                            
                                            Vector<int> routePath;
                                            routePath.push_back(i);               
                                            routePath.push_back(destLocationIdx); 

                                            
                                            RouteAnimation newAnim;
                                            newAnim.path = routePath;
                                            newAnim.waitPorts = Vector<string>();
                                            newAnim.waitDurations = Vector<string>();
                                            newAnim.waitTimes = Vector<int>();
                                            newAnim.routeColor = routeColors[routeCount % 10];
                                            newAnim.routeType = route->destinationPort + " ($" + to_string(route->cost) + ")";
                                            newAnim.currentSegment = 0;
                                            newAnim.segmentProgress = 0.0f;
                                            newAnim.isPaused = false;
                                            newAnim.pauseTimer = 0.0f;
                                            newAnim.isComplete = false;
                                            newAnim.totalCost = route->cost;

                                            
                                            int depMin = graph.convertToMinutes(route->departureTime);
                                            int arrMin = graph.convertToMinutes(route->arrivalTime);
                                            int travelTime = arrMin - depMin;
                                            if (travelTime < 0)
                                                travelTime += 24 * 60;
                                            newAnim.totalTime = travelTime;

                                            
                                            newAnim.pathRoutes.push_back(route);

                                            activeAnimations.push_back(newAnim);
                                            routeCount++;
                                        }

                                        route = route->next;
                                    }

                                    if (routeCount > 0)
                                    {
                                        searchResultText = to_string(routeCount) + " routes from " + clickedPortName;
                                        searchResultColor = Color::Green;
                                        errorDetails = "Click on another port or use Search Route for specific paths";
                                        showSearchResult = true;
                                    }
                                    else
                                    {
                                        searchResultText = "No outgoing routes from " + clickedPortName;
                                        searchResultColor = Color::Yellow;
                                        errorDetails = "This port has no direct outbound connections";
                                        showSearchResult = true;
                                    }
                                }
                            }
                        }
                    }
                }

                if (showInputWindow)
                {
                    originInput.handleEvent(event, window);
                    destinationInput.handleEvent(event, window);
                    dateInput.handleEvent(event, window);
                }

                if (event.type == Event::KeyPressed && event.key.code == Keyboard::Escape)
                {
                    if (showAddShipDialog)
                    {
                        showAddShipDialog = false;
                        dockNameActive = false;
                        dockCompanyActive = false;
                        dockTimeActive = false;
                    }
                    else if (showDockingMode)
                    {
                        showDockingMode = false;
                    }
                    else if (showInputWindow)
                    {
                        showInputWindow = false;
                        showSearchResult = false;
                        errorDetails = "";
                    }
                    else
                    {
                        selectedPortIndex = -1;
                        showRoutes = false;
                        hoveredRoute = nullptr;
                        showSearchResult = false;
                        errorDetails = "";
                    }
                }
                
                
                if (showAddShipDialog && event.type == Event::TextEntered)
                {
                    char c = static_cast<char>(event.text.unicode);
                    if (dockNameActive) {
                        if (event.text.unicode == '\b') {
                            if (!dockInputShipName.empty()) dockInputShipName.pop_back();
                        } else if (event.text.unicode < 128 && event.text.unicode != '\r' && dockInputShipName.length() < 20) {
                            dockInputShipName += c;
                        }
                    } else if (dockCompanyActive) {
                        if (event.text.unicode == '\b') {
                            if (!dockInputCompany.empty()) dockInputCompany.pop_back();
                        } else if (event.text.unicode < 128 && event.text.unicode != '\r' && dockInputCompany.length() < 15) {
                            dockInputCompany += c;
                        }
                    } else if (dockTimeActive) {
                        if (event.text.unicode == '\b') {
                            if (!dockInputTime.empty()) dockInputTime.pop_back();
                        } else if (c >= '0' && c <= '9' && dockInputTime.length() < 3) {
                            dockInputTime += c;
                        }
                    }
                }

                if (event.type == Event::KeyPressed && event.key.code == Keyboard::Tab && showInputWindow)
                {
                    if (originInput.isActive)
                    {
                        originInput.isActive = false;
                        destinationInput.isActive = true;
                    }
                    else if (destinationInput.isActive)
                    {
                        destinationInput.isActive = false;
                        dateInput.isActive = true;
                    }
                    else
                    {
                        dateInput.isActive = false;
                        originInput.isActive = true;
                    }
                }
            }

            for (int i = 0; i < portPulse.getSize(); i++)
            {
                if (portPulse[i] > 0)
                {
                    portPulse[i] -= 0.05f;
                    if (portPulse[i] < 0)
                        portPulse[i] = 0;
                }
            }

            if (showInputWindow)
            {
                originInput.update(deltaTime);
                destinationInput.update(deltaTime);
                dateInput.update(deltaTime);
            }

            for (int i = 0; i < waterRipples.getSize(); i++)
            {
                float scale = 1.0f + 0.1f * sin(time * 0.5f + waterRipples[i].getPosition().x * 0.01f);
                waterRipples[i].setScale(scale, scale);
                Color c = waterRipples[i].getFillColor();
                c.a = 20 + 10 * sin(time * 0.3f + waterRipples[i].getPosition().y * 0.01f);
                waterRipples[i].setFillColor(c);
            }

            for (int animIndex = 0; animIndex < activeAnimations.getSize(); animIndex++)
            {
                RouteAnimation &anim = activeAnimations[animIndex];

                if (!anim.isComplete)
                {
                    if (!anim.isPaused)
                    {
                        anim.segmentProgress += deltaTime * animationSpeed;

                        if (anim.segmentProgress >= 1.0f)
                        {
                            anim.segmentProgress = 1.0f;
                            anim.isPaused = true;
                            anim.pauseTimer = 0.0f;
                        }
                    }
                    else
                    {
                        anim.pauseTimer += deltaTime;
                        if (anim.pauseTimer >= SEGMENT_PAUSE)
                        {
                            anim.currentSegment++;
                            anim.segmentProgress = 0.0f;
                            anim.isPaused = false;

                            if (anim.currentSegment >= anim.path.getSize() - 1)
                            {
                                anim.isComplete = true;
                            }
                        }
                    }
                }
            }

            window.clear(Color(10, 20, 40));
            
            
            mapZoom += (targetZoom - mapZoom) * ZOOM_SMOOTH;
            mapOffsetX += (targetOffsetX - mapOffsetX) * ZOOM_SMOOTH;
            mapOffsetY += (targetOffsetY - mapOffsetY) * ZOOM_SMOOTH;
            
            
            if (isPanning) {
                Vector2f currentMouse = window.mapPixelToCoords(Mouse::getPosition(window));
                targetOffsetX += (currentMouse.x - panStart.x) * 0.5f;
                targetOffsetY += (currentMouse.y - panStart.y) * 0.5f;
                panStart = currentMouse;
            }
            
            
            float maxOffset = 400.0f * (mapZoom - 1.0f);
            if (targetOffsetX > maxOffset) targetOffsetX = maxOffset;
            if (targetOffsetX < -maxOffset) targetOffsetX = -maxOffset;
            if (targetOffsetY > maxOffset) targetOffsetY = maxOffset;
            if (targetOffsetY < -maxOffset) targetOffsetY = -maxOffset;
            
            
            View mapView(FloatRect(0, 0, 1920, 1080));
            mapView.setCenter(960 - mapOffsetX, 540 - mapOffsetY);
            mapView.zoom(1.0f / mapZoom);
            window.setView(mapView);

            for (int i = 0; i < waterRipples.getSize(); i++)
            {
                window.draw(waterRipples[i]);
            }
            window.draw(mapSprite);

            for (int animIndex = 0; animIndex < activeAnimations.getSize(); animIndex++)
            {
                RouteAnimation &anim = activeAnimations[animIndex];

                if (anim.path.getSize() > 1)
                {
                    for (int i = 0; i < anim.path.getSize() - 1; i++)
                    {
                        int startIdx = anim.path[i];
                        int endIdx = anim.path[i + 1];
                        Vector2f start(portLocations[startIdx].x, portLocations[startIdx].y);
                        Vector2f end(portLocations[endIdx].x, portLocations[endIdx].y);

                        float length = sqrt(pow(end.x - start.x, 2) + pow(end.y - start.y, 2));
                        float angle = atan2(end.y - start.y, end.x - start.x) * 180 / 3.14159f;

                        if (i == anim.currentSegment && !anim.isComplete)
                        {
                            float currentLength = length * anim.segmentProgress;

                            RectangleShape currentLine(Vector2f(currentLength, 6.0f));
                            currentLine.setPosition(start);
                            currentLine.setRotation(angle);
                            currentLine.setFillColor(anim.routeColor);
                            window.draw(currentLine);

                            RectangleShape currentGlow(Vector2f(currentLength, 12.0f));
                            currentGlow.setPosition(start);
                            currentGlow.setRotation(angle);
                            currentGlow.setFillColor(Color(anim.routeColor.r, anim.routeColor.g, anim.routeColor.b, 100));
                            window.draw(currentGlow);

                            if (!anim.isComplete)
                            {
                                Vector2f currentPos = start + (end - start) * anim.segmentProgress;
                                float pulse = 0.7f + 0.3f * sin(time * 8.0f);

                                
                                CircleShape arrowGlow(18.f * pulse);
                                arrowGlow.setFillColor(Color(anim.routeColor.r, anim.routeColor.g, anim.routeColor.b, 100));
                                arrowGlow.setOrigin(18.f * pulse, 18.f * pulse);
                                arrowGlow.setPosition(currentPos);
                                window.draw(arrowGlow);

                                
                                ConvexShape arrow;
                                arrow.setPointCount(3);
                                float arrowSize = 14.f;
                                arrow.setPoint(0, Vector2f(arrowSize, 0));
                                arrow.setPoint(1, Vector2f(-arrowSize * 0.6f, -arrowSize * 0.7f));
                                arrow.setPoint(2, Vector2f(-arrowSize * 0.6f, arrowSize * 0.7f));
                                arrow.setOrigin(0, 0);
                                arrow.setPosition(currentPos);
                                arrow.setRotation(angle);
                                arrow.setFillColor(Color(255, 220, 100));
                                arrow.setOutlineColor(Color::White);
                                arrow.setOutlineThickness(2);
                                window.draw(arrow);
                            }
                        }
                        else if (i < anim.currentSegment || anim.isComplete)
                        {
                            RectangleShape completedLine(Vector2f(length, 5.0f));
                            completedLine.setPosition(start);
                            completedLine.setRotation(angle);
                            completedLine.setFillColor(anim.routeColor);
                            window.draw(completedLine);
                            
                            
                            Vector2f midPoint = (start + end) * 0.5f;
                            ConvexShape segArrow;
                            segArrow.setPointCount(3);
                            float segArrowSize = 10.f;
                            segArrow.setPoint(0, Vector2f(segArrowSize, 0));
                            segArrow.setPoint(1, Vector2f(-segArrowSize * 0.5f, -segArrowSize * 0.6f));
                            segArrow.setPoint(2, Vector2f(-segArrowSize * 0.5f, segArrowSize * 0.6f));
                            segArrow.setPosition(midPoint);
                            segArrow.setRotation(angle);
                            segArrow.setFillColor(anim.routeColor);
                            segArrow.setOutlineColor(Color::White);
                            segArrow.setOutlineThickness(1);
                            window.draw(segArrow);
                        }
                        else
                        {
                            RectangleShape upcomingLine(Vector2f(length, 3.0f));
                            upcomingLine.setPosition(start);
                            upcomingLine.setRotation(angle);
                            upcomingLine.setFillColor(Color(anim.routeColor.r, anim.routeColor.g, anim.routeColor.b, 100));
                            window.draw(upcomingLine);
                        }
                    }
                }
            }

            hoveredRoute = nullptr;
            int hoveredAnimIndex = -1;
            if (activeAnimations.getSize() > 0)
            {
                for (int animIndex = 0; animIndex < activeAnimations.getSize(); animIndex++)
                {
                    RouteAnimation &anim = activeAnimations[animIndex];
                    for (int i = 0; i < anim.path.getSize() - 1; i++)
                    {
                        Vector2f start(portLocations[anim.path[i]].x, portLocations[anim.path[i]].y);
                        Vector2f end(portLocations[anim.path[i + 1]].x, portLocations[anim.path[i + 1]].y);

                        float length = sqrt(pow(end.x - start.x, 2) + pow(end.y - start.y, 2));

                        float lineLength = length;
                        Vector2f lineDir = (end - start) / lineLength;
                        Vector2f toMouse = mousePos - start;
                        float projection = toMouse.x * lineDir.x + toMouse.y * lineDir.y;
                        projection = max(0.0f, min(lineLength, projection));
                        Vector2f closestPoint = start + lineDir * projection;

                        float distance = sqrt(pow(mousePos.x - closestPoint.x, 2) + pow(mousePos.y - closestPoint.y, 2));

                        if (distance < 15.0f && i < anim.pathRoutes.getSize())
                        {
                            hoveredRoute = anim.pathRoutes[i];
                            hoveredAnimIndex = animIndex;
                            break;
                        }
                    }
                    if (hoveredRoute != nullptr)
                        break;
                }
            }

            for (int i = 0; i < numLocations; i++)
            {
                
                string currentPortName = portLocations[i].name;
                bool isAvoided = shipPrefs.isPortAvoided(currentPortName);
                
                
                float fadeFactor = 1.0f;
                if (shipPrefs.filterActive && isAvoided) {
                    fadeFactor = 0.1f; 
                }

                
                if (shipPrefs.filterActive && isAvoided) {
                    CircleShape avoidCircle(25.f);
                    avoidCircle.setOrigin(25, 25);
                    avoidCircle.setFillColor(Color(255, 0, 0, 60)); 
                    avoidCircle.setOutlineThickness(4);
                    avoidCircle.setOutlineColor(Color(255, 0, 0, 220)); 
                    avoidCircle.setPosition(portLocations[i].x, portLocations[i].y);
                    window.draw(avoidCircle);
                }

                CircleShape glow(16.f);
                glow.setOrigin(16, 16);
                glow.setFillColor(Color(200, 80, 80, static_cast<Uint8>(100 * fadeFactor)));  
                glow.setPosition(portLocations[i].x, portLocations[i].y);
                window.draw(glow);

                float pulseScale = 1.0f + 0.2f * portPulse[i];
                CircleShape dot(8.f * pulseScale);
                dot.setOrigin(8.f * pulseScale, 8.f * pulseScale);

                bool inPath = false;
                for (int animIndex = 0; animIndex < activeAnimations.getSize(); animIndex++)
                {
                    RouteAnimation &anim = activeAnimations[animIndex];
                    for (int j = 0; j < anim.path.getSize(); j++)
                    {
                        if (anim.path[j] == i)
                        {
                            inPath = true;
                            break;
                        }
                    }
                    if (inPath)
                        break;
                }

                if (inPath)
                {
                    
                    Color portColor = Color(255, 100, 100);  
                    for (int animIndex = 0; animIndex < activeAnimations.getSize(); animIndex++)
                    {
                        RouteAnimation &anim = activeAnimations[animIndex];
                        for (int j = 0; j < anim.path.getSize(); j++)
                        {
                            if (anim.path[j] == i)
                            {
                                portColor = anim.routeColor;
                                break;
                            }
                        }
                    }
                    dot.setFillColor(portColor);
                }
                else if (i == selectedPortIndex)
                {
                    dot.setFillColor(Color(255, 80, 80));  
                    CircleShape selectionGlow(20.f);
                    selectionGlow.setOrigin(20, 20);
                    selectionGlow.setFillColor(Color(255, 50, 50, 120));
                    selectionGlow.setPosition(portLocations[i].x, portLocations[i].y);
                    window.draw(selectionGlow);
                }
                else if (isAvoided && shipPrefs.filterActive)
                {
                    
                    dot.setFillColor(Color(80, 30, 30, 50));
                }
                else
                {
                    float gentlePulse = 0.5f + 0.5f * sin(time * 2.0f + i);
                    dot.setFillColor(Color(180 + 75 * gentlePulse, 60, 60));  
                }

                dot.setOutlineColor(Color(255, 200, 200, static_cast<Uint8>(255 * fadeFactor)));
                dot.setOutlineThickness(1.5f);
                dot.setPosition(portLocations[i].x, portLocations[i].y);
                window.draw(dot);

                
                if (isAvoided && shipPrefs.filterActive) {
                    
                    Text bigX("X", font, 30);
                    bigX.setFillColor(Color(255, 0, 0, 255)); 
                    bigX.setStyle(Text::Bold);
                    bigX.setPosition(portLocations[i].x - 12, portLocations[i].y - 20);
                    window.draw(bigX);
                    
                    
                    Text avoidedLabel("AVOIDED", font, 11);
                    avoidedLabel.setFillColor(Color(255, 50, 50, 255));
                    avoidedLabel.setStyle(Text::Bold);
                    avoidedLabel.setPosition(portLocations[i].x - 25, portLocations[i].y + 15);
                    window.draw(avoidedLabel);
                }
                
                
                
                int dockedCount = globalPortDocking[i].getOccupiedDocks();
                int totalDocks = globalPortDocking[i].maxDocks;
                int queueLen = globalPortDocking[i].getQueueLength();
                
                
                if (dockedCount > 0 || queueLen > 0) {
                    float portX = portLocations[i].x;
                    float portY = portLocations[i].y;
                    
                    
                    for (int d = 0; d < totalDocks; d++) {
                        float angle = (d * 360.0f / totalDocks) * 3.14159f / 180.0f;
                        float dockX = portX + cos(angle) * 25;
                        float dockY = portY + sin(angle) * 25;
                        
                        
                        RectangleShape dockSlot(Vector2f(8, 12));
                        dockSlot.setOrigin(4, 6);
                        dockSlot.setPosition(dockX, dockY);
                        dockSlot.setRotation(angle * 180.0f / 3.14159f + 90);
                        
                        if (d < dockedCount) {
                            
                            dockSlot.setFillColor(Color(100, 200, 255, 200)); 
                            dockSlot.setOutlineThickness(1);
                            dockSlot.setOutlineColor(Color(150, 230, 255));
                        } else {
                            
                            dockSlot.setFillColor(Color(60, 60, 60, 150));
                            dockSlot.setOutlineThickness(1);
                            dockSlot.setOutlineColor(Color(100, 100, 100, 150));
                        }
                        window.draw(dockSlot);
                    }
                    
                    
                    if (queueLen > 0) {
                        for (int qPos = 0; qPos < queueLen && qPos < globalPortDocking[i].queuedShips.getSize(); qPos++) {
                            
                            float qAngle = (time * 0.3f + qPos * 1.2f); 
                            float qRadius = 45 + qPos * 12; 
                            float qX = portX + cos(qAngle) * qRadius;
                            float qY = portY + sin(qAngle) * qRadius;
                            
                            
                            float dashLen = 5;
                            float gapLen = 4;
                            float totalDist = sqrt(pow(portX - qX, 2) + pow(portY - qY, 2));
                            float dirX = (portX - qX) / totalDist;
                            float dirY = (portY - qY) / totalDist;
                            
                            for (float dd = 0; dd < totalDist - 20; dd += dashLen + gapLen) {
                                RectangleShape dash(Vector2f(dashLen, 1.5f));
                                dash.setFillColor(Color(255, 200, 100, 150)); 
                                dash.setPosition(qX + dirX * dd, qY + dirY * dd);
                                dash.setRotation(atan2(dirY, dirX) * 180.0f / 3.14159f);
                                window.draw(dash);
                            }
                            
                            
                            ConvexShape waitingShip;
                            waitingShip.setPointCount(3);
                            waitingShip.setPoint(0, Vector2f(0, -6));
                            waitingShip.setPoint(1, Vector2f(-4, 6));
                            waitingShip.setPoint(2, Vector2f(4, 6));
                            waitingShip.setFillColor(Color(255, 180, 80, 220)); 
                            waitingShip.setOutlineThickness(1);
                            waitingShip.setOutlineColor(Color(255, 220, 150));
                            waitingShip.setPosition(qX, qY);
                            waitingShip.setRotation(qAngle * 180.0f / 3.14159f + 90);
                            window.draw(waitingShip);
                            
                            
                            Text qNum(to_string(qPos + 1), font, 9);
                            qNum.setFillColor(Color::White);
                            qNum.setStyle(Text::Bold);
                            qNum.setPosition(qX - 3, qY - 4);
                            window.draw(qNum);
                        }
                    }
                    
                    
                    string dockStatusStr = to_string(dockedCount) + "/" + to_string(totalDocks);
                    if (queueLen > 0) dockStatusStr += " +" + to_string(queueLen);
                    Text dockText(dockStatusStr, font, 10);
                    dockText.setFillColor(dockedCount >= totalDocks ? Color(255, 150, 100) : Color(150, 255, 150));
                    dockText.setStyle(Text::Bold);
                    FloatRect dtBounds = dockText.getLocalBounds();
                    dockText.setPosition(portX - dtBounds.width / 2, portY + 30);
                    window.draw(dockText);
                }
            }
            
            
            if (hoveredPortIdx >= 0 && hoveredPortIdx < numLocations) {
                float hx = portLocations[hoveredPortIdx].x;
                float hy = portLocations[hoveredPortIdx].y;
                
                
                CircleShape hoverGlow(22);
                hoverGlow.setOrigin(22, 22);
                hoverGlow.setPosition(hx, hy);
                hoverGlow.setFillColor(Color(255, 200, 150, 100));
                hoverGlow.setOutlineThickness(3);
                hoverGlow.setOutlineColor(Color(255, 255, 200, 200));
                window.draw(hoverGlow);
                
                
                Text hoverLabel(portLocations[hoveredPortIdx].name, font, 16);
                hoverLabel.setStyle(Text::Bold);
                hoverLabel.setFillColor(Color(255, 255, 220));
                hoverLabel.setOutlineThickness(2);
                hoverLabel.setOutlineColor(Color(40, 20, 20));
                FloatRect hlBounds = hoverLabel.getLocalBounds();
                
                
                float labelX = hx - hlBounds.width / 2;
                float labelY = hy - 35;
                if (hy < 100) labelY = hy + 20; 
                
                hoverLabel.setPosition(labelX, labelY);
                window.draw(hoverLabel);
            }

            
            

            
            View uiView(FloatRect(0, 0, 1920, 1080));
            window.setView(uiView);
            
            bool searchHover = searchButton.getGlobalBounds().contains(mousePos) && !showInputWindow;

            float buttonPulse = sin(time * 3.0f) * 0.2f + 0.8f;
            buttonGlow.setRadius(40 * buttonPulse);
            buttonGlow.setOrigin(40 * buttonPulse, 40 * buttonPulse);
            buttonGlow.setPosition(30, 1080);

            if (searchHover)
            {
                searchButton.setFillColor(Color(140, 40, 50, 240));
                searchButton.setOutlineColor(Color(255, 100, 100));
                window.draw(buttonGlow);
            }
            else
            {
                searchButton.setFillColor(Color(80, 20, 30, 230));
                searchButton.setOutlineColor(Color(200, 50, 50));
            }

            window.draw(searchButton);
            window.draw(searchButtonText);
            
            
            bool dockHover = dockingButton.getGlobalBounds().contains(mousePos) && !showInputWindow && !showDockingMode;
            if (dockHover)
            {
                dockingButton.setFillColor(Color(120, 35, 45, 240));
                dockingButton.setOutlineColor(Color(255, 120, 120));
            }
            else
            {
                dockingButton.setFillColor(Color(60, 15, 25, 230));
                dockingButton.setOutlineColor(Color(180, 60, 60));
            }
            window.draw(dockingButton);
            window.draw(dockingButtonText);

            
            bool spHover = shortestPathButton.getGlobalBounds().contains(mousePos) && !showInputWindow && !showDockingMode && !showShortestPathMode;
            if (spHover)
            {
                shortestPathButton.setFillColor(Color(160, 50, 60, 240));
                shortestPathButton.setOutlineColor(Color(255, 140, 140));
            }
            else
            {
                shortestPathButton.setFillColor(Color(100, 25, 35, 230));
                shortestPathButton.setOutlineColor(Color(220, 80, 80));
            }
            window.draw(shortestPathButton);
            window.draw(shortestPathButtonText);
            
            
            if (!showInputWindow && !showDockingMode && !showShortestPathMode) {
                bool shipPrefsHover = shipPrefsButton.getGlobalBounds().contains(mousePos);
                if (shipPrefsHover)
                {
                    shipPrefsButton.setFillColor(Color(150, 50, 60, 250));
                    shipPrefsButton.setOutlineColor(Color(255, 150, 150));
                }
                else
                {
                    shipPrefsButton.setFillColor(Color(100, 30, 40, 230));
                    shipPrefsButton.setOutlineColor(Color(220, 100, 100));
                }
                window.draw(shipPrefsButton);
                window.draw(shipPrefsButtonText);
            }
            
            
            if (!showInputWindow && !showDockingMode && !showShortestPathMode) {
                
                RectangleShape zoomPanel(Vector2f(150, 50));
                zoomPanel.setFillColor(Color(40, 20, 25, 200));
                zoomPanel.setOutlineThickness(2);
                zoomPanel.setOutlineColor(Color(150, 70, 70));
                zoomPanel.setPosition(1750, 950);
                window.draw(zoomPanel);
                
                
                RectangleShape zoomOutBtn(Vector2f(40, 40));
                zoomOutBtn.setPosition(1755, 955);
                bool zoomOutHover = zoomOutBtn.getGlobalBounds().contains(mousePos);
                zoomOutBtn.setFillColor(zoomOutHover ? Color(150, 60, 60) : Color(80, 35, 40));
                zoomOutBtn.setOutlineThickness(2);
                zoomOutBtn.setOutlineColor(zoomOutHover ? Color(255, 120, 120) : Color(120, 60, 60));
                window.draw(zoomOutBtn);
                
                Text zoomOutTxt("-", font, 28);
                zoomOutTxt.setStyle(Text::Bold);
                zoomOutTxt.setFillColor(Color::White);
                zoomOutTxt.setPosition(1768, 955);
                window.draw(zoomOutTxt);
                
                
                int zoomPercent = static_cast<int>(mapZoom * 100);
                Text zoomText(to_string(zoomPercent) + "%", font, 14);
                zoomText.setFillColor(Color(200, 150, 150));
                FloatRect ztB = zoomText.getLocalBounds();
                zoomText.setPosition(1825 - ztB.width / 2, 967);
                window.draw(zoomText);
                
                
                RectangleShape zoomInBtn(Vector2f(40, 40));
                zoomInBtn.setPosition(1855, 955);
                bool zoomInHover = zoomInBtn.getGlobalBounds().contains(mousePos);
                zoomInBtn.setFillColor(zoomInHover ? Color(150, 60, 60) : Color(80, 35, 40));
                zoomInBtn.setOutlineThickness(2);
                zoomInBtn.setOutlineColor(zoomInHover ? Color(255, 120, 120) : Color(120, 60, 60));
                window.draw(zoomInBtn);
                
                Text zoomInTxt("+", font, 24);
                zoomInTxt.setStyle(Text::Bold);
                zoomInTxt.setFillColor(Color::White);
                zoomInTxt.setPosition(1867, 959);
                window.draw(zoomInTxt);
                
                
                static bool zoomBtnClicked = false;
                if (Mouse::isButtonPressed(Mouse::Button::Left)) {
                    if (!zoomBtnClicked) {
                        if (zoomOutHover) {
                            clickSound.play();
                            targetZoom -= ZOOM_SPEED * 2;
                            if (targetZoom < MIN_ZOOM) targetZoom = MIN_ZOOM;
                            zoomBtnClicked = true;
                        }
                        if (zoomInHover) {
                            clickSound.play();
                            targetZoom += ZOOM_SPEED * 2;
                            if (targetZoom > MAX_ZOOM) targetZoom = MAX_ZOOM;
                            zoomBtnClicked = true;
                        }
                    }
                } else {
                    zoomBtnClicked = false;
                }
                
                
                RectangleShape resetZoomBtn(Vector2f(60, 25));
                resetZoomBtn.setPosition(1795, 1010);
                bool resetZoomHover = resetZoomBtn.getGlobalBounds().contains(mousePos);
                resetZoomBtn.setFillColor(resetZoomHover ? Color(100, 50, 55) : Color(60, 30, 35));
                resetZoomBtn.setOutlineThickness(1);
                resetZoomBtn.setOutlineColor(Color(120, 60, 60));
                window.draw(resetZoomBtn);
                
                Text resetTxt("Reset", font, 11);
                resetTxt.setFillColor(Color(200, 150, 150));
                FloatRect rtB = resetTxt.getLocalBounds();
                resetTxt.setPosition(1825 - rtB.width / 2, 1013);
                window.draw(resetTxt);
                
                
                static bool resetClicked = false;
                if (Mouse::isButtonPressed(Mouse::Button::Left) && resetZoomHover) {
                    if (!resetClicked) {
                        clickSound.play();
                        targetZoom = 1.0f;
                        targetOffsetX = 0.0f;
                        targetOffsetY = 0.0f;
                        resetClicked = true;
                    }
                } else if (!Mouse::isButtonPressed(Mouse::Button::Left)) {
                    resetClicked = false;
                }
                
                
                Text zoomHint("Scroll to zoom | Middle-drag to pan", font, 11);
                zoomHint.setFillColor(Color(150, 100, 100, 180));
                FloatRect zhB = zoomHint.getLocalBounds();
                zoomHint.setPosition(1825 - zhB.width / 2, 1040);
                window.draw(zoomHint);
            }

            if (showInputWindow)
            {
                
                RectangleShape overlay(Vector2f(1920, 1080));
                overlay.setFillColor(Color(15, 5, 8, 180));
                window.draw(overlay);
                
                
                window.draw(inputWindowGlow);
                
                window.draw(inputWindow);
                
                
                window.draw(inputWindowInner);

                window.draw(windowTitle);

                originInput.draw(window);
                destinationInput.draw(window);
                dateInput.draw(window);

                bool popupSearchHover = popupSearchButton.getGlobalBounds().contains(mousePos);
                bool closeHover = closeButton.getGlobalBounds().contains(mousePos);

                
                window.draw(popupSearchButtonGlow);
                window.draw(closeButtonGlow);

                if (popupSearchHover)
                {
                    popupSearchButton.setFillColor(Color(200, 40, 50, 255));
                    popupSearchButton.setOutlineColor(Color(255, 80, 80));
                    popupSearchText.setFillColor(Color(255, 220, 220));
                }
                else
                {
                    popupSearchButton.setFillColor(Color(150, 20, 30, 250));
                    popupSearchButton.setOutlineColor(Color(255, 60, 60));
                    popupSearchText.setFillColor(Color(255, 180, 180));
                }

                if (closeHover)
                {
                    closeButton.setFillColor(Color(70, 30, 35, 255));
                    closeButton.setOutlineColor(Color(180, 70, 70));
                    closeText.setFillColor(Color(255, 180, 180));
                }
                else
                {
                    closeButton.setFillColor(Color(40, 15, 20, 250));
                    closeButton.setOutlineColor(Color(120, 40, 40));
                    closeText.setFillColor(Color(180, 120, 120));
                }

                window.draw(popupSearchButton);
                window.draw(popupSearchText);
                window.draw(closeButton);
                window.draw(closeText);

                if (showSearchResult)
                {
                    Text resultText;
                    resultText.setFont(font);
                    resultText.setString(searchResultText);
                    resultText.setCharacterSize(20);
                    resultText.setFillColor(searchResultColor);
                    resultText.setStyle(Text::Bold);
                    resultText.setOutlineThickness(1);
                    resultText.setOutlineColor(Color(0, 0, 0, 150));
                    resultText.setPosition(750, 670);
                    window.draw(resultText);

                    if (!errorDetails.empty())
                    {
                        Text detailsText;
                        detailsText.setFont(font);
                        detailsText.setString(errorDetails);
                        detailsText.setCharacterSize(16);
                        detailsText.setFillColor(searchResultColor == Color::Green ? Color::Cyan : Color::Yellow);
                        detailsText.setStyle(Text::Bold);
                        detailsText.setPosition(730, 700);
                        window.draw(detailsText);
                    }
                }
            }

            if (hoveredRoute != nullptr && !showInputWindow && hoveredAnimIndex != -1)
            {
                RouteAnimation &anim = activeAnimations[hoveredAnimIndex];
                
                float boxWidth = 480.f, boxHeight = 420.f;
                float boxX = mousePos.x + 20.f;
                float boxY = mousePos.y - 400.f;
                if (boxX + boxWidth > 1920)
                    boxX = 1920 - boxWidth - 20;
                if (boxY < 0)
                    boxY = 20;

                RectangleShape infoBox(Vector2f(boxWidth, boxHeight));
                infoBox.setFillColor(Color(40, 15, 25, 245));
                infoBox.setOutlineThickness(3.f);
                infoBox.setOutlineColor(Color(200, 80, 80));
                infoBox.setPosition(boxX, boxY);

                VertexArray gradient(Quads, 4);
                gradient[0].position = Vector2f(boxX, boxY);
                gradient[1].position = Vector2f(boxX + boxWidth, boxY);
                gradient[2].position = Vector2f(boxX + boxWidth, boxY + boxHeight);
                gradient[3].position = Vector2f(boxX, boxY + boxHeight);

                gradient[0].color = Color(60, 25, 35, 245);
                gradient[1].color = Color(40, 15, 25, 245);
                gradient[2].color = Color(30, 10, 20, 245);
                gradient[3].color = Color(60, 25, 35, 245);

                window.draw(gradient);
                window.draw(infoBox);

                Text header;
                header.setFont(font);
                header.setString("ROUTE SEGMENT DETAILS");
                header.setCharacterSize(18);
                header.setFillColor(Color(255, 150, 150));
                header.setStyle(Text::Bold);
                header.setPosition(boxX + 15.f, boxY + 10.f);
                window.draw(header);

                
                int segmentCost = hoveredRoute->cost;
                int destDockWait = getPortDockWaitMinutesByName(hoveredRoute->destinationPort);
                
                
                float portCharge = 100.0f;
                for (int p = 0; p < numLocations; p++) {
                    if (ports[p].portName == hoveredRoute->destinationPort) {
                        portCharge = ports[p].charge;
                        break;
                    }
                }
                
                
                int layoverCost = static_cast<int>(portCharge * (destDockWait / 1440.0f));
                
                
                int totalPathCost = anim.totalCost;
                int totalPathTime = anim.totalTime; 
                
                
                int basePathCost = 0;
                int baseTravelTime = 0;
                for (int r = 0; r < anim.pathRoutes.getSize(); r++) {
                    if (anim.pathRoutes[r] != nullptr) {
                        basePathCost += anim.pathRoutes[r]->cost;
                    }
                }
                
                
                int totalDockingWait = 0;
                for (int pi = 0; pi < anim.path.getSize(); pi++) {
                    totalDockingWait += getPortDockWaitMinutes(anim.path[pi]);
                }
                
                
                int dockingPenalty = totalPathCost - basePathCost;
                
                string dockWaitStr = destDockWait > 0 ? 
                    to_string(destDockWait/60) + "h " + to_string(destDockWait%60) + "m" : "None";

                Text details;
                details.setFont(font);
                details.setCharacterSize(13);
                details.setFillColor(Color(255, 220, 220));
                details.setPosition(boxX + 15.f, boxY + 35.f);

                string infoText = "From: " + hoveredRoute->startingPort + "\n" +
                                  "To: " + hoveredRoute->destinationPort + "\n" +
                                  "Searched Date: " + userSearchedDate + "\n" +
                                  "Route Valid On: " + hoveredRoute->departureDate + "\n" +
                                  "Departure: " + hoveredRoute->departureTime + "  Arrival: " + hoveredRoute->arrivalTime + "\n" +
                                  "Company: " + hoveredRoute->shippingCompany + "\n" +
                                  "Segment Cost: $" + to_string(segmentCost);

                details.setString(infoText);
                window.draw(details);
                
                
                RectangleShape costBox(Vector2f(boxWidth - 30, 100));
                costBox.setFillColor(Color(30, 10, 15, 200));
                costBox.setOutlineThickness(1);
                costBox.setOutlineColor(Color(150, 60, 60));
                costBox.setPosition(boxX + 15, boxY + 135);
                window.draw(costBox);
                
                Text costHeader;
                costHeader.setFont(font);
                costHeader.setString("TOTAL PATH COST BREAKDOWN");
                costHeader.setCharacterSize(12);
                costHeader.setFillColor(Color(255, 180, 100));
                costHeader.setStyle(Text::Bold);
                costHeader.setPosition(boxX + 25, boxY + 138);
                window.draw(costHeader);
                
                
                Text baseCostTxt;
                baseCostTxt.setFont(font);
                baseCostTxt.setString("Base Route Cost:     $" + to_string(basePathCost));
                baseCostTxt.setCharacterSize(11);
                baseCostTxt.setFillColor(Color(200, 200, 200));
                baseCostTxt.setPosition(boxX + 25, boxY + 155);
                window.draw(baseCostTxt);
                
                
                Text dockCostTxt;
                dockCostTxt.setFont(font);
                dockCostTxt.setString("Layover Penalty:     $" + to_string(dockingPenalty) + " (charge x hrs/24)");
                dockCostTxt.setCharacterSize(11);
                dockCostTxt.setFillColor(dockingPenalty > 0 ? Color(255, 180, 100) : Color(150, 150, 150));
                dockCostTxt.setPosition(boxX + 25, boxY + 170);
                window.draw(dockCostTxt);
                
                
                Text segLayover;
                segLayover.setFont(font);
                segLayover.setString("This Port Queue:     " + dockWaitStr + " (+$" + to_string(layoverCost) + ")");
                segLayover.setCharacterSize(11);
                segLayover.setFillColor(Color(200, 180, 255));
                segLayover.setPosition(boxX + 25, boxY + 185);
                window.draw(segLayover);
                
                
                RectangleShape sepLine(Vector2f(boxWidth - 60, 1));
                sepLine.setFillColor(Color(150, 80, 80));
                sepLine.setPosition(boxX + 25, boxY + 202);
                window.draw(sepLine);
                
                
                Text totalCostTxt;
                totalCostTxt.setFont(font);
                totalCostTxt.setString("TOTAL PATH COST:     $" + to_string(totalPathCost));
                totalCostTxt.setCharacterSize(13);
                totalCostTxt.setFillColor(Color(100, 255, 150));
                totalCostTxt.setStyle(Text::Bold);
                totalCostTxt.setPosition(boxX + 25, boxY + 210);
                window.draw(totalCostTxt);
                
                
                RectangleShape timeBox(Vector2f(boxWidth - 30, 100));
                timeBox.setFillColor(Color(10, 20, 40, 200));
                timeBox.setOutlineThickness(1);
                timeBox.setOutlineColor(Color(80, 120, 180));
                timeBox.setPosition(boxX + 15, boxY + 240);
                window.draw(timeBox);
                
                Text timeHeader;
                timeHeader.setFont(font);
                timeHeader.setString("TOTAL PATH TIME BREAKDOWN");
                timeHeader.setCharacterSize(12);
                timeHeader.setFillColor(Color(100, 180, 255));
                timeHeader.setStyle(Text::Bold);
                timeHeader.setPosition(boxX + 25, boxY + 243);
                window.draw(timeHeader);
                
                
                int baseTravelMinutes = totalPathTime - totalDockingWait;
                if (baseTravelMinutes < 0) baseTravelMinutes = totalPathTime;
                
                
                int baseDays = baseTravelMinutes / (24 * 60);
                int baseHrs = (baseTravelMinutes % (24 * 60)) / 60;
                int baseMins = baseTravelMinutes % 60;
                string baseTimeStr = to_string(baseDays) + "d " + to_string(baseHrs) + "h " + to_string(baseMins) + "m";
                
                int dockDays = totalDockingWait / (24 * 60);
                int dockHrs = (totalDockingWait % (24 * 60)) / 60;
                int dockMins = totalDockingWait % 60;
                string dockTimeStr = to_string(dockDays) + "d " + to_string(dockHrs) + "h " + to_string(dockMins) + "m";
                
                int totalDays = totalPathTime / (24 * 60);
                int totalHrs = (totalPathTime % (24 * 60)) / 60;
                int totalMins = totalPathTime % 60;
                string totalTimeStr = to_string(totalDays) + "d " + to_string(totalHrs) + "h " + to_string(totalMins) + "m";
                
                
                Text baseTravelTxt;
                baseTravelTxt.setFont(font);
                baseTravelTxt.setString("Travel Time:         " + baseTimeStr);
                baseTravelTxt.setCharacterSize(11);
                baseTravelTxt.setFillColor(Color(200, 200, 200));
                baseTravelTxt.setPosition(boxX + 25, boxY + 260);
                window.draw(baseTravelTxt);
                
                
                Text dockTimeTxt;
                dockTimeTxt.setFont(font);
                dockTimeTxt.setString("Docking Wait:        " + dockTimeStr);
                dockTimeTxt.setCharacterSize(11);
                dockTimeTxt.setFillColor(totalDockingWait > 0 ? Color(255, 180, 100) : Color(150, 150, 150));
                dockTimeTxt.setPosition(boxX + 25, boxY + 275);
                window.draw(dockTimeTxt);
                
                
                RectangleShape timeSepLine(Vector2f(boxWidth - 60, 1));
                timeSepLine.setFillColor(Color(80, 120, 180));
                timeSepLine.setPosition(boxX + 25, boxY + 295);
                window.draw(timeSepLine);
                
                
                Text totalTimeTxt;
                totalTimeTxt.setFont(font);
                totalTimeTxt.setString("TOTAL JOURNEY:       " + totalTimeStr);
                totalTimeTxt.setCharacterSize(13);
                totalTimeTxt.setFillColor(Color(100, 200, 255));
                totalTimeTxt.setStyle(Text::Bold);
                totalTimeTxt.setPosition(boxX + 25, boxY + 305);
                window.draw(totalTimeTxt);
                
                
                Text algoIndicator;
                algoIndicator.setFont(font);
                string algoStr = anim.routeType.empty() ? "Standard" : anim.routeType;
                algoIndicator.setString("Algorithm: " + algoStr);
                algoIndicator.setCharacterSize(11);
                algoIndicator.setFillColor(Color(180, 150, 255));
                algoIndicator.setPosition(boxX + 25, boxY + 350);
                window.draw(algoIndicator);
                
                
                Text segCount;
                segCount.setFont(font);
                segCount.setString("Path Segments: " + to_string(anim.pathRoutes.getSize()));
                segCount.setCharacterSize(11);
                segCount.setFillColor(Color(150, 200, 150));
                segCount.setPosition(boxX + 25, boxY + 365);
                window.draw(segCount);

                static float boxPulse = 0.0f;
                boxPulse += 0.1f;
                float pulseValue = 0.5f + 0.5f * sin(boxPulse);
                infoBox.setOutlineColor(Color(200, 80, 80, 150 + 105 * pulseValue));
            }

            Text title;
            title.setFont(font);
            title.setString("OceanRoute Navigator");
            title.setCharacterSize(28);
            title.setFillColor(Color(200, 230, 255));
            title.setStyle(Text::Bold);
            title.setPosition(20, 20);
            window.draw(title);

            
            if (selectedPortIndex != -1 && showRoutes && activeAnimations.getSize() > 0)
            {
                
                RectangleShape routePanel(Vector2f(380, 500));
                routePanel.setFillColor(Color(0, 30, 60, 230));
                routePanel.setOutlineThickness(3);
                routePanel.setOutlineColor(Color(100, 200, 255));
                routePanel.setPosition(20, 430);
                window.draw(routePanel);

                
                Text panelTitle;
                panelTitle.setFont(font);
                panelTitle.setString("Routes from " + string(portLocations[selectedPortIndex].name));
                panelTitle.setCharacterSize(18);
                panelTitle.setFillColor(Color(100, 255, 255));
                panelTitle.setStyle(Text::Bold);
                panelTitle.setPosition(35, 440);
                window.draw(panelTitle);

                
                float yOffset = 475;
                for (int animIdx = 0; animIdx < activeAnimations.getSize() && animIdx < 10; animIdx++)
                {
                    RouteAnimation &anim = activeAnimations[animIdx];

                    
                    RectangleShape colorBar(Vector2f(8, 35));
                    colorBar.setFillColor(anim.routeColor);
                    colorBar.setPosition(30, yOffset);
                    window.draw(colorBar);

                    
                    Text routeText;
                    routeText.setFont(font);

                    string routeInfo = "";
                    if (anim.pathRoutes.getSize() > 0)
                    {
                        RouteNode *r = anim.pathRoutes[0];
                        routeInfo = "To: " + r->destinationPort + "\n";
                        routeInfo += "$" + to_string(r->cost) + " | " + r->departureDate + "\n";
                        routeInfo += r->departureTime + " -> " + r->arrivalTime + " | " + r->shippingCompany;
                    }
                    else
                    {
                        routeInfo = anim.routeType;
                    }

                    routeText.setString(routeInfo);
                    routeText.setCharacterSize(12);
                    routeText.setFillColor(Color(200, 230, 255));
                    routeText.setPosition(45, yOffset);
                    window.draw(routeText);

                    yOffset += 45;
                }

                
                if (activeAnimations.getSize() > 10)
                {
                    Text moreText;
                    moreText.setFont(font);
                    moreText.setString("+" + to_string(activeAnimations.getSize() - 10) + " more routes...");
                    moreText.setCharacterSize(12);
                    moreText.setFillColor(Color::Yellow);
                    moreText.setPosition(45, yOffset);
                    window.draw(moreText);
                }
            }

            
            if (showDockingMode)
            {
                
                RectangleShape dockOverlay(Vector2f(1920, 1080));
                dockOverlay.setFillColor(Color(0, 20, 40, 220));
                window.draw(dockOverlay);
                
                
                if (showPortSelection)
                {
                    Text selTitle("SELECT PORT FOR DOCKING", font, 38);
                    selTitle.setStyle(Text::Bold);
                    selTitle.setFillColor(Color(100, 200, 255));
                    selTitle.setOutlineColor(Color(0, 50, 100));
                    selTitle.setOutlineThickness(2);
                    FloatRect stB = selTitle.getLocalBounds();
                    selTitle.setPosition(960 - stB.width / 2, 50);
                    window.draw(selTitle);
                    
                    Text selSub("Click on a port to manage its docking. Ships persist after closing.", font, 16);
                    selSub.setFillColor(Color(150, 200, 230));
                    FloatRect ssB = selSub.getLocalBounds();
                    selSub.setPosition(960 - ssB.width / 2, 100);
                    window.draw(selSub);
                    
                    
                    float startX = 100, startY = 150;
                    float cardW = 280, cardH = 90, gapX = 20, gapY = 15;
                    int cols = 6;
                    
                    for (int p = 0; p < numLocations; p++) {
                        int col = p % cols;
                        int row = p / cols;
                        float cx = startX + col * (cardW + gapX);
                        float cy = startY + row * (cardH + gapY) - portScrollOffset;
                        
                        if (cy < 130 || cy > 950) continue; 
                        
                        RectangleShape card(Vector2f(cardW, cardH));
                        card.setPosition(cx, cy);
                        
                        bool hover = card.getGlobalBounds().contains(mousePos);
                        int docked = globalPortDocking[p].dockedShips.getSize();
                        int queued = globalPortDocking[p].queuedShips.getSize();
                        int waitMins = globalPortDocking[p].calculateWaitTimeMinutes();
                        
                        if (hover) {
                            card.setFillColor(Color(40, 80, 120));
                            card.setOutlineColor(Color(100, 200, 255));
                        } else {
                            card.setFillColor(Color(25, 50, 80));
                            card.setOutlineColor(docked > 0 ? Color(100, 200, 100) : Color(80, 120, 160));
                        }
                        card.setOutlineThickness(2);
                        window.draw(card);
                        
                        Text pName(portLocations[p].name, font, 18);
                        pName.setStyle(Text::Bold);
                        pName.setFillColor(Color::White);
                        pName.setPosition(cx + 10, cy + 8);
                        window.draw(pName);
                        
                        Text dockInfo("Docks: " + to_string(docked) + "/4  Queue: " + to_string(queued), font, 13);
                        dockInfo.setFillColor(Color(150, 200, 220));
                        dockInfo.setPosition(cx + 10, cy + 35);
                        window.draw(dockInfo);
                        
                        string waitStr = waitMins > 0 ? "Wait: " + to_string(waitMins / 60) + "h " + to_string(waitMins % 60) + "m" : "No wait";
                        Text waitInfo(waitStr, font, 12);
                        waitInfo.setFillColor(waitMins > 0 ? Color(255, 200, 100) : Color(100, 255, 150));
                        waitInfo.setPosition(cx + 10, cy + 58);
                        window.draw(waitInfo);
                        
                        
                        for (int d = 0; d < 4; d++) {
                            CircleShape dInd(8);
                            dInd.setPosition(cx + cardW - 90 + d * 20, cy + 55);
                            bool occupied = d < docked;
                            dInd.setFillColor(occupied ? Color(50, 200, 100) : Color(50, 70, 90));
                            dInd.setOutlineColor(occupied ? Color(100, 255, 150) : Color(80, 100, 120));
                            dInd.setOutlineThickness(1);
                            window.draw(dInd);
                        }
                        
                        
                        if (hover && Mouse::isButtonPressed(Mouse::Button::Left) && !dockAddBtnClicked) {
                            clickSound.play();
                            selectedDockingPort = p;
                            showPortSelection = false;
                            arrivingShips.clear();
                            departingShips.clear();
                            dockAddBtnClicked = true;
                        }
                    }
                    
                    if (!Mouse::isButtonPressed(Mouse::Button::Left)) dockAddBtnClicked = false;
                    
                    
                    Text scrollHint("Scroll to see more ports", font, 14);
                    scrollHint.setFillColor(Color(120, 150, 180));
                    FloatRect shB = scrollHint.getLocalBounds();
                    scrollHint.setPosition(960 - shB.width / 2, 980);
                    window.draw(scrollHint);
                    
                    
                    RectangleShape closeBtn(Vector2f(140, 45));
                    closeBtn.setPosition(890, 1010);
                    bool closeHover = closeBtn.getGlobalBounds().contains(mousePos);
                    closeBtn.setFillColor(closeHover ? Color(180, 80, 80) : Color(140, 60, 60));
                    closeBtn.setOutlineColor(Color(255, 150, 150));
                    closeBtn.setOutlineThickness(2);
                    window.draw(closeBtn);
                    
                    Text closeTxt("CLOSE", font, 18);
                    closeTxt.setFillColor(Color::White);
                    FloatRect ctB = closeTxt.getLocalBounds();
                    closeTxt.setPosition(960 - ctB.width / 2, 1018);
                    window.draw(closeTxt);
                    
                    if (closeHover && Mouse::isButtonPressed(Mouse::Button::Left)) {
                        showDockingMode = false;
                    }
                }
                
                else if (selectedDockingPort >= 0 && selectedDockingPort < numLocations)
                {
                    PortDockingState& portDock = globalPortDocking[selectedDockingPort];
                    
                    
                    for (int i = arrivingShips.getSize() - 1; i >= 0; i--) {
                        arrivingShips[i].animProgress += deltaTime * 1.5f;
                        if (arrivingShips[i].animProgress >= 1.0f) {
                            
                            GlobalDockShipInfo gShip;
                            gShip.shipId = arrivingShips[i].shipId;
                            gShip.company = arrivingShips[i].company;
                            gShip.dockTimer = arrivingShips[i].maxTime;
                            gShip.maxTime = arrivingShips[i].maxTime;
                            gShip.dockSlot = arrivingShips[i].dockSlot;
                            portDock.dockedShips.push_back(gShip);
                            
                            Vector<DockShipInfo> newArr;
                            for (int j = 0; j < arrivingShips.getSize(); j++) {
                                if (j != i) newArr.push_back(arrivingShips[j]);
                            }
                            arrivingShips = newArr;
                        }
                    }
                    
                    
                    for (int i = departingShips.getSize() - 1; i >= 0; i--) {
                        departingShips[i].animProgress += deltaTime * 1.5f;
                        if (departingShips[i].animProgress >= 1.0f) {
                            Vector<DockShipInfo> newDep;
                            for (int j = 0; j < departingShips.getSize(); j++) {
                                if (j != i) newDep.push_back(departingShips[j]);
                            }
                            departingShips = newDep;
                        }
                    }
                    
                    
                    for (int i = 0; i < portDock.dockedShips.getSize(); i++) {
                        portDock.dockedShips[i].dockTimer -= deltaTime * dockTimeMultiplier;
                        if (portDock.dockedShips[i].dockTimer <= 0) {
                            
                            DockShipInfo departing;
                            departing.shipId = portDock.dockedShips[i].shipId;
                            departing.company = portDock.dockedShips[i].company;
                            departing.dockSlot = portDock.dockedShips[i].dockSlot;
                            departing.isDeparting = true;
                            departing.isArriving = false;
                            departing.animProgress = 0;
                            departingShips.push_back(departing);
                            
                            int freedSlot = portDock.dockedShips[i].dockSlot;
                            
                            
                            Vector<GlobalDockShipInfo> newDocked;
                            for (int j = 0; j < portDock.dockedShips.getSize(); j++) {
                                if (j != i) newDocked.push_back(portDock.dockedShips[j]);
                            }
                            portDock.dockedShips = newDocked;
                            
                            
                            if (portDock.queuedShips.getSize() > 0) {
                                GlobalQueueShipInfo nextShip = portDock.queuedShips[0];
                                Vector<GlobalQueueShipInfo> newQueue;
                                for (int j = 1; j < portDock.queuedShips.getSize(); j++) {
                                    portDock.queuedShips[j].queuePosition--;
                                    newQueue.push_back(portDock.queuedShips[j]);
                                }
                                portDock.queuedShips = newQueue;
                                
                                DockShipInfo newDockShip;
                                newDockShip.shipId = nextShip.shipId;
                                newDockShip.company = nextShip.company;
                                newDockShip.maxTime = 30.0f;
                                newDockShip.dockTimer = 30.0f;
                                newDockShip.animProgress = 0;
                                newDockShip.isArriving = true;
                                newDockShip.isDeparting = false;
                                newDockShip.dockSlot = freedSlot;
                                arrivingShips.push_back(newDockShip);
                            }
                            i--;
                        }
                    }
                
                
                Text dockTitle(portLocations[selectedDockingPort].name + " - PORT DOCKING", font, 38);
                dockTitle.setStyle(Text::Bold);
                dockTitle.setFillColor(Color(255, 100, 100));
                dockTitle.setOutlineColor(Color(60, 20, 20));
                dockTitle.setOutlineThickness(3);
                FloatRect dtBounds = dockTitle.getLocalBounds();
                dockTitle.setPosition(960 - dtBounds.width / 2, 30);
                window.draw(dockTitle);
                
                int waitMins = portDock.calculateWaitTimeMinutes();
                string waitStr = waitMins > 0 ? "Wait time: " + to_string(waitMins / 60) + "h " + to_string(waitMins % 60) + "m" : "No wait - dock available";
                Text dockSubtitle(waitStr + " | Ships persist after closing", font, 16);
                dockSubtitle.setFillColor(waitMins > 0 ? Color(255, 180, 100) : Color(150, 255, 150));
                FloatRect dsBounds = dockSubtitle.getLocalBounds();
                dockSubtitle.setPosition(960 - dsBounds.width / 2, 80);
                window.draw(dockSubtitle);
                
                
                float portCX = 700, portCY = 450, portR = 150;
                
                
                for (int ring = 4; ring >= 0; ring--) {
                    CircleShape pGlow(portR + ring * 15);
                    pGlow.setOrigin(portR + ring * 15, portR + ring * 15);
                    pGlow.setPosition(portCX, portCY);
                    pGlow.setFillColor(Color(180, 50, 50, 15 - ring * 3));
                    window.draw(pGlow);
                }
                
                
                CircleShape portC(portR);
                portC.setOrigin(portR, portR);
                portC.setPosition(portCX, portCY);
                portC.setFillColor(Color(60, 25, 30, 220));
                portC.setOutlineColor(Color(200, 80, 80));
                portC.setOutlineThickness(4);
                window.draw(portC);
                
                Text portLbl(portLocations[selectedDockingPort].name, font, 20);
                portLbl.setStyle(Text::Bold);
                portLbl.setFillColor(Color(255, 200, 200));
                FloatRect plB = portLbl.getLocalBounds();
                portLbl.setPosition(portCX - plB.width / 2, portCY - 12);
                window.draw(portLbl);
                
                
                for (int slot = 0; slot < MAX_DOCKS; slot++) {
                    float angle = (static_cast<float>(slot) / MAX_DOCKS) * 2.0f * 3.14159f - 3.14159f / 2;
                    float slotX = portCX + cos(angle) * (portR + 55);
                    float slotY = portCY + sin(angle) * (portR + 55);
                    
                    bool isOccupied = false;
                    for (int i = 0; i < portDock.dockedShips.getSize(); i++) {
                        if (portDock.dockedShips[i].dockSlot == slot) { isOccupied = true; break; }
                    }
                    
                    RectangleShape dSlot(Vector2f(70, 35));
                    dSlot.setOrigin(35, 17);
                    dSlot.setPosition(slotX, slotY);
                    dSlot.setRotation(angle * 180 / 3.14159f + 90);
                    dSlot.setFillColor(isOccupied ? Color(80, 160, 100, 220) : Color(80, 40, 50, 180));
                    dSlot.setOutlineColor(isOccupied ? Color(120, 255, 150) : Color(180, 100, 100));
                    dSlot.setOutlineThickness(2);
                    window.draw(dSlot);
                    
                    
                    float dashLen = 8.0f, gapLen = 6.0f, lineLen = 45.0f;
                    float startX = portCX + cos(angle) * portR;
                    float startY = portCY + sin(angle) * portR;
                    float dx = cos(angle), dy = sin(angle);
                    float curDist = 0;
                    while (curDist < lineLen) {
                        float endDist = (curDist + dashLen < lineLen) ? curDist + dashLen : lineLen;
                        Vertex dash[] = {
                            Vertex(Vector2f(startX + dx * curDist, startY + dy * curDist), 
                                   isOccupied ? Color(120, 255, 150, 180) : Color(180, 100, 100, 150)),
                            Vertex(Vector2f(startX + dx * endDist, startY + dy * endDist), 
                                   isOccupied ? Color(120, 255, 150, 180) : Color(180, 100, 100, 150))
                        };
                        window.draw(dash, 2, Lines);
                        curDist += dashLen + gapLen;
                    }
                    
                    Text slotNum(to_string(slot + 1), font, 14);
                    slotNum.setFillColor(Color(255, 220, 220));
                    FloatRect snB = slotNum.getLocalBounds();
                    slotNum.setPosition(slotX - snB.width / 2, slotY - snB.height / 2 - 22);
                    window.draw(slotNum);
                }
                
                
                for (int i = 0; i < portDock.dockedShips.getSize(); i++) {
                    int slot = portDock.dockedShips[i].dockSlot;
                    float angle = (static_cast<float>(slot) / MAX_DOCKS) * 2.0f * 3.14159f - 3.14159f / 2;
                    float shipX = portCX + cos(angle) * (portR + 55);
                    float shipY = portCY + sin(angle) * (portR + 55);
                    
                    Color shipCol = dockCompanyColors[0];
                    for (int c = 0; c < 8; c++) {
                        if (dockCompanies[c] == portDock.dockedShips[i].company) { shipCol = dockCompanyColors[c]; break; }
                    }
                    
                    RectangleShape shipBody(Vector2f(60, 26));
                    shipBody.setOrigin(30, 13);
                    shipBody.setPosition(shipX, shipY);
                    shipBody.setRotation(angle * 180 / 3.14159f + 90);
                    shipBody.setFillColor(shipCol);
                    shipBody.setOutlineColor(Color::White);
                    shipBody.setOutlineThickness(2);
                    window.draw(shipBody);
                    
                    
                    float timerRatio = portDock.dockedShips[i].dockTimer / portDock.dockedShips[i].maxTime;
                    float barW = 55.0f, barH = 7.0f;
                    RectangleShape timerBg(Vector2f(barW, barH));
                    timerBg.setOrigin(barW / 2, barH / 2);
                    timerBg.setPosition(shipX, shipY - 35);
                    timerBg.setFillColor(Color(50, 50, 50, 200));
                    timerBg.setOutlineColor(Color(100, 100, 100));
                    timerBg.setOutlineThickness(1);
                    window.draw(timerBg);
                    
                    RectangleShape timerBar(Vector2f(barW * timerRatio, barH - 2));
                    timerBar.setOrigin(barW / 2, (barH - 2) / 2);
                    timerBar.setPosition(shipX - (barW * (1 - timerRatio)) / 2, shipY - 35);
                    if (timerRatio > 0.5f) timerBar.setFillColor(Color(50, 200, 100));
                    else if (timerRatio > 0.2f) timerBar.setFillColor(Color(255, 200, 50));
                    else timerBar.setFillColor(Color(255, 80, 80));
                    window.draw(timerBar);
                    
                    int secsLeft = static_cast<int>(portDock.dockedShips[i].dockTimer);
                    Text timerTxt(to_string(secsLeft) + "s", font, 11);
                    timerTxt.setFillColor(Color::White);
                    FloatRect ttB = timerTxt.getLocalBounds();
                    timerTxt.setPosition(shipX - ttB.width / 2, shipY - 50);
                    window.draw(timerTxt);
                    
                    Text shipIdTxt(portDock.dockedShips[i].shipId, font, 10);
                    shipIdTxt.setFillColor(Color(200, 230, 255));
                    FloatRect siB = shipIdTxt.getLocalBounds();
                    shipIdTxt.setPosition(shipX - siB.width / 2, shipY + 22);
                    window.draw(shipIdTxt);
                }
                
                
                for (int i = 0; i < arrivingShips.getSize(); i++) {
                    int slot = arrivingShips[i].dockSlot;
                    float angle = (static_cast<float>(slot) / MAX_DOCKS) * 2.0f * 3.14159f - 3.14159f / 2;
                    float endX = portCX + cos(angle) * (portR + 55);
                    float endY = portCY + sin(angle) * (portR + 55);
                    float startX = portCX + cos(angle) * (portR + 200);
                    float startY = portCY + sin(angle) * (portR + 200);
                    float progress = arrivingShips[i].animProgress;
                    float curX = startX + (endX - startX) * progress;
                    float curY = startY + (endY - startY) * progress;
                    
                    Color shipCol = dockCompanyColors[0];
                    for (int c = 0; c < 8; c++) {
                        if (dockCompanies[c] == arrivingShips[i].company) { shipCol = dockCompanyColors[c]; break; }
                    }
                    
                    CircleShape arrGlow(22 + sin(time * 8) * 4);
                    arrGlow.setOrigin(22, 22);
                    arrGlow.setPosition(curX, curY);
                    arrGlow.setFillColor(Color(100, 255, 150, 80));
                    window.draw(arrGlow);
                    
                    RectangleShape shipBody(Vector2f(60, 26));
                    shipBody.setOrigin(30, 13);
                    shipBody.setPosition(curX, curY);
                    shipBody.setRotation(angle * 180 / 3.14159f + 90);
                    shipBody.setFillColor(shipCol);
                    shipBody.setOutlineColor(Color(100, 255, 150));
                    shipBody.setOutlineThickness(3);
                    window.draw(shipBody);
                    
                    Text arrLbl("ARRIVING", font, 9);
                    arrLbl.setFillColor(Color(100, 255, 150));
                    FloatRect alB = arrLbl.getLocalBounds();
                    arrLbl.setPosition(curX - alB.width / 2, curY + 18);
                    window.draw(arrLbl);
                }
                
                
                for (int i = 0; i < departingShips.getSize(); i++) {
                    int slot = departingShips[i].dockSlot;
                    float angle = (static_cast<float>(slot) / MAX_DOCKS) * 2.0f * 3.14159f - 3.14159f / 2;
                    float startX = portCX + cos(angle) * (portR + 55);
                    float startY = portCY + sin(angle) * (portR + 55);
                    float endX = portCX + cos(angle) * (portR + 300);
                    float endY = portCY + sin(angle) * (portR + 300);
                    float progress = departingShips[i].animProgress;
                    float curX = startX + (endX - startX) * progress;
                    float curY = startY + (endY - startY) * progress;
                    
                    Color shipCol = dockCompanyColors[0];
                    for (int c = 0; c < 8; c++) {
                        if (dockCompanies[c] == departingShips[i].company) { shipCol = dockCompanyColors[c]; break; }
                    }
                    float alpha = 255 * (1 - progress);
                    
                    RectangleShape shipBody(Vector2f(60, 26));
                    shipBody.setOrigin(30, 13);
                    shipBody.setPosition(curX, curY);
                    shipBody.setRotation(angle * 180 / 3.14159f + 90);
                    shipBody.setFillColor(Color(shipCol.r, shipCol.g, shipCol.b, static_cast<int>(alpha)));
                    shipBody.setOutlineColor(Color(255, 200, 100, static_cast<int>(alpha)));
                    shipBody.setOutlineThickness(2);
                    window.draw(shipBody);
                    
                    Text depLbl("DEPARTING", font, 9);
                    depLbl.setFillColor(Color(255, 200, 100, static_cast<int>(alpha)));
                    FloatRect dlB = depLbl.getLocalBounds();
                    depLbl.setPosition(curX - dlB.width / 2, curY + 18);
                    window.draw(depLbl);
                }
                
                
                float qX = 1100, qY = 150;
                RectangleShape qPanel(Vector2f(320, 450));
                qPanel.setPosition(qX, qY);
                qPanel.setFillColor(Color(50, 25, 35, 240));
                qPanel.setOutlineColor(Color(180, 80, 80));
                qPanel.setOutlineThickness(3);
                window.draw(qPanel);
                
                Text qTitle("WAITING QUEUE", font, 20);
                qTitle.setStyle(Text::Bold);
                qTitle.setFillColor(Color(255, 150, 100));
                FloatRect qtB = qTitle.getLocalBounds();
                qTitle.setPosition(qX + 160 - qtB.width / 2, qY + 12);
                window.draw(qTitle);
                
                Text qCount("Ships: " + to_string(portDock.queuedShips.getSize()) + "/" + to_string(MAX_QUEUE), font, 14);
                qCount.setFillColor(Color(220, 180, 180));
                qCount.setPosition(qX + 15, qY + 45);
                window.draw(qCount);
                
                
                for (int d = 0; d < 14; d++) {
                    RectangleShape dash(Vector2f(12, 2));
                    dash.setPosition(qX + 15 + d * 21, qY + 70);
                    dash.setFillColor(Color(180, 80, 80, 180));
                    window.draw(dash);
                }
                
                
                for (int i = 0; i < portDock.queuedShips.getSize(); i++) {
                    float shipY = qY + 90 + i * 55;
                    float wobble = sin(time * 2.0f + i * 0.5f) * 2;
                    
                    CircleShape posInd(16);
                    posInd.setOrigin(16, 16);
                    posInd.setPosition(qX + 35, shipY + 18 + wobble);
                    posInd.setFillColor(Color(120, 50, 60));
                    posInd.setOutlineColor(Color(200, 100, 100));
                    posInd.setOutlineThickness(2);
                    window.draw(posInd);
                    
                    Text posNum(to_string(i + 1), font, 16);
                    posNum.setStyle(Text::Bold);
                    posNum.setFillColor(Color::White);
                    FloatRect pnB = posNum.getLocalBounds();
                    posNum.setPosition(qX + 35 - pnB.width / 2, shipY + 10 + wobble);
                    window.draw(posNum);
                    
                    Color shipCol = dockCompanyColors[0];
                    for (int c = 0; c < 8; c++) {
                        if (dockCompanies[c] == portDock.queuedShips[i].company) { shipCol = dockCompanyColors[c]; break; }
                    }
                    RectangleShape qShip(Vector2f(45, 22));
                    qShip.setPosition(qX + 65, shipY + 7 + wobble);
                    qShip.setFillColor(shipCol);
                    qShip.setOutlineColor(Color::White);
                    qShip.setOutlineThickness(1);
                    window.draw(qShip);
                    
                    Text shipInfo(portDock.queuedShips[i].shipId + " - " + portDock.queuedShips[i].company, font, 12);
                    shipInfo.setFillColor(Color(255, 220, 220));
                    shipInfo.setPosition(qX + 120, shipY + 10 + wobble);
                    window.draw(shipInfo);
                }
                
                if (portDock.queuedShips.getSize() == 0) {
                    Text emptyMsg("No ships waiting", font, 14);
                    emptyMsg.setFillColor(Color(180, 120, 130));
                    emptyMsg.setStyle(Text::Italic);
                    FloatRect emB = emptyMsg.getLocalBounds();
                    emptyMsg.setPosition(qX + 160 - emB.width / 2, qY + 220);
                    window.draw(emptyMsg);
                }
                
                
                float stX = 1100, stY = 620;
                RectangleShape stPanel(Vector2f(320, 200));
                stPanel.setPosition(stX, stY);
                stPanel.setFillColor(Color(50, 25, 35, 240));
                stPanel.setOutlineColor(Color(150, 80, 80));
                stPanel.setOutlineThickness(3);
                window.draw(stPanel);
                
                Text stTitle("DOCK STATUS", font, 20);
                stTitle.setStyle(Text::Bold);
                stTitle.setFillColor(Color(255, 120, 120));
                FloatRect stTB = stTitle.getLocalBounds();
                stTitle.setPosition(stX + 160 - stTB.width / 2, stY + 12);
                window.draw(stTitle);
                
                Text dockInfo("Docks: " + to_string(portDock.dockedShips.getSize()) + "/" + to_string(MAX_DOCKS), font, 16);
                dockInfo.setFillColor(Color(220, 180, 180));
                dockInfo.setPosition(stX + 15, stY + 50);
                window.draw(dockInfo);
                
                
                Text speedLbl("Speed:", font, 14);
                speedLbl.setFillColor(Color(200, 150, 150));
                speedLbl.setPosition(stX + 15, stY + 80);
                window.draw(speedLbl);
                
                string speeds[] = {"0.5x", "1x", "2x", "5x"};
                float mults[] = {0.5f, 1.0f, 2.0f, 5.0f};
                for (int s = 0; s < 4; s++) {
                    RectangleShape sBtn(Vector2f(55, 26));
                    sBtn.setPosition(stX + 15 + s * 65, stY + 105);
                    bool isActive = (abs(dockTimeMultiplier - mults[s]) < 0.01f);
                    bool isHover = sBtn.getGlobalBounds().contains(mousePos);
                    if (isActive) {
                        sBtn.setFillColor(Color(160, 60, 60));
                        sBtn.setOutlineColor(Color(255, 120, 120));
                    } else if (isHover) {
                        sBtn.setFillColor(Color(100, 50, 60));
                        sBtn.setOutlineColor(Color(200, 100, 100));
                    } else {
                        sBtn.setFillColor(Color(70, 35, 45));
                        sBtn.setOutlineColor(Color(120, 70, 70));
                    }
                    sBtn.setOutlineThickness(2);
                    window.draw(sBtn);
                    
                    Text sTxt(speeds[s], font, 12);
                    sTxt.setFillColor(Color::White);
                    FloatRect sTB = sTxt.getLocalBounds();
                    sTxt.setPosition(stX + 15 + s * 65 + 27 - sTB.width / 2, stY + 109);
                    window.draw(sTxt);
                    
                    if (isHover && Mouse::isButtonPressed(Mouse::Button::Left) && !dockAddBtnClicked) {
                        dockTimeMultiplier = mults[s];
                    }
                }
                
                
                RectangleShape addBtn(Vector2f(280, 40));
                addBtn.setPosition(stX + 20, stY + 145);
                bool addHover = addBtn.getGlobalBounds().contains(mousePos);
                if (addHover) {
                    addBtn.setFillColor(Color(180, 70, 70));
                    addBtn.setOutlineColor(Color(255, 130, 130));
                } else {
                    addBtn.setFillColor(Color(140, 50, 55));
                    addBtn.setOutlineColor(Color(200, 100, 100));
                }
                addBtn.setOutlineThickness(2);
                window.draw(addBtn);
                
                Text addTxt("+ ADD SHIP", font, 18);
                addTxt.setStyle(Text::Bold);
                addTxt.setFillColor(Color::White);
                FloatRect atB = addTxt.getLocalBounds();
                addTxt.setPosition(stX + 160 - atB.width / 2, stY + 152);
                window.draw(addTxt);
                
                if (addHover && Mouse::isButtonPressed(Mouse::Button::Left)) {
                    if (!dockAddBtnClicked) {
                        clickSound.play();
                        showAddShipDialog = true;
                        dockShipCounter++;
                        dockInputShipName = "SH-" + to_string(dockShipCounter);
                        dockInputCompany = dockCompanies[rand() % 8];
                        dockInputTime = "30";
                        dockAddBtnClicked = true;
                    }
                }
                if (!Mouse::isButtonPressed(Mouse::Button::Left)) dockAddBtnClicked = false;
                
                
                RectangleShape backPortBtn(Vector2f(180, 40));
                backPortBtn.setPosition(50, 950);
                bool backPortHover = backPortBtn.getGlobalBounds().contains(mousePos);
                if (backPortHover) {
                    backPortBtn.setFillColor(Color(120, 50, 60));
                    backPortBtn.setOutlineColor(Color(200, 100, 100));
                } else {
                    backPortBtn.setFillColor(Color(80, 35, 45));
                    backPortBtn.setOutlineColor(Color(150, 70, 70));
                }
                backPortBtn.setOutlineThickness(2);
                window.draw(backPortBtn);
                
                Text backPortTxt("< SELECT PORT", font, 16);
                backPortTxt.setFillColor(Color(255, 200, 200));
                FloatRect bptB = backPortTxt.getLocalBounds();
                backPortTxt.setPosition(140 - bptB.width / 2, 958);
                window.draw(backPortTxt);
                
                if (backPortHover && Mouse::isButtonPressed(Mouse::Button::Left) && !dockCancelClicked) {
                    clickSound.play();
                    showPortSelection = true;
                    selectedDockingPort = -1;
                    arrivingShips.clear();
                    departingShips.clear();
                    dockCancelClicked = true;
                }
                
                
                RectangleShape closeDockBtn(Vector2f(140, 40));
                closeDockBtn.setPosition(280, 950);
                bool closeDockHover = closeDockBtn.getGlobalBounds().contains(mousePos);
                if (closeDockHover) {
                    closeDockBtn.setFillColor(Color(180, 80, 80));
                    closeDockBtn.setOutlineColor(Color(255, 150, 150));
                } else {
                    closeDockBtn.setFillColor(Color(140, 60, 60));
                    closeDockBtn.setOutlineColor(Color(200, 100, 100));
                }
                closeDockBtn.setOutlineThickness(2);
                window.draw(closeDockBtn);
                
                Text closeDockTxt("CLOSE", font, 18);
                closeDockTxt.setFillColor(Color::White);
                FloatRect cdtB = closeDockTxt.getLocalBounds();
                closeDockTxt.setPosition(350 - cdtB.width / 2, 958);
                window.draw(closeDockTxt);
                
                if (closeDockHover && Mouse::isButtonPressed(Mouse::Button::Left)) {
                    clickSound.play();
                    showDockingMode = false;
                }
                
                
                Text dockInstr("Ships persist after closing! Dock wait time affects route calculations.", font, 13);
                dockInstr.setFillColor(Color(180, 120, 130));
                FloatRect diB = dockInstr.getLocalBounds();
                dockInstr.setPosition(700 - diB.width / 2, 830);
                window.draw(dockInstr);
                
                
                if (showAddShipDialog) {
                    RectangleShape dimOv(Vector2f(1920, 1080));
                    dimOv.setFillColor(Color(0, 0, 0, 180));
                    window.draw(dimOv);
                    
                    RectangleShape dialog(Vector2f(400, 300));
                    dialog.setOrigin(200, 150);
                    dialog.setPosition(700, 450);
                    dialog.setFillColor(Color(50, 25, 35, 250));
                    dialog.setOutlineColor(Color(200, 80, 80));
                    dialog.setOutlineThickness(4);
                    window.draw(dialog);
                    
                    Text dlgTitle("ADD SHIP TO " + portLocations[selectedDockingPort].name, font, 20);
                    dlgTitle.setStyle(Text::Bold);
                    dlgTitle.setFillColor(Color(255, 120, 120));
                    FloatRect dltB = dlgTitle.getLocalBounds();
                    dlgTitle.setPosition(700 - dltB.width / 2, 320);
                    window.draw(dlgTitle);
                    
                    
                    Text nameLbl("Ship ID:", font, 14);
                    nameLbl.setFillColor(Color(220, 180, 180));
                    nameLbl.setPosition(520, 360);
                    window.draw(nameLbl);
                    
                    RectangleShape nameBox(Vector2f(260, 30));
                    nameBox.setPosition(520, 380);
                    nameBox.setFillColor(Color(40, 20, 28));
                    nameBox.setOutlineColor(dockNameActive ? Color(255, 180, 100) : Color(150, 80, 80));
                    nameBox.setOutlineThickness(2);
                    window.draw(nameBox);
                    
                    Text nameTxt(dockInputShipName + (dockNameActive ? "|" : ""), font, 14);
                    nameTxt.setFillColor(Color::White);
                    nameTxt.setPosition(528, 385);
                    window.draw(nameTxt);
                    
                    
                    Text compLbl("Company:", font, 14);
                    compLbl.setFillColor(Color(220, 180, 180));
                    compLbl.setPosition(520, 420);
                    window.draw(compLbl);
                    
                    RectangleShape compBox(Vector2f(260, 30));
                    compBox.setPosition(520, 440);
                    compBox.setFillColor(Color(40, 20, 28));
                    compBox.setOutlineColor(dockCompanyActive ? Color(255, 180, 100) : Color(150, 80, 80));
                    compBox.setOutlineThickness(2);
                    window.draw(compBox);
                    
                    Text compTxt(dockInputCompany + (dockCompanyActive ? "|" : ""), font, 14);
                    compTxt.setFillColor(Color::White);
                    compTxt.setPosition(528, 445);
                    window.draw(compTxt);
                    
                    
                    Text timeLbl("Dock Time (sec):", font, 14);
                    timeLbl.setFillColor(Color(220, 180, 180));
                    timeLbl.setPosition(520, 480);
                    window.draw(timeLbl);
                    
                    RectangleShape timeBox(Vector2f(80, 30));
                    timeBox.setPosition(520, 500);
                    timeBox.setFillColor(Color(40, 20, 28));
                    timeBox.setOutlineColor(dockTimeActive ? Color(255, 180, 100) : Color(150, 80, 80));
                    timeBox.setOutlineThickness(2);
                    window.draw(timeBox);
                    
                    Text timeTxt(dockInputTime + (dockTimeActive ? "|" : ""), font, 14);
                    timeTxt.setFillColor(Color::White);
                    timeTxt.setPosition(528, 505);
                    window.draw(timeTxt);
                    
                    
                    if (Mouse::isButtonPressed(Mouse::Button::Left) && !dockConfirmClicked) {
                        if (nameBox.getGlobalBounds().contains(mousePos)) {
                            dockNameActive = true; dockCompanyActive = false; dockTimeActive = false;
                        } else if (compBox.getGlobalBounds().contains(mousePos)) {
                            dockNameActive = false; dockCompanyActive = true; dockTimeActive = false;
                        } else if (timeBox.getGlobalBounds().contains(mousePos)) {
                            dockNameActive = false; dockCompanyActive = false; dockTimeActive = true;
                        }
                    }
                    
                    
                    RectangleShape confirmBtn(Vector2f(110, 35));
                    confirmBtn.setPosition(560, 550);
                    bool confHover = confirmBtn.getGlobalBounds().contains(mousePos);
                    confirmBtn.setFillColor(confHover ? Color(80, 180, 100) : Color(60, 140, 80));
                    confirmBtn.setOutlineColor(Color(120, 220, 150));
                    confirmBtn.setOutlineThickness(2);
                    window.draw(confirmBtn);
                    
                    Text confTxt("CONFIRM", font, 16);
                    confTxt.setStyle(Text::Bold);
                    confTxt.setFillColor(Color::White);
                    FloatRect cfB = confTxt.getLocalBounds();
                    confTxt.setPosition(615 - cfB.width / 2, 556);
                    window.draw(confTxt);
                    
                    
                    RectangleShape cancelBtn(Vector2f(110, 35));
                    cancelBtn.setPosition(730, 550);
                    bool cancHover = cancelBtn.getGlobalBounds().contains(mousePos);
                    cancelBtn.setFillColor(cancHover ? Color(200, 80, 80) : Color(150, 55, 55));
                    cancelBtn.setOutlineColor(Color(255, 130, 130));
                    cancelBtn.setOutlineThickness(2);
                    window.draw(cancelBtn);
                    
                    Text cancTxt("CANCEL", font, 16);
                    cancTxt.setStyle(Text::Bold);
                    cancTxt.setFillColor(Color::White);
                    FloatRect cnB = cancTxt.getLocalBounds();
                    cancTxt.setPosition(785 - cnB.width / 2, 556);
                    window.draw(cancTxt);
                    
                    if (Mouse::isButtonPressed(Mouse::Button::Left)) {
                        if (!dockConfirmClicked && confHover) {
                            clickSound.play();
                            int dTime = 30;
                            if (!dockInputTime.empty()) {
                                dTime = stoi(dockInputTime);
                                if (dTime < 5) dTime = 5;
                                if (dTime > 300) dTime = 300;
                            }
                            
                            
                            PortDockingState& portDockRef = globalPortDocking[selectedDockingPort];
                            if (portDockRef.dockedShips.getSize() < MAX_DOCKS) {
                                int freeSlot = portDockRef.findFreeSlot();
                                
                                for (int a = 0; a < arrivingShips.getSize(); a++) {
                                    if (arrivingShips[a].dockSlot == freeSlot) {
                                        
                                        for (int s = 0; s < MAX_DOCKS; s++) {
                                            bool ok = true;
                                            for (int d = 0; d < portDockRef.dockedShips.getSize(); d++) {
                                                if (portDockRef.dockedShips[d].dockSlot == s) { ok = false; break; }
                                            }
                                            for (int aa = 0; aa < arrivingShips.getSize(); aa++) {
                                                if (arrivingShips[aa].dockSlot == s) { ok = false; break; }
                                            }
                                            if (ok) { freeSlot = s; break; }
                                        }
                                    }
                                }
                                if (freeSlot >= 0) {
                                    DockShipInfo newShip;
                                    newShip.shipId = dockInputShipName;
                                    newShip.company = dockInputCompany;
                                    newShip.maxTime = static_cast<float>(dTime);
                                    newShip.dockTimer = static_cast<float>(dTime);
                                    newShip.animProgress = 0;
                                    newShip.isArriving = true;
                                    newShip.isDeparting = false;
                                    newShip.dockSlot = freeSlot;
                                    arrivingShips.push_back(newShip);
                                }
                            } else if (portDockRef.queuedShips.getSize() < MAX_QUEUE) {
                                
                                GlobalQueueShipInfo qShip;
                                qShip.shipId = dockInputShipName;
                                qShip.company = dockInputCompany;
                                qShip.queuePosition = portDockRef.queuedShips.getSize();
                                portDockRef.queuedShips.push_back(qShip);
                            }
                            
                            showAddShipDialog = false;
                            dockNameActive = false;
                            dockCompanyActive = false;
                            dockTimeActive = false;
                            dockConfirmClicked = true;
                        }
                        if (!dockCancelClicked && cancHover) {
                            clickSound.play();
                            showAddShipDialog = false;
                            dockNameActive = false;
                            dockCompanyActive = false;
                            dockTimeActive = false;
                            dockCancelClicked = true;
                        }
                    }
                    if (!Mouse::isButtonPressed(Mouse::Button::Left)) {
                        dockConfirmClicked = false;
                        dockCancelClicked = false;
                    }
                }
                } 
            }

            
            if (showShortestPathMode)
            {
                
                RectangleShape spOverlay(Vector2f(1920, 1080));
                spOverlay.setFillColor(Color(10, 5, 8, 220));
                window.draw(spOverlay);
                
                
                RectangleShape topVignette(Vector2f(1920, 80));
                topVignette.setFillColor(Color(150, 20, 20, 80));
                topVignette.setPosition(0, 0);
                window.draw(topVignette);
                
                RectangleShape bottomVignette(Vector2f(1920, 80));
                bottomVignette.setFillColor(Color(150, 20, 20, 80));
                bottomVignette.setPosition(0, 1000);
                window.draw(bottomVignette);
                
                
                if (showAlgorithmSelection)
                {
                    Text spTitle("SHORTEST PATH ALGORITHM", font, 48);
                    spTitle.setStyle(Text::Bold);
                    spTitle.setFillColor(Color(255, 50, 50));
                    spTitle.setOutlineColor(Color(100, 0, 0));
                    spTitle.setOutlineThickness(4);
                    spTitle.setLetterSpacing(1.8f);
                    FloatRect spTB = spTitle.getLocalBounds();
                    spTitle.setPosition(960 - spTB.width / 2, 60);
                    window.draw(spTitle);
                    
                    
                    RectangleShape titleUnderline(Vector2f(spTB.width + 40, 4));
                    titleUnderline.setFillColor(Color(255, 50, 50, 180));
                    titleUnderline.setPosition(960 - spTB.width / 2 - 20, 125);
                    window.draw(titleUnderline);
                    
                    Text spSub("Choose your pathfinding algorithm", font, 18);
                    spSub.setFillColor(Color(255, 150, 150));
                    spSub.setLetterSpacing(1.2f);
                    FloatRect spSB = spSub.getLocalBounds();
                    spSub.setPosition(960 - spSB.width / 2, 145);
                    window.draw(spSub);
                    
                    
                    RectangleShape dijkGlow(Vector2f(370, 420));
                    dijkGlow.setFillColor(Color(180, 30, 30, 50));
                    dijkGlow.setPosition(450, 190);
                    window.draw(dijkGlow);
                    
                    
                    RectangleShape dijkCard(Vector2f(350, 400));
                    dijkCard.setFillColor(Color(20, 8, 12, 250));
                    dijkCard.setOutlineThickness(4);
                    dijkCard.setPosition(460, 200);
                    
                    bool dijkHover = dijkCard.getGlobalBounds().contains(mousePos);
                    dijkCard.setOutlineColor(dijkHover ? Color(255, 50, 50) : Color(180, 40, 40));
                    if (dijkHover) dijkCard.setFillColor(Color(40, 15, 20, 255));
                    window.draw(dijkCard);
                    
                    
                    RectangleShape dijkInner(Vector2f(340, 390));
                    dijkInner.setFillColor(Color::Transparent);
                    dijkInner.setOutlineThickness(1);
                    dijkInner.setOutlineColor(dijkHover ? Color(255, 80, 80, 120) : Color(120, 40, 40, 80));
                    dijkInner.setPosition(465, 205);
                    window.draw(dijkInner);
                    
                    
                    CircleShape dijkIconGlow(50);
                    dijkIconGlow.setFillColor(Color(200, 40, 40, 60));
                    dijkIconGlow.setPosition(585, 230);
                    window.draw(dijkIconGlow);
                    
                    CircleShape dijkIcon(40);
                    dijkIcon.setFillColor(Color(180, 30, 30));
                    dijkIcon.setOutlineThickness(4);
                    dijkIcon.setOutlineColor(dijkHover ? Color(255, 80, 80) : Color(220, 60, 60));
                    dijkIcon.setPosition(595, 240);
                    window.draw(dijkIcon);
                    
                    Text dijkLabel("DIJKSTRA", font, 34);
                    dijkLabel.setStyle(Text::Bold);
                    dijkLabel.setFillColor(dijkHover ? Color(255, 80, 80) : Color(255, 100, 100));
                    dijkLabel.setLetterSpacing(1.5f);
                    FloatRect dLB = dijkLabel.getLocalBounds();
                    dijkLabel.setPosition(635 - dLB.width / 2, 340);
                    window.draw(dijkLabel);
                    
                    Text dijkDesc("Find optimal path by\ncost or travel time.\nGuaranteed optimal for\nweighted graphs.", font, 15);
                    dijkDesc.setFillColor(Color(255, 160, 160));
                    dijkDesc.setLineSpacing(1.5f);
                    dijkDesc.setPosition(490, 400);
                    window.draw(dijkDesc);
                    
                    Text dijkClick("[ CLICK TO SELECT ]", font, 14);
                    dijkClick.setFillColor(dijkHover ? Color(255, 80, 80) : Color(150, 60, 60, 200));
                    dijkClick.setStyle(dijkHover ? Text::Bold : Text::Regular);
                    FloatRect dcB = dijkClick.getLocalBounds();
                    dijkClick.setPosition(635 - dcB.width / 2, 550);
                    window.draw(dijkClick);
                    
                    
                    RectangleShape astarGlow(Vector2f(370, 420));
                    astarGlow.setFillColor(Color(200, 50, 30, 50));
                    astarGlow.setPosition(1100, 190);
                    window.draw(astarGlow);
                    
                    
                    RectangleShape astarCard(Vector2f(350, 400));
                    astarCard.setFillColor(Color(20, 10, 12, 250));
                    astarCard.setOutlineThickness(4);
                    astarCard.setPosition(1110, 200);
                    
                    bool astarHover = astarCard.getGlobalBounds().contains(mousePos);
                    astarCard.setOutlineColor(astarHover ? Color(255, 100, 50) : Color(200, 80, 60));
                    if (astarHover) astarCard.setFillColor(Color(45, 20, 22, 255));
                    window.draw(astarCard);
                    
                    
                    RectangleShape astarInner(Vector2f(340, 390));
                    astarInner.setFillColor(Color::Transparent);
                    astarInner.setOutlineThickness(1);
                    astarInner.setOutlineColor(astarHover ? Color(255, 120, 80, 120) : Color(150, 60, 40, 80));
                    astarInner.setPosition(1115, 205);
                    window.draw(astarInner);
                    
                    
                    CircleShape astarIconGlow(50, 5);
                    astarIconGlow.setFillColor(Color(255, 80, 40, 60));
                    astarIconGlow.setPosition(1235, 230);
                    window.draw(astarIconGlow);
                    
                    CircleShape astarIcon(40, 5);
                    astarIcon.setFillColor(Color(220, 80, 50));
                    astarIcon.setOutlineThickness(4);
                    astarIcon.setOutlineColor(astarHover ? Color(255, 150, 100) : Color(255, 120, 80));
                    astarIcon.setPosition(1245, 240);
                    window.draw(astarIcon);
                    
                    Text astarLabel("A* STAR", font, 34);
                    astarLabel.setStyle(Text::Bold);
                    astarLabel.setFillColor(astarHover ? Color(255, 120, 80) : Color(255, 140, 100));
                    astarLabel.setLetterSpacing(1.5f);
                    FloatRect aLB = astarLabel.getLocalBounds();
                    astarLabel.setPosition(1285 - aLB.width / 2, 340);
                    window.draw(astarLabel);
                    
                    Text astarDesc("Heuristic-based search.\nCan optimize by cost or\ntime. Uses intelligent\nestimation for speed.", font, 15);
                    astarDesc.setFillColor(Color(255, 180, 160));
                    astarDesc.setLineSpacing(1.5f);
                    astarDesc.setPosition(1140, 400);
                    window.draw(astarDesc);
                    
                    Text astarClick("[ CLICK TO SELECT ]", font, 14);
                    astarClick.setFillColor(astarHover ? Color(255, 100, 60) : Color(180, 80, 60, 200));
                    astarClick.setStyle(astarHover ? Text::Bold : Text::Regular);
                    FloatRect acB = astarClick.getLocalBounds();
                    astarClick.setPosition(1285 - acB.width / 2, 550);
                    window.draw(astarClick);
                    
                    
                    RectangleShape spBackBtnGlow(Vector2f(170, 55));
                    spBackBtnGlow.setFillColor(Color(150, 30, 30, 40));
                    spBackBtnGlow.setPosition(875, 645);
                    window.draw(spBackBtnGlow);
                    
                    RectangleShape spBackBtn(Vector2f(150, 45));
                    spBackBtn.setFillColor(Color(30, 12, 15, 250));
                    spBackBtn.setOutlineThickness(3);
                    spBackBtn.setOutlineColor(Color(150, 50, 50));
                    spBackBtn.setPosition(885, 650);
                    
                    bool backHover = spBackBtn.getGlobalBounds().contains(mousePos);
                    if (backHover) {
                        spBackBtn.setFillColor(Color(80, 30, 35, 255));
                        spBackBtn.setOutlineColor(Color(255, 80, 80));
                    }
                    window.draw(spBackBtn);
                    
                    Text backText("BACK", font, 18);
                    backText.setStyle(Text::Bold);
                    backText.setFillColor(backHover ? Color(255, 150, 150) : Color(200, 140, 140));
                    backText.setLetterSpacing(1.5f);
                    FloatRect bkB = backText.getLocalBounds();
                    backText.setPosition(960 - bkB.width / 2, 660);
                    window.draw(backText);
                    
                    
                    static bool spAlgoClicked = false;
                    if (Mouse::isButtonPressed(Mouse::Button::Left)) {
                        if (!spAlgoClicked) {
                            spAlgoClicked = true;
                            if (dijkHover) {
                                clickSound.play();
                                transitionSound.play();
                                selectedAlgorithm = 1;
                                showAlgorithmSelection = false;
                                showShortestPathInput = true;
                                spOrigin = "";
                                spDestination = "";
                                spDate = "";
                                spOriginActive = true;
                                spDestActive = false;
                                spDateActive = false;
                            }
                            else if (astarHover) {
                                clickSound.play();
                                transitionSound.play();
                                selectedAlgorithm = 2;
                                showAlgorithmSelection = false;
                                showShortestPathInput = true;
                                spOrigin = "";
                                spDestination = "";
                                spDate = "";
                                spOriginActive = true;
                                spDestActive = false;
                                spDateActive = false;
                            }
                            else if (backHover) {
                                clickSound.play();
                                transitionSound.play();
                                showShortestPathMode = false;
                            }
                        }
                    } else {
                        spAlgoClicked = false;
                    }
                }
                
                else if (showShortestPathInput)
                {
                    string algoName = (selectedAlgorithm == 1) ? "DIJKSTRA" : "A* STAR";
                    Color algoColor = (selectedAlgorithm == 1) ? Color(255, 60, 60) : Color(255, 120, 60);
                    Color algoGlow = (selectedAlgorithm == 1) ? Color(200, 30, 30, 80) : Color(200, 80, 30, 80);
                    
                    Text inputTitle(algoName + " PATHFINDING", font, 40);
                    inputTitle.setStyle(Text::Bold);
                    inputTitle.setFillColor(algoColor);
                    inputTitle.setOutlineColor(Color(60, 0, 0));
                    inputTitle.setOutlineThickness(3);
                    inputTitle.setLetterSpacing(1.5f);
                    FloatRect itB = inputTitle.getLocalBounds();
                    inputTitle.setPosition(960 - itB.width / 2, 140);
                    window.draw(inputTitle);
                    
                    
                    RectangleShape inputTitleLine(Vector2f(itB.width + 40, 3));
                    inputTitleLine.setFillColor(Color(algoColor.r, algoColor.g, algoColor.b, 150));
                    inputTitleLine.setPosition(960 - itB.width / 2 - 20, 195);
                    window.draw(inputTitleLine);
                    
                    
                    RectangleShape inputPanelGlow(Vector2f(520, 470));
                    inputPanelGlow.setFillColor(algoGlow);
                    inputPanelGlow.setPosition(700, 210);
                    window.draw(inputPanelGlow);
                    
                    
                    RectangleShape inputPanel(Vector2f(500, 450));
                    inputPanel.setFillColor(Color(15, 8, 10, 250));
                    inputPanel.setOutlineThickness(4);
                    inputPanel.setOutlineColor(algoColor);
                    inputPanel.setPosition(710, 220);
                    window.draw(inputPanel);
                    
                    
                    RectangleShape inputPanelInner(Vector2f(490, 440));
                    inputPanelInner.setFillColor(Color::Transparent);
                    inputPanelInner.setOutlineThickness(1);
                    inputPanelInner.setOutlineColor(Color(algoColor.r, algoColor.g, algoColor.b, 80));
                    inputPanelInner.setPosition(715, 225);
                    window.draw(inputPanelInner);
                    
                    
                    Text originLabel("DEPARTURE PORT", font, 14);
                    originLabel.setFillColor(Color(255, 160, 140));
                    originLabel.setLetterSpacing(1.3f);
                    originLabel.setPosition(740, 255);
                    window.draw(originLabel);
                    
                    RectangleShape originBox(Vector2f(440, 45));
                    originBox.setFillColor(Color(25, 12, 15, 255));
                    originBox.setOutlineThickness(spOriginActive ? 3 : 2);
                    originBox.setOutlineColor(spOriginActive ? algoColor : Color(100, 50, 50));
                    originBox.setPosition(740, 280);
                    window.draw(originBox);
                    
                    Text originText(spOrigin + (spOriginActive ? "|" : ""), font, 18);
                    originText.setFillColor(Color(255, 220, 220));
                    originText.setPosition(752, 290);
                    window.draw(originText);
                    
                    
                    Text destLabel("DESTINATION PORT", font, 14);
                    destLabel.setFillColor(Color(255, 160, 140));
                    destLabel.setLetterSpacing(1.3f);
                    destLabel.setPosition(740, 345);
                    window.draw(destLabel);
                    
                    RectangleShape destBox(Vector2f(440, 45));
                    destBox.setFillColor(Color(25, 12, 15, 255));
                    destBox.setOutlineThickness(spDestActive ? 3 : 2);
                    destBox.setOutlineColor(spDestActive ? algoColor : Color(100, 50, 50));
                    destBox.setPosition(740, 370);
                    window.draw(destBox);
                    
                    Text destText(spDestination + (spDestActive ? "|" : ""), font, 18);
                    destText.setFillColor(Color(255, 220, 220));
                    destText.setPosition(752, 380);
                    window.draw(destText);
                    
                    
                    Text dateLabel("DATE (REQUIRED)", font, 14);
                    dateLabel.setFillColor(Color(255, 160, 140));
                    dateLabel.setLetterSpacing(1.3f);
                    dateLabel.setPosition(740, 435);
                    window.draw(dateLabel);
                    
                    Text dateFormat("D/M/YYYY", font, 12);
                    dateFormat.setFillColor(Color(180, 100, 100));
                    dateFormat.setPosition(880, 438);
                    window.draw(dateFormat);
                    
                    Text dateRequired("*", font, 24);
                    dateRequired.setFillColor(Color(255, 50, 50));
                    dateRequired.setStyle(Text::Bold);
                    dateRequired.setPosition(860, 432);
                    window.draw(dateRequired);
                    
                    RectangleShape dateBox(Vector2f(440, 45));
                    dateBox.setFillColor(Color(25, 12, 15, 255));
                    dateBox.setOutlineThickness(spDateActive ? 3 : 2);
                    dateBox.setOutlineColor(spDateActive ? algoColor : Color(100, 50, 50));
                    dateBox.setPosition(740, 460);
                    window.draw(dateBox);
                    
                    Text dateText(spDate + (spDateActive ? "|" : ""), font, 18);
                    dateText.setFillColor(Color(255, 220, 220));
                    dateText.setPosition(752, 470);
                    window.draw(dateText);
                    
                    
                    Text modeLabel("OPTIMIZE BY:", font, 13);
                    modeLabel.setFillColor(Color(180, 130, 130));
                    modeLabel.setLetterSpacing(1.2f);
                    modeLabel.setPosition(740, 525);
                    window.draw(modeLabel);
                    
                    Color toggleColorActive = Color(180, 50, 50);
                    Color toggleColorInactive = Color(40, 20, 25);
                    
                    RectangleShape costBtn(Vector2f(100, 38));
                    costBtn.setFillColor(useCostMode ? toggleColorActive : toggleColorInactive);
                    costBtn.setOutlineThickness(2);
                    costBtn.setOutlineColor(useCostMode ? Color(255, 80, 80) : Color(80, 40, 40));
                    costBtn.setPosition(860, 518);
                    window.draw(costBtn);
                    
                    Text costText("COST", font, 14);
                    costText.setFillColor(useCostMode ? Color(255, 220, 220) : Color(120, 80, 80));
                    costText.setStyle(Text::Bold);
                    costText.setLetterSpacing(1.2f);
                    costText.setPosition(883, 527);
                    window.draw(costText);
                    
                    RectangleShape timeBtn(Vector2f(100, 38));
                    timeBtn.setFillColor(!useCostMode ? toggleColorActive : toggleColorInactive);
                    timeBtn.setOutlineThickness(2);
                    timeBtn.setOutlineColor(!useCostMode ? Color(255, 80, 80) : Color(80, 40, 40));
                    timeBtn.setPosition(970, 518);
                    window.draw(timeBtn);
                    
                    Text timeText("TIME", font, 14);
                    timeText.setFillColor(!useCostMode ? Color(255, 220, 220) : Color(120, 80, 80));
                    timeText.setStyle(Text::Bold);
                    timeText.setLetterSpacing(1.2f);
                    timeText.setPosition(996, 527);
                    window.draw(timeText);
                    
                    
                    RectangleShape searchBtnGlow(Vector2f(195, 60));
                    searchBtnGlow.setFillColor(Color(200, 40, 40, 50));
                    searchBtnGlow.setPosition(755, 575);
                    window.draw(searchBtnGlow);
                    
                    RectangleShape searchBtn(Vector2f(180, 50));
                    searchBtn.setFillColor(Color(160, 30, 30, 250));
                    searchBtn.setOutlineThickness(3);
                    searchBtn.setOutlineColor(Color(255, 60, 60));
                    searchBtn.setPosition(762, 580);
                    
                    bool searchHover = searchBtn.getGlobalBounds().contains(mousePos);
                    if (searchHover) {
                        searchBtn.setFillColor(Color(200, 50, 50, 255));
                        searchBtn.setOutlineColor(Color(255, 100, 100));
                    }
                    window.draw(searchBtn);
                    
                    Text searchText("FIND PATH", font, 18);
                    searchText.setStyle(Text::Bold);
                    searchText.setFillColor(Color(255, 200, 200));
                    searchText.setLetterSpacing(1.3f);
                    searchText.setPosition(795, 594);
                    window.draw(searchText);
                    
                    
                    RectangleShape cancelBtnGlow(Vector2f(135, 60));
                    cancelBtnGlow.setFillColor(Color(100, 30, 30, 30));
                    cancelBtnGlow.setPosition(955, 575);
                    window.draw(cancelBtnGlow);
                    
                    RectangleShape cancelBtn(Vector2f(120, 50));
                    cancelBtn.setFillColor(Color(35, 15, 18, 250));
                    cancelBtn.setOutlineThickness(3);
                    cancelBtn.setOutlineColor(Color(120, 50, 50));
                    cancelBtn.setPosition(962, 580);
                    
                    bool cancelHover = cancelBtn.getGlobalBounds().contains(mousePos);
                    if (cancelHover) {
                        cancelBtn.setFillColor(Color(80, 35, 40, 255));
                        cancelBtn.setOutlineColor(Color(200, 80, 80));
                    }
                    window.draw(cancelBtn);
                    
                    Text cancelText("BACK", font, 16);
                    cancelText.setStyle(Text::Bold);
                    cancelText.setFillColor(cancelHover ? Color(255, 160, 160) : Color(180, 120, 120));
                    cancelText.setLetterSpacing(1.3f);
                    cancelText.setPosition(990, 596);
                    window.draw(cancelText);
                    
                    
                    if (spPathFound || !spResultText.empty()) {
                        RectangleShape resultBoxGlow(Vector2f(515, 90));
                        resultBoxGlow.setFillColor(Color(spResultColor.r, spResultColor.g, spResultColor.b, 40));
                        resultBoxGlow.setPosition(702, 675);
                        window.draw(resultBoxGlow);
                        
                        RectangleShape resultBox(Vector2f(500, 80));
                        resultBox.setFillColor(Color(20, 10, 12, 250));
                        resultBox.setOutlineThickness(3);
                        resultBox.setOutlineColor(spResultColor);
                        resultBox.setPosition(710, 680);
                        window.draw(resultBox);
                        
                        Text resultText(spResultText, font, 16);
                        resultText.setFillColor(spResultColor);
                        resultText.setPosition(730, 700);
                        window.draw(resultText);
                    }
                    
                    
                    static bool spInputClicked = false;
                    if (Mouse::isButtonPressed(Mouse::Button::Left)) {
                        if (!spInputClicked) {
                            spInputClicked = true;
                            if (originBox.getGlobalBounds().contains(mousePos)) {
                                spOriginActive = true; spDestActive = false; spDateActive = false;
                            }
                            else if (destBox.getGlobalBounds().contains(mousePos)) {
                                spOriginActive = false; spDestActive = true; spDateActive = false;
                            }
                            else if (dateBox.getGlobalBounds().contains(mousePos)) {
                                spOriginActive = false; spDestActive = false; spDateActive = true;
                            }
                            
                            
                            RectangleShape costBtnArea(Vector2f(100, 35));
                            costBtnArea.setPosition(860, 515);
                            RectangleShape timeBtnArea(Vector2f(100, 35));
                            timeBtnArea.setPosition(970, 515);
                            if (costBtnArea.getGlobalBounds().contains(mousePos)) useCostMode = true;
                            if (timeBtnArea.getGlobalBounds().contains(mousePos)) useCostMode = false;
                        }
                    } else {
                        spInputClicked = false;
                    }
                    
                    
                    static bool spBtnClicked = false;
                    if (Mouse::isButtonPressed(Mouse::Button::Left)) {
                        if (!spBtnClicked) {
                            if (searchHover && !spOrigin.empty() && !spDestination.empty()) {
                                clickSound.play();
                                
                                if (spDate.empty()) {
                                    spPathFound = false;
                                    spResultText = "Date is required! Please enter a valid date.";
                                    spResultColor = Color(255, 100, 100);
                                    spBtnClicked = true;
                                }
                                else {
                                
                                activeAnimations.clear();
                                shortestPath.clear();
                                spWaitPorts.clear();
                                spWaitDurations.clear();
                                
                                
                                int originIdx = graph.findPortIndex(spOrigin);
                                int destIdx = graph.findPortIndex(spDestination);
                                string searchDate = graph.normalizeDate(spDate);
                                
                                if (originIdx == -1 || destIdx == -1) {
                                    spPathFound = false;
                                    spResultText = "Invalid port name(s). Check spelling.";
                                    spResultColor = Color(255, 100, 100);
                                } else {
                                    bool found = false;
                                    Vector<RouteNode*> spPathRoutes;
                                    Vector<int> spWaitTimes;
                                    
                                    if (selectedAlgorithm == 1) {
                                        
                                        DijkstraAlgorithm dijkstra;
                                        if (useCostMode) {
                                            shortestPath = dijkstra.findCheapestPath(graph.ports, originIdx, destIdx, searchDate, &shipPrefs);
                                        } else {
                                            shortestPath = dijkstra.findFastestPath(graph.ports, originIdx, destIdx, searchDate, &shipPrefs);
                                        }
                                        found = (shortestPath.getSize() > 0);
                                        
                                        
                                        if (found) {
                                            spTotalCost = 0;
                                            spTotalTime = 0;
                                            string currentDate = searchDate;
                                            string currentTime = "00:00";
                                            
                                            for (int i = 0; i < shortestPath.getSize() - 1; i++) {
                                                RouteNode* route = graph.ports[shortestPath[i]].routeHead;
                                                RouteNode* bestRoute = nullptr;
                                                int bestScore = INT_MAX; 
                                                
                                                
                                                while (route) {
                                                    if (route->destinationPort == graph.ports[shortestPath[i+1]].portName) {
                                                        
                                                        if (graph.isDateGreaterOrEqual(route->departureDate, currentDate)) {
                                                            
                                                            bool canTake = false;
                                                            if (route->departureDate == currentDate) {
                                                                int arrMin = dijkstra.toMinutes(currentTime);
                                                                int depMin = dijkstra.toMinutes(route->departureTime);
                                                                if (depMin >= arrMin + 120) canTake = true;
                                                            } else {
                                                                canTake = true;
                                                            }
                                                            
                                                            if (canTake) {
                                                                int score;
                                                                if (useCostMode) {
                                                                    score = route->cost; 
                                                                } else {
                                                                    
                                                                    int waitDays = dijkstra.calculateDaysBetween(currentDate, route->departureDate);
                                                                    int waitMins = waitDays * 24 * 60;
                                                                    int travelMins = dijkstra.calculateTravelTime(route->departureTime, route->arrivalTime);
                                                                    score = waitMins + travelMins;
                                                                }
                                                                
                                                                if (score < bestScore) {
                                                                    bestScore = score;
                                                                    bestRoute = route;
                                                                }
                                                            }
                                                        }
                                                    }
                                                    route = route->next;
                                                }
                                                
                                                if (bestRoute) {
                                                    spTotalCost += bestRoute->cost;
                                                    int segTime = dijkstra.calculateTravelTime(bestRoute->departureTime, bestRoute->arrivalTime);
                                                    spTotalTime += segTime;
                                                    spPathRoutes.push_back(bestRoute);
                                                    spWaitTimes.push_back(0);
                                                    
                                                    currentDate = bestRoute->departureDate;
                                                    currentTime = bestRoute->arrivalTime;
                                                    
                                                    int depMin = dijkstra.toMinutes(bestRoute->departureTime);
                                                    int arrMin = dijkstra.toMinutes(bestRoute->arrivalTime);
                                                    if (arrMin < depMin) {
                                                        currentDate = dijkstra.addDaysToDate(currentDate, 1);
                                                    }
                                                }
                                            }
                                        }
                                    } else {
                                        
                                        AStar astar;
                                        if (useCostMode) {
                                            shortestPath = astar.findAStarPathCost(graph.ports, originIdx, destIdx, searchDate, &shipPrefs);
                                        } else {
                                            shortestPath = astar.findAStarPathTime(graph.ports, originIdx, destIdx, searchDate, &shipPrefs);
                                        }
                                        found = (shortestPath.getSize() > 0);
                                        
                                        
                                        if (found) {
                                            spTotalCost = 0;
                                            spTotalTime = 0;
                                            DijkstraAlgorithm dijkstra;
                                            string currentDate = searchDate;
                                            string currentTime = "00:00";
                                            
                                            for (int i = 0; i < shortestPath.getSize() - 1; i++) {
                                                RouteNode* route = graph.ports[shortestPath[i]].routeHead;
                                                RouteNode* bestRoute = nullptr;
                                                int bestScore = INT_MAX;
                                                
                                                
                                                while (route) {
                                                    if (route->destinationPort == graph.ports[shortestPath[i+1]].portName) {
                                                        if (graph.isDateGreaterOrEqual(route->departureDate, currentDate)) {
                                                            bool canTake = false;
                                                            if (route->departureDate == currentDate) {
                                                                int arrMin = dijkstra.toMinutes(currentTime);
                                                                int depMin = dijkstra.toMinutes(route->departureTime);
                                                                if (depMin >= arrMin + 120) canTake = true;
                                                            } else {
                                                                canTake = true;
                                                            }
                                                            
                                                            if (canTake) {
                                                                int score;
                                                                if (useCostMode) {
                                                                    score = route->cost;
                                                                } else {
                                                                    int waitDays = dijkstra.calculateDaysBetween(currentDate, route->departureDate);
                                                                    int waitMins = waitDays * 24 * 60;
                                                                    int travelMins = dijkstra.calculateTravelTime(route->departureTime, route->arrivalTime);
                                                                    score = waitMins + travelMins;
                                                                }
                                                                
                                                                if (score < bestScore) {
                                                                    bestScore = score;
                                                                    bestRoute = route;
                                                                }
                                                            }
                                                        }
                                                    }
                                                    route = route->next;
                                                }
                                                
                                                if (bestRoute) {
                                                    spTotalCost += bestRoute->cost;
                                                    int segTime = dijkstra.calculateTravelTime(bestRoute->departureTime, bestRoute->arrivalTime);
                                                    spTotalTime += segTime;
                                                    spPathRoutes.push_back(bestRoute);
                                                    spWaitTimes.push_back(0);
                                                    currentDate = bestRoute->departureDate;
                                                    currentTime = bestRoute->arrivalTime;
                                                    int depMin = dijkstra.toMinutes(bestRoute->departureTime);
                                                    int arrMin = dijkstra.toMinutes(bestRoute->arrivalTime);
                                                    if (arrMin < depMin) {
                                                        currentDate = dijkstra.addDaysToDate(currentDate, 1);
                                                    }
                                                }
                                            }
                                        }
                                    }
                                    
                                    if (found && shortestPath.getSize() > 0) {
                                        spPathFound = true;
                                        spResultText = "Path found! Cost: $" + to_string(spTotalCost) + 
                                                       " | Time: " + graph.formatTime(spTotalTime);
                                        spResultColor = algoColor;
                                        
                                        
                                        RouteAnimation anim;
                                        anim.path = shortestPath;
                                        anim.pathRoutes = spPathRoutes;
                                        anim.waitTimes = spWaitTimes;
                                        anim.waitPorts = spWaitPorts;
                                        anim.waitDurations = spWaitDurations;
                                        anim.routeColor = algoColor;
                                        anim.routeType = (selectedAlgorithm == 1) ? 
                                            ("Dijkstra " + string(useCostMode ? "Cost" : "Time")) : 
                                            ("A* " + string(useCostMode ? "Cost" : "Time"));
                                        anim.currentSegment = 0;
                                        anim.segmentProgress = 0.0f;
                                        anim.isPaused = false;
                                        anim.pauseTimer = 0.0f;
                                        anim.isComplete = false;
                                        anim.totalCost = spTotalCost;
                                        anim.totalTime = spTotalTime;
                                        activeAnimations.push_back(anim);
                                        
                                        showShortestPathInput = false;
                                        showShortestPathMode = false;
                                    } else {
                                        spPathFound = false;
                                        spResultText = "No path found between " + spOrigin + " and " + spDestination;
                                        spResultColor = Color(255, 100, 100);
                                    }
                                }
                                } 
                            }
                            else if (cancelHover) {
                                clickSound.play();
                                showShortestPathInput = false;
                                showAlgorithmSelection = true;
                                spPathFound = false;
                                spResultText = "";
                            }
                            spBtnClicked = true;
                        }
                    } else {
                        spBtnClicked = false;
                    }
                }
            }

            Text legend;
            legend.setFont(font);
            legend.setString("Click on any port to see all routes from it\nClick Search Route to find specific paths\nDirect routes show all available dates\nConnecting routes require specific date\nMultiple routes show in different colors");
            legend.setCharacterSize(14);
            legend.setFillColor(Color(180, 220, 255));
            legend.setPosition(20, 830);
            window.draw(legend);

            
            
            if (shipPrefs.filterActive)
            {
                
                int totalPorts = graph.ports.getSize();
                int avoidedCount = 0;
                for (int i = 0; i < totalPorts; i++)
                {
                    if (shipPrefs.isPortAvoided(graph.ports[i].portName))
                    {
                        avoidedCount++;
                    }
                }
                int activeCount = totalPorts - avoidedCount;

                
                RectangleShape subgraphPanel(Vector2f(280, 110));
                subgraphPanel.setFillColor(Color(40, 20, 30, 230));
                subgraphPanel.setOutlineThickness(3);
                subgraphPanel.setOutlineColor(Color(255, 80, 80));
                subgraphPanel.setPosition(20, 700);
                window.draw(subgraphPanel);

                
                float blink = (sin(time * 4.0f) + 1.0f) / 2.0f;
                Text subgraphTitle("!! SUBGRAPH ACTIVE !!", font, 16);
                subgraphTitle.setFillColor(Color(255, static_cast<Uint8>(100 + 155 * blink), static_cast<Uint8>(100 + 155 * blink)));
                subgraphTitle.setStyle(Text::Bold);
                subgraphTitle.setPosition(40, 708);
                window.draw(subgraphTitle);

                
                Text activeText("Active Ports: " + to_string(activeCount), font, 15);
                activeText.setFillColor(Color(100, 255, 100));
                activeText.setPosition(40, 738);
                window.draw(activeText);

                
                Text avoidedText("Avoided Ports: " + to_string(avoidedCount), font, 15);
                avoidedText.setFillColor(Color(255, 100, 100));
                avoidedText.setPosition(40, 763);
                window.draw(avoidedText);

                
                Text instrText("Avoided ports show big red X", font, 12);
                instrText.setFillColor(Color(200, 200, 200));
                instrText.setPosition(40, 790);
                window.draw(instrText);
            }

            window.display();
        }
    }
    bool findAllDirectRoutes(string origin, string destination,
                             Vector<Vector<int>> &allPaths,
                             Vector<Vector<string>> &allDates)
    {
        allPaths.clear();
        allDates.clear();

        
        int originIndex = findPortIndex(origin);
        int destIndex = findPortIndex(destination);

        if (originIndex == -1 || destIndex == -1)
        {
            return false;
        }

        
        RouteNode *currentRoute = ports[originIndex].routeHead;
        while (currentRoute != nullptr)
        {
            if (currentRoute->destinationPort == destination)
            {
                
                bool passesFilter = true;
                if (shipPrefs.filterActive)
                {
                    
                    if (!shipPrefs.isCompanyPreferred(currentRoute->shippingCompany))
                        passesFilter = false;
                    
                    
                    if (shipPrefs.isPortAvoided(destination))
                        passesFilter = false;
                }
                
                if (passesFilter)
                {
                    
                    Vector<int> path;
                    path.push_back(originIndex);
                    path.push_back(destIndex);
                    allPaths.push_back(path);

                    
                    Vector<string> dates;
                    dates.push_back(currentRoute->departureDate);
                    allDates.push_back(dates);
                }
            }
            currentRoute = currentRoute->next;
        }

        return !allPaths.empty();
    }
};



void openShipPreferencesPopup(Font &font)
{
    RenderWindow prefWindow(VideoMode(1400, 900), "Ship Preferences - Filter Routes", Style::Close);
    prefWindow.setFramerateLimit(60);

    
    Vector<string> companies;
    companies.push_back("MaerskLine");
    companies.push_back("MSC");
    companies.push_back("COSCO");
    companies.push_back("CMA_CGM");
    companies.push_back("Evergreen");
    companies.push_back("HapagLloyd");
    companies.push_back("ONE");
    companies.push_back("YangMing");
    companies.push_back("ZIM");
    companies.push_back("PIL");

    
    Color companyColors[] = {
        Color(0, 91, 187),  
        Color(255, 204, 0), 
        Color(0, 51, 102),  
        Color(0, 114, 198), 
        Color(0, 128, 0),   
        Color(255, 102, 0), 
        Color(255, 0, 127), 
        Color(255, 165, 0), 
        Color(0, 102, 153), 
        Color(153, 0, 0)    
    };

    
    Vector<string> allPorts;
    for (int i = 0; i < numLocations; i++)
    {
        allPorts.push_back(portLocations[i].name);
    }

    
    int portScrollOffset = 0;
    const int maxVisiblePorts = 10;
    const int portItemHeight = 48;

    
    string maxTimeInput = "";
    if (shipPrefs.maxVoyageMinutes > 0)
        maxTimeInput = to_string(shipPrefs.maxVoyageMinutes / 60);
    bool maxTimeActive = false;

    Clock animClock;

    
    struct Particle
    {
        float x, y, speed, size;
        Color color;
    };
    Vector<Particle> particles;
    for (int i = 0; i < 60; i++)
    {
        Particle p;
        p.x = rand() % 1400;
        p.y = rand() % 900;
        p.speed = 0.2f + (rand() % 100) / 200.0f;
        p.size = 1 + rand() % 3;
        p.color = Color(100 + rand() % 100, 150 + rand() % 100, 255, 80 + rand() % 80);
        particles.push_back(p);
    }

    while (prefWindow.isOpen())
    {
        float time = animClock.getElapsedTime().asSeconds();
        Vector2i mousePos = Mouse::getPosition(prefWindow);
        Vector2f mouse(mousePos.x, mousePos.y);

        Event e;
        while (prefWindow.pollEvent(e))
        {
            if (e.type == Event::Closed)
                prefWindow.close();

            if (e.type == Event::KeyPressed)
            {
                if (e.key.code == Keyboard::Escape)
                    prefWindow.close();
            }

            
            if (e.type == Event::MouseWheelScrolled)
            {
                if (mouse.x >= 460 && mouse.x <= 860 && mouse.y >= 140 && mouse.y <= 820)
                {
                    portScrollOffset -= (int)(e.mouseWheelScroll.delta * 2);
                    int maxScroll = max(0, allPorts.getSize() - maxVisiblePorts);
                    if (portScrollOffset < 0) portScrollOffset = 0;
                    if (portScrollOffset > maxScroll) portScrollOffset = maxScroll;
                }
            }

            
            if (e.type == Event::TextEntered && maxTimeActive)
            {
                if (e.text.unicode == '\b' && maxTimeInput.length() > 0)
                {
                    maxTimeInput.pop_back();
                }
                else if (e.text.unicode >= '0' && e.text.unicode <= '9' && maxTimeInput.length() < 5)
                {
                    maxTimeInput += static_cast<char>(e.text.unicode);
                }
                if (!maxTimeInput.empty())
                    shipPrefs.maxVoyageMinutes = stoi(maxTimeInput) * 60;
                else
                    shipPrefs.maxVoyageMinutes = 0;
            }

            
            if (e.type == Event::MouseButtonPressed && e.mouseButton.button == Mouse::Button::Left)
            {
                
                for (int c = 0; c < companies.getSize(); c++)
                {
                    FloatRect companyBounds(50, 180 + c * 55, 350, 50);
                    if (companyBounds.contains(mouse))
                    {
                        string company = companies[c];
                        bool isSelected = false;
                        for (int i = 0; i < shipPrefs.preferredCompanies.getSize(); i++)
                        {
                            if (shipPrefs.preferredCompanies[i] == company)
                            {
                                isSelected = true;
                                break;
                            }
                        }

                        if (isSelected)
                        {
                            shipPrefs.removePreferredCompany(company);
                            if (shipPrefs.preferredCompanies.getSize() == 0)
                                shipPrefs.filterActive = false;
                        }
                        else
                        {
                            shipPrefs.addPreferredCompany(company);
                            shipPrefs.filterActive = true;
                        }
                        break;
                    }
                }

                
                if (mouse.x >= 460 && mouse.x <= 860 && mouse.y >= 195 && mouse.y <= 680)
                {
                    for (int p = 0; p < maxVisiblePorts && (p + portScrollOffset) < allPorts.getSize(); p++)
                    {
                        int actualIndex = p + portScrollOffset;
                        FloatRect portBounds(480, 195 + p * portItemHeight, 350, portItemHeight - 3);
                        if (portBounds.contains(mouse))
                        {
                            string port = allPorts[actualIndex];
                            bool isAvoided = false;
                            for (int i = 0; i < shipPrefs.avoidedPorts.getSize(); i++)
                            {
                                if (shipPrefs.avoidedPorts[i] == port)
                                {
                                    isAvoided = true;
                                    break;
                                }
                            }

                            if (isAvoided)
                            {
                                shipPrefs.removeAvoidedPort(port);
                            }
                            else
                            {
                                shipPrefs.addAvoidedPort(port);
                                shipPrefs.filterActive = true;
                            }
                            break;
                        }
                    }
                }

                
                FloatRect timeInputBounds(950, 250, 200, 50);
                maxTimeActive = timeInputBounds.contains(mouse);

                
                FloatRect clearBounds(950, 700, 180, 50);
                if (clearBounds.contains(mouse))
                {
                    clickSound.play();
                    while (shipPrefs.preferredCompanies.getSize() > 0)
                        shipPrefs.preferredCompanies.pop();
                    while (shipPrefs.avoidedPorts.getSize() > 0)
                        shipPrefs.avoidedPorts.pop();
                    shipPrefs.maxVoyageMinutes = 0;
                    shipPrefs.filterActive = false;
                    maxTimeInput = "";
                }

                
                FloatRect applyBounds(1150, 700, 180, 50);
                if (applyBounds.contains(mouse))
                {
                    clickSound.play();
                    prefWindow.close();
                }
            }
        }

        
        RectangleShape bgTop(Vector2f(1400, 450));
        bgTop.setFillColor(Color(8, 20, 45));
        bgTop.setPosition(0, 0);
        prefWindow.draw(bgTop);

        RectangleShape bgBottom(Vector2f(1400, 450));
        bgBottom.setFillColor(Color(15, 35, 70));
        bgBottom.setPosition(0, 450);
        prefWindow.draw(bgBottom);

        
        for (int i = 0; i < particles.getSize(); i++)
        {
            particles[i].y -= particles[i].speed;
            if (particles[i].y < -10)
            {
                particles[i].y = 910;
                particles[i].x = rand() % 1400;
            }

            float twinkle = 0.5f + 0.5f * sin(time * 3.0f + i);
            CircleShape star(particles[i].size);
            star.setFillColor(Color(
                particles[i].color.r,
                particles[i].color.g,
                particles[i].color.b,
                (Uint8)(particles[i].color.a * twinkle)));
            star.setPosition(particles[i].x, particles[i].y);
            prefWindow.draw(star);
        }

        
        for (int w = 0; w < 3; w++)
        {
            for (int i = 0; i < 30; i++)
            {
                float waveY = 850 + w * 15 + sin(time * 1.5f + i * 0.3f + w) * 10;
                CircleShape wave(5 - w);
                wave.setFillColor(Color(30 + w * 15, 80 + w * 20, 150 + w * 25, 80 - w * 20));
                wave.setPosition(i * 50 + fmod(time * 15, 50.0f), waveY);
                prefWindow.draw(wave);
            }
        }

        
        RectangleShape headerBg(Vector2f(1400, 80));
        headerBg.setFillColor(Color(10, 30, 60, 230));
        headerBg.setPosition(0, 0);
        prefWindow.draw(headerBg);

        Text title("SHIP PREFERENCES", font, 36);
        title.setStyle(Text::Bold);
        title.setFillColor(Color(100, 200, 255));
        title.setLetterSpacing(3);
        FloatRect titleBounds = title.getLocalBounds();
        title.setPosition(700 - titleBounds.width / 2, 15);
        prefWindow.draw(title);

        Text subtitle("Filter routes by company, avoid ports, set time limits", font, 16);
        subtitle.setFillColor(Color(150, 180, 220));
        FloatRect subBounds = subtitle.getLocalBounds();
        subtitle.setPosition(700 - subBounds.width / 2, 55);
        prefWindow.draw(subtitle);

        
        RectangleShape statusBox(Vector2f(200, 35));
        statusBox.setFillColor(shipPrefs.filterActive ? Color(0, 80, 50, 200) : Color(80, 40, 40, 200));
        statusBox.setOutlineThickness(2);
        statusBox.setOutlineColor(shipPrefs.filterActive ? Color(0, 255, 150) : Color(255, 100, 100));
        statusBox.setPosition(1180, 95);
        prefWindow.draw(statusBox);

        Text statusText(shipPrefs.filterActive ? "FILTER ACTIVE" : "NO FILTER", font, 14);
        statusText.setFillColor(shipPrefs.filterActive ? Color(100, 255, 150) : Color(255, 150, 150));
        statusText.setStyle(Text::Bold);
        statusText.setPosition(1210, 102);
        prefWindow.draw(statusText);

        
        RectangleShape companyPanel(Vector2f(380, 680));
        companyPanel.setFillColor(Color(15, 30, 55, 220));
        companyPanel.setOutlineThickness(2);
        companyPanel.setOutlineColor(Color(0, 150, 200, 150));
        companyPanel.setPosition(30, 140);
        prefWindow.draw(companyPanel);

        RectangleShape companyHeader(Vector2f(380, 45));
        companyHeader.setFillColor(Color(0, 80, 120, 200));
        companyHeader.setPosition(30, 140);
        prefWindow.draw(companyHeader);

        CircleShape shipIcon(15, 3);
        shipIcon.setFillColor(Color(100, 200, 255));
        shipIcon.setRotation(90);
        shipIcon.setPosition(70, 148);
        prefWindow.draw(shipIcon);

        Text companyTitle("PREFERRED COMPANIES", font, 16);
        companyTitle.setFillColor(Color(200, 240, 255));
        companyTitle.setStyle(Text::Bold);
        companyTitle.setPosition(95, 152);
        prefWindow.draw(companyTitle);

        
        for (int c = 0; c < companies.getSize(); c++)
        {
            bool isSelected = false;
            for (int i = 0; i < shipPrefs.preferredCompanies.getSize(); i++)
            {
                if (shipPrefs.preferredCompanies[i] == companies[c])
                {
                    isSelected = true;
                    break;
                }
            }

            FloatRect bounds(50, 195 + c * 55, 340, 48);
            bool isHovered = bounds.contains(mouse);

            RectangleShape companyCard(Vector2f(340, 48));
            if (isSelected)
            {
                companyCard.setFillColor(Color(companyColors[c].r / 3, companyColors[c].g / 3, companyColors[c].b / 3, 220));
                companyCard.setOutlineColor(companyColors[c]);
            }
            else if (isHovered)
            {
                companyCard.setFillColor(Color(40, 60, 90, 200));
                companyCard.setOutlineColor(Color(100, 150, 200));
            }
            else
            {
                companyCard.setFillColor(Color(25, 40, 65, 180));
                companyCard.setOutlineColor(Color(60, 80, 110));
            }
            companyCard.setOutlineThickness(2);
            companyCard.setPosition(50, 195 + c * 55);
            prefWindow.draw(companyCard);

            RectangleShape colorBar(Vector2f(8, 48));
            colorBar.setFillColor(companyColors[c]);
            colorBar.setPosition(50, 195 + c * 55);
            prefWindow.draw(colorBar);

            RectangleShape checkbox(Vector2f(24, 24));
            checkbox.setFillColor(isSelected ? companyColors[c] : Color(40, 50, 70));
            checkbox.setOutlineThickness(2);
            checkbox.setOutlineColor(isSelected ? Color::White : Color(100, 120, 150));
            checkbox.setPosition(70, 207 + c * 55);
            prefWindow.draw(checkbox);

            if (isSelected)
            {
                Text checkmark("v", font, 18);
                checkmark.setFillColor(Color::White);
                checkmark.setStyle(Text::Bold);
                checkmark.setPosition(75, 203 + c * 55);
                prefWindow.draw(checkmark);
            }

            Text companyName(companies[c], font, 16);
            companyName.setFillColor(isSelected ? Color::White : Color(180, 200, 220));
            companyName.setStyle(isSelected ? Text::Bold : Text::Regular);
            companyName.setPosition(110, 208 + c * 55);
            prefWindow.draw(companyName);
        }

        
        RectangleShape portPanel(Vector2f(380, 680));
        portPanel.setFillColor(Color(15, 30, 55, 220));
        portPanel.setOutlineThickness(2);
        portPanel.setOutlineColor(Color(200, 100, 50, 150));
        portPanel.setPosition(460, 140);
        prefWindow.draw(portPanel);

        RectangleShape portHeader(Vector2f(380, 45));
        portHeader.setFillColor(Color(120, 60, 30, 200));
        portHeader.setPosition(460, 140);
        prefWindow.draw(portHeader);

        CircleShape warnIcon(12, 3);
        warnIcon.setFillColor(Color(255, 180, 80));
        warnIcon.setPosition(495, 150);
        prefWindow.draw(warnIcon);

        Text portTitle("PORTS TO AVOID", font, 16);
        portTitle.setFillColor(Color(255, 220, 180));
        portTitle.setStyle(Text::Bold);
        portTitle.setPosition(525, 152);
        prefWindow.draw(portTitle);

        Text scrollInfo("Scroll: " + to_string(portScrollOffset + 1) + "-" + 
                       to_string(min(portScrollOffset + maxVisiblePorts, (int)allPorts.getSize())) + 
                       " of " + to_string(allPorts.getSize()), font, 11);
        scrollInfo.setFillColor(Color(180, 150, 120));
        scrollInfo.setPosition(700, 155);
        prefWindow.draw(scrollInfo);

        
        for (int p = 0; p < maxVisiblePorts && (p + portScrollOffset) < allPorts.getSize(); p++)
        {
            int actualIndex = p + portScrollOffset;
            
            bool isAvoided = false;
            for (int i = 0; i < shipPrefs.avoidedPorts.getSize(); i++)
            {
                if (shipPrefs.avoidedPorts[i] == allPorts[actualIndex])
                {
                    isAvoided = true;
                    break;
                }
            }

            FloatRect bounds(480, 195 + p * portItemHeight, 340, portItemHeight - 3);
            bool isHovered = bounds.contains(mouse);

            RectangleShape portCard(Vector2f(340, portItemHeight - 3));
            if (isAvoided)
            {
                portCard.setFillColor(Color(80, 30, 30, 220));
                portCard.setOutlineColor(Color(255, 100, 100));
            }
            else if (isHovered)
            {
                portCard.setFillColor(Color(50, 40, 35, 200));
                portCard.setOutlineColor(Color(200, 150, 100));
            }
            else
            {
                portCard.setFillColor(Color(30, 25, 25, 180));
                portCard.setOutlineColor(Color(80, 60, 50));
            }
            portCard.setOutlineThickness(2);
            portCard.setPosition(480, 195 + p * portItemHeight);
            prefWindow.draw(portCard);

            RectangleShape avoidCheck(Vector2f(24, 24));
            avoidCheck.setFillColor(isAvoided ? Color(200, 50, 50) : Color(40, 35, 35));
            avoidCheck.setOutlineThickness(2);
            avoidCheck.setOutlineColor(isAvoided ? Color(255, 150, 150) : Color(100, 80, 80));
            avoidCheck.setPosition(495, 195 + p * portItemHeight + 10);
            prefWindow.draw(avoidCheck);

            if (isAvoided)
            {
                Text xMark("X", font, 16);
                xMark.setFillColor(Color::White);
                xMark.setStyle(Text::Bold);
                xMark.setPosition(501, 195 + p * portItemHeight + 8);
                prefWindow.draw(xMark);
            }

            Text portName(allPorts[actualIndex], font, 14);
            portName.setFillColor(isAvoided ? Color(255, 150, 150) : Color(200, 180, 160));
            portName.setStyle(isAvoided ? Text::Bold : Text::Regular);
            portName.setPosition(530, 195 + p * portItemHeight + 12);
            prefWindow.draw(portName);
        }

        
        RectangleShape timePanel(Vector2f(420, 400));
        timePanel.setFillColor(Color(15, 30, 55, 220));
        timePanel.setOutlineThickness(2);
        timePanel.setOutlineColor(Color(100, 200, 100, 150));
        timePanel.setPosition(900, 140);
        prefWindow.draw(timePanel);

        RectangleShape timeHeader(Vector2f(420, 45));
        timeHeader.setFillColor(Color(40, 100, 60, 200));
        timeHeader.setPosition(900, 140);
        prefWindow.draw(timeHeader);

        Text timeTitle("MAX VOYAGE TIME", font, 16);
        timeTitle.setFillColor(Color(200, 255, 200));
        timeTitle.setStyle(Text::Bold);
        timeTitle.setPosition(1020, 152);
        prefWindow.draw(timeTitle);

        Text timeLabel("Enter max hours (0 = unlimited):", font, 14);
        timeLabel.setFillColor(Color(180, 220, 180));
        timeLabel.setPosition(920, 210);
        prefWindow.draw(timeLabel);

        RectangleShape timeInputBox(Vector2f(200, 50));
        timeInputBox.setFillColor(maxTimeActive ? Color(30, 60, 40, 220) : Color(20, 40, 30, 200));
        timeInputBox.setOutlineThickness(2);
        timeInputBox.setOutlineColor(maxTimeActive ? Color(100, 255, 150) : Color(80, 150, 100));
        timeInputBox.setPosition(950, 250);
        prefWindow.draw(timeInputBox);

        Text timeInputText(maxTimeInput.empty() ? "0" : maxTimeInput, font, 24);
        timeInputText.setFillColor(maxTimeInput.empty() ? Color(100, 150, 100) : Color(150, 255, 150));
        timeInputText.setPosition(1020, 257);
        prefWindow.draw(timeInputText);

        Text hoursLabel("hours", font, 16);
        hoursLabel.setFillColor(Color(150, 200, 150));
        hoursLabel.setPosition(1170, 265);
        prefWindow.draw(hoursLabel);

        
        RectangleShape clearBtn(Vector2f(180, 50));
        clearBtn.setFillColor(Color(120, 50, 50, 220));
        clearBtn.setOutlineThickness(2);
        clearBtn.setOutlineColor(Color(255, 100, 100));
        clearBtn.setPosition(950, 700);
        prefWindow.draw(clearBtn);

        Text clearText("CLEAR ALL", font, 16);
        clearText.setFillColor(Color::White);
        clearText.setStyle(Text::Bold);
        clearText.setPosition(990, 712);
        prefWindow.draw(clearText);

        RectangleShape applyBtn(Vector2f(180, 50));
        applyBtn.setFillColor(Color(50, 100, 80, 220));
        applyBtn.setOutlineThickness(2);
        applyBtn.setOutlineColor(Color(100, 255, 150));
        applyBtn.setPosition(1150, 700);
        prefWindow.draw(applyBtn);

        Text applyText("APPLY", font, 16);
        applyText.setFillColor(Color::White);
        applyText.setStyle(Text::Bold);
        applyText.setPosition(1210, 712);
        prefWindow.draw(applyText);

        prefWindow.display();
    }
}



void showMultiLegRouteBuilder(Maps &graph, Font &font)
{
    
    RenderWindow window(VideoMode(1920, 1080), "Multi-Leg Route Builder");
    window.setFramerateLimit(60);
    
    
    View fixedView(FloatRect(0, 0, 1920, 1080));
    window.setView(fixedView);
    
    
    RenderWindow routeWindow(VideoMode(420, 750), "Linked List View");
    routeWindow.setFramerateLimit(60);
    routeWindow.setPosition(Vector2i(50, 100));

    
    Texture mapTexture;
    if (!mapTexture.loadFromFile("pics/map2.png"))
    {
        cout << "Map image not loaded!" << endl;
        return;
    }
    Sprite mapSprite(mapTexture);
    mapSprite.setColor(Color(160, 140, 160, 220));  

    
    LinkedListRoute customRoute;
    
    
    Clock animClock;
    float animTime = 0;
    
    
    int hoveredPortIndex = -1;
    
    
    bool insertMode = false;
    string insertAfterPort = "";  
    
    
    bool routeValid = true;
    string validationMessage = "Click on ports to build your multi-leg route";
    
    
    Color validColor(50, 220, 100);      
    Color invalidColor(220, 60, 60);     
    Color nodeColor(200, 100, 100);      
    Color accentColor(180, 80, 80);      
    
    
    struct ShipAnim {
        float progress;      
        float speed;         
        Color shipColor;     
    };
    Vector<ShipAnim> shipAnimations;
    
    
    auto getPortInfo = [&graph](int portIndex) -> tuple<int, int, Vector<string>> {
        
        string portName = portLocations[portIndex].name;
        int graphIdx = graph.findPortIndex(portName);
        int charge = 0;
        int routeCount = 0;
        Vector<string> companies;
        
        if (graphIdx != -1) {
            charge = graph.ports[graphIdx].charge;
            RouteNode* route = graph.ports[graphIdx].routeHead;
            while (route) {
                routeCount++;
                
                bool found = false;
                for (int c = 0; c < companies.getSize(); c++) {
                    if (companies[c] == route->shippingCompany) {
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    companies.push_back(route->shippingCompany);
                }
                route = route->next;
            }
        }
        return make_tuple(charge, routeCount, companies);
    };
    
    
    auto getRouteDetails = [&graph](const string& from, const string& to) -> tuple<int, string, string, string> {
        
        int fromIdx = graph.findPortIndex(from);
        if (fromIdx == -1) return make_tuple(0, "", "", "");
        RouteNode* route = graph.ports[fromIdx].routeHead;
        while (route) {
            if (route->destinationPort == to) {
                return make_tuple(route->cost, route->shippingCompany, route->departureTime, route->arrivalTime);
            }
            route = route->next;
        }
        return make_tuple(0, "", "", "");
    };
    
    
    auto calculateTotalTime = [&graph, &getRouteDetails](LinkedListRoute& route) -> int {
        int totalMinutes = 0;
        CustomRouteNode* cur = route.head;
        while (cur && cur->next) {
            auto [cost, company, depTime, arrTime] = getRouteDetails(cur->portName, cur->next->portName);
            if (!depTime.empty() && !arrTime.empty()) {
                int depMins = graph.convertToMinutes(depTime);
                int arrMins = graph.convertToMinutes(arrTime);
                int travelTime = arrMins - depMins;
                if (travelTime < 0) travelTime += 24 * 60; 
                totalMinutes += travelTime;
            }
            cur = cur->next;
        }
        return totalMinutes;
    };
    
    
    auto getPortPos = [](const string& portName) -> Vector2f {
        for (int i = 0; i < numLocations; i++) {
            if (portLocations[i].name == portName) {
                return Vector2f(portLocations[i].x, portLocations[i].y);
            }
        }
        return Vector2f(-1, -1);
    };
    
    
    auto routeExists = [&graph](const string& from, const string& to) -> bool {
        int fromIdx = graph.findPortIndex(from);
        if (fromIdx == -1) return false;
        RouteNode* route = graph.ports[fromIdx].routeHead;
        while (route) {
            if (route->destinationPort == to) return true;
            route = route->next;
        }
        return false;
    };
    
    
    auto getRouteCost = [&graph](const string& from, const string& to) -> int {
        int fromIdx = graph.findPortIndex(from);
        if (fromIdx == -1) return 0;
        RouteNode* route = graph.ports[fromIdx].routeHead;
        while (route) {
            if (route->destinationPort == to) {
                return route->cost;
            }
            route = route->next;
        }
        return 0;
    };
    
    
    auto validateRoute = [&]() {
        routeValid = true;
        validationMessage = "";
        
        if (!customRoute.head) {
            validationMessage = "Click on ports to build your multi-leg route";
            return;
        }
        
        CustomRouteNode* cur = customRoute.head;
        while (cur && cur->next) {
            if (!routeExists(cur->portName, cur->next->portName)) {
                routeValid = false;
                validationMessage = "Invalid: No route from " + cur->portName + " to " + cur->next->portName;
                return;
            }
            cur = cur->next;
        }
        
        if (routeValid && customRoute.head) {
            int stops = 0;
            int totalCost = 0;
            cur = customRoute.head;
            while (cur) {
                stops++;
                if (cur->next) {
                    totalCost += getRouteCost(cur->portName, cur->next->portName);
                }
                cur = cur->next;
            }
            validationMessage = "Valid Route | " + to_string(stops) + " Stops | Total: $" + to_string(totalCost);
        }
    };
    
    
    float scrollOffset = 0;
    
    while (window.isOpen())
    {
        Event event;
        Vector2f mousePos = window.mapPixelToCoords(Mouse::getPosition(window));
        float dt = animClock.restart().asSeconds();
        animTime += dt;
        
        
        while (window.pollEvent(event))
        {
            if (event.type == Event::Closed) {
                window.close();
                routeWindow.close();
            }
            
            if (event.type == Event::KeyPressed)
            {
                if (event.key.code == Keyboard::Escape) {
                    if (insertMode) {
                        
                        insertMode = false;
                        insertAfterPort = "";
                        cout << "Insert mode: CANCELLED" << endl;
                    } else {
                        window.close();
                        routeWindow.close();
                    }
                }
                if (event.key.code == Keyboard::C) {
                    customRoute.clear();
                    validateRoute();
                    insertMode = false;
                    insertAfterPort = "";
                }
            }
            
            
            if (event.type == Event::MouseButtonPressed && event.mouseButton.button == Mouse::Button::Left)
            {
                
                FloatRect backRect(20, 20, 120, 45);
                if (backRect.contains(mousePos)) {
                    clickSound.play();
                    window.close();
                    routeWindow.close();
                    continue;
                }
                
                
                FloatRect clearRect(160, 20, 150, 45);
                if (clearRect.contains(mousePos)) {
                    clickSound.play();
                    customRoute.clear();
                    validateRoute();
                    insertMode = false;
                    insertAfterPort = "";
                    continue;
                }
                
                
                for (int i = 0; i < numLocations; i++)
                {
                    float dist = sqrt(pow(mousePos.x - portLocations[i].x, 2) + 
                                     pow(mousePos.y - portLocations[i].y, 2));
                    if (dist < 25)
                    {
                        clickSound.play();
                        string clickedPort = portLocations[i].name;
                        
                        
                        bool portInRoute = false;
                        CustomRouteNode* foundNode = nullptr;
                        CustomRouteNode* cur = customRoute.head;
                        while (cur) {
                            if (cur->portName == clickedPort) {
                                portInRoute = true;
                                foundNode = cur;
                                break;
                            }
                            cur = cur->next;
                        }
                        
                        if (insertMode && !insertAfterPort.empty())
                        {
                            
                            
                            
                            if (customRoute.insertAfter(insertAfterPort, clickedPort)) {
                                cout << "Inserted " << clickedPort << " after " << insertAfterPort << endl;
                            }
                            validateRoute();  
                            insertMode = false;
                            insertAfterPort = "";
                        }
                        else if (portInRoute)
                        {
                            
                            if (insertMode && insertAfterPort == clickedPort) {
                                insertMode = false;
                                insertAfterPort = "";
                                cout << "Insert mode: OFF" << endl;
                            } else {
                                
                                insertMode = true;
                                insertAfterPort = clickedPort;
                                cout << "Insert mode: Click ANY port to insert AFTER " << insertAfterPort << endl;
                            }
                        }
                        else
                        {
                            
                            customRoute.append(clickedPort);
                            validateRoute();
                        }
                        
                        
                        {
                            auto ports = customRoute.getPortSequence();
                            cout << "Route: ";
                            for (int pi = 0; pi < ports.getSize(); ++pi) {
                                cout << ports[pi];
                                if (pi + 1 < ports.getSize()) cout << " -> ";
                            }
                            cout << endl;
                        }
                        break;
                    }
                }
            }
            
            
            if (event.type == Event::MouseButtonPressed && event.mouseButton.button == Mouse::Button::Right)
            {
                CustomRouteNode* cur = customRoute.head;
                while (cur)
                {
                    Vector2f pos = getPortPos(cur->portName);
                    float dist = sqrt(pow(mousePos.x - pos.x, 2) + pow(mousePos.y - pos.y, 2));
                    if (dist < 30)
                    {
                        customRoute.remove(cur->portName);
                        validateRoute();
                        
                        {
                            auto ports = customRoute.getPortSequence();
                            cout << "Ports seq: ";
                            for (int pi = 0; pi < ports.getSize(); ++pi) {
                                cout << ports[pi];
                                if (pi + 1 < ports.getSize()) cout << " -> ";
                            }
                            cout << endl;
                            auto legs = customRoute.getLegPairs();
                            if (!legs.empty()) {
                                cout << "Legs: ";
                                for (int li = 0; li < legs.getSize(); ++li) {
                                    auto p = legs[li];
                                    cout << "[" << p.first << " -> " << p.second << "]";
                                    if (li + 1 < legs.getSize()) cout << " -> ";
                                }
                                cout << endl;
                            }
                        }
                        break;
                    }
                    cur = cur->next;
                }
            }
        }
        
        
        if (routeWindow.isOpen())
        {
            Event routeEvent;
            Vector2f routeMousePos = routeWindow.mapPixelToCoords(Mouse::getPosition(routeWindow));
            
            while (routeWindow.pollEvent(routeEvent))
            {
                if (routeEvent.type == Event::Closed) {
                    routeWindow.close();
                }
                
                
                if (routeEvent.type == Event::MouseWheelScrolled) {
                    scrollOffset -= routeEvent.mouseWheelScroll.delta * 30;
                    if (scrollOffset < 0) scrollOffset = 0;
                }
                
                
                if (routeEvent.type == Event::MouseButtonPressed && routeEvent.mouseButton.button == Mouse::Button::Right)
                {
                    float checkY = 100 - scrollOffset;
                    CustomRouteNode* cur = customRoute.head;
                    while (cur)
                    {
                        FloatRect nodeRect(20, checkY, 360, 50);
                        if (nodeRect.contains(routeMousePos)) {
                            customRoute.remove(cur->portName);
                            validateRoute();
                            break;
                        }
                        checkY += cur->next ? 80 : 60;
                        cur = cur->next;
                    }
                }
            }
        }
        
        
        hoveredPortIndex = -1;
        for (int i = 0; i < numLocations; i++)
        {
            float dist = sqrt(pow(mousePos.x - portLocations[i].x, 2) + 
                             pow(mousePos.y - portLocations[i].y, 2));
            if (dist < 25) {
                hoveredPortIndex = i;
                break;
            }
        }
        
        
        window.clear(Color(20, 15, 25));  
        
        
        window.draw(mapSprite);
        
        
        RectangleShape mapOverlay(Vector2f(1920, 1080));
        mapOverlay.setFillColor(Color(20, 10, 30, 80));
        window.draw(mapOverlay);
        
        
        for (int i = 0; i < numLocations; i++)
        {
            
            bool inRoute = false;
            int routePosition = -1;
            CustomRouteNode* cur = customRoute.head;
            int pos = 0;
            while (cur) {
                if (cur->portName == portLocations[i].name) {
                    inRoute = true;
                    routePosition = pos;
                    break;
                }
                cur = cur->next;
                pos++;
            }
            
            float pulse = 0.7f + 0.3f * sin(animTime * 2.5f + i * 0.2f);
            
            
            if (insertMode && portLocations[i].name == insertAfterPort) {
                
                float insertPulse = 0.5f + 0.5f * sin(animTime * 6.0f);
                CircleShape insertRing(30 + insertPulse * 5);
                insertRing.setOrigin(insertRing.getRadius(), insertRing.getRadius());
                insertRing.setPosition(portLocations[i].x, portLocations[i].y);
                insertRing.setFillColor(Color::Transparent);
                insertRing.setOutlineThickness(4);
                insertRing.setOutlineColor(Color(255, 200, 50, static_cast<Uint8>(150 + 100 * insertPulse)));
                window.draw(insertRing);
                
                
                Text insertLabel("INSERT AFTER", font, 11);
                insertLabel.setStyle(Text::Bold);
                insertLabel.setFillColor(Color(255, 220, 80));
                insertLabel.setOutlineThickness(1);
                insertLabel.setOutlineColor(Color(40, 30, 0));
                FloatRect lb = insertLabel.getLocalBounds();
                insertLabel.setPosition(portLocations[i].x - lb.width / 2, portLocations[i].y - 50);
                window.draw(insertLabel);
            }
            
            if (inRoute) {
                
                CircleShape glow(25);
                glow.setOrigin(25, 25);
                glow.setPosition(portLocations[i].x, portLocations[i].y);
                glow.setFillColor(Color(180, 100, 80, static_cast<Uint8>(60 * pulse)));
                window.draw(glow);
                
                
                CircleShape dot(16);
                dot.setOrigin(16, 16);
                dot.setPosition(portLocations[i].x, portLocations[i].y);
                dot.setFillColor(Color(60, 30, 40));
                dot.setOutlineThickness(3);
                dot.setOutlineColor(Color(200, 120, 100));
                window.draw(dot);
                
                
                Text num(to_string(routePosition + 1), font, 16);
                num.setStyle(Text::Bold);
                num.setFillColor(Color(255, 220, 200));
                FloatRect nb = num.getLocalBounds();
                num.setOrigin(nb.width / 2, nb.height / 2 + 2);
                num.setPosition(portLocations[i].x, portLocations[i].y);
                window.draw(num);
            }
            else {
                
                CircleShape dot(i == hoveredPortIndex ? 12 : 9);
                dot.setOrigin(dot.getRadius(), dot.getRadius());
                dot.setPosition(portLocations[i].x, portLocations[i].y);
                
                if (i == hoveredPortIndex) {
                    CircleShape hoverGlow(18);
                    hoverGlow.setOrigin(18, 18);
                    hoverGlow.setPosition(portLocations[i].x, portLocations[i].y);
                    hoverGlow.setFillColor(Color(200, 150, 120, 80));
                    window.draw(hoverGlow);
                    
                    dot.setFillColor(Color(220, 180, 160));
                    dot.setOutlineThickness(2);
                    dot.setOutlineColor(Color::White);
                } else {
                    dot.setFillColor(Color(150, 80, 80, static_cast<Uint8>(200 * pulse)));
                    dot.setOutlineThickness(1);
                    dot.setOutlineColor(Color(200, 120, 120, 150));
                }
                window.draw(dot);
            }
            
            
            if (i == hoveredPortIndex) {
                Text label(portLocations[i].name, font, 16);
                label.setStyle(Text::Bold);
                label.setFillColor(Color::White);
                label.setOutlineThickness(2);
                label.setOutlineColor(Color(40, 20, 30));
                label.setPosition(portLocations[i].x + 18, portLocations[i].y - 10);
                window.draw(label);
            }
        }
        
        
        if (hoveredPortIndex != -1) {
            auto [charge, routeCount, companies] = getPortInfo(hoveredPortIndex);
            
            
            int tooltipHeight = 95 + (companies.getSize() > 0 ? 20 + min((int)companies.getSize(), 4) * 16 : 0);
            
            
            float tooltipX = portLocations[hoveredPortIndex].x + 25;
            float tooltipY = portLocations[hoveredPortIndex].y - 20;
            if (tooltipX + 220 > 1900) tooltipX = portLocations[hoveredPortIndex].x - 245;
            if (tooltipY + tooltipHeight > 1000) tooltipY = 1000 - tooltipHeight;
            if (tooltipY < 80) tooltipY = 80;
            
            
            RectangleShape tooltipBg(Vector2f(220, tooltipHeight));
            tooltipBg.setFillColor(Color(30, 25, 35, 240));
            tooltipBg.setOutlineThickness(2);
            tooltipBg.setOutlineColor(Color(180, 120, 120));
            tooltipBg.setPosition(tooltipX, tooltipY);
            window.draw(tooltipBg);
            
            
            RectangleShape tooltipHeader(Vector2f(220, 28));
            tooltipHeader.setFillColor(Color(80, 50, 60));
            tooltipHeader.setPosition(tooltipX, tooltipY);
            window.draw(tooltipHeader);
            
            
            Text portNameText(portLocations[hoveredPortIndex].name, font, 14);
            portNameText.setStyle(Text::Bold);
            portNameText.setFillColor(Color(255, 220, 200));
            portNameText.setPosition(tooltipX + 10, tooltipY + 5);
            window.draw(portNameText);
            
            
            Text chargeText("Port Charge: $" + to_string(charge), font, 13);
            chargeText.setFillColor(Color(100, 220, 150));
            chargeText.setPosition(tooltipX + 10, tooltipY + 35);
            window.draw(chargeText);
            
            
            Text routesText("Outgoing Routes: " + to_string(routeCount), font, 13);
            routesText.setFillColor(Color(150, 200, 255));
            routesText.setPosition(tooltipX + 10, tooltipY + 55);
            window.draw(routesText);
            
            
            int dockStatus = globalPortDocking[hoveredPortIndex].getOccupiedDocks();
            int queueLen = globalPortDocking[hoveredPortIndex].getQueueLength();
            Text dockText("Docks: " + to_string(dockStatus) + "/4 | Queue: " + to_string(queueLen), font, 12);
            dockText.setFillColor(dockStatus < 4 ? Color(100, 255, 150) : Color(255, 150, 100));
            dockText.setPosition(tooltipX + 10, tooltipY + 75);
            window.draw(dockText);
            
            
            if (companies.getSize() > 0) {
                Text compLabel("Companies:", font, 12);
                compLabel.setFillColor(Color(180, 180, 180));
                compLabel.setPosition(tooltipX + 10, tooltipY + 95);
                window.draw(compLabel);
                
                for (int c = 0; c < min((int)companies.getSize(), 4); c++) {
                    Text compText("• " + companies[c], font, 11);
                    compText.setFillColor(Color(200, 180, 160));
                    compText.setPosition(tooltipX + 15, tooltipY + 112 + c * 16);
                    window.draw(compText);
                }
                if (companies.getSize() > 4) {
                    Text moreText("+" + to_string(companies.getSize() - 4) + " more...", font, 11);
                    moreText.setFillColor(Color(150, 150, 150));
                    moreText.setPosition(tooltipX + 15, tooltipY + 112 + 4 * 16);
                    window.draw(moreText);
                }
            }
        }
        
        
        CustomRouteNode* routeNode = customRoute.head;
        
        
        int legCount = 0;
        routeNode = customRoute.head;
        while (routeNode && routeNode->next) {
            legCount++;
            routeNode = routeNode->next;
        }
        
        
        while (shipAnimations.getSize() < legCount) {
            ShipAnim newShip;
            newShip.progress = (float)(rand() % 100) / 100.0f;
            newShip.speed = 0.15f + (float)(rand() % 20) / 100.0f;
            newShip.shipColor = Color(200 + rand() % 55, 200 + rand() % 55, 200 + rand() % 55);
            shipAnimations.push_back(newShip);
        }
        
        
        routeNode = customRoute.head;
        int legIndex = 0;
        while (routeNode && routeNode->next) {
            Vector2f p1 = getPortPos(routeNode->portName);
            Vector2f p2 = getPortPos(routeNode->next->portName);
            
            if (p1.x >= 0 && p2.x >= 0) {
                float len = sqrt(pow(p2.x - p1.x, 2) + pow(p2.y - p1.y, 2));
                float angle = atan2(p2.y - p1.y, p2.x - p1.x) * 180 / 3.14159f;
                float angleRad = atan2(p2.y - p1.y, p2.x - p1.x);
                
                
                bool legValid = routeExists(routeNode->portName, routeNode->next->portName);
                Color lineColor = legValid ? validColor : invalidColor;
                
                
                RectangleShape glow(Vector2f(len, 12));
                glow.setPosition(p1);
                glow.setRotation(angle);
                glow.setFillColor(Color(lineColor.r, lineColor.g, lineColor.b, 40));
                window.draw(glow);
                
                
                RectangleShape line(Vector2f(len, 4));
                line.setPosition(p1);
                line.setRotation(angle);
                line.setFillColor(lineColor);
                window.draw(line);
                
                
                if (legValid && legIndex < shipAnimations.getSize()) {
                    
                    shipAnimations[legIndex].progress += dt * shipAnimations[legIndex].speed;
                    if (shipAnimations[legIndex].progress > 1.0f) {
                        shipAnimations[legIndex].progress = 0.0f;
                    }
                    
                    float t = shipAnimations[legIndex].progress;
                    Vector2f shipPos = p1 + (p2 - p1) * t;
                    
                    
                    ConvexShape shipBody;
                    shipBody.setPointCount(5);
                    shipBody.setPoint(0, Vector2f(15, 0));   
                    shipBody.setPoint(1, Vector2f(-10, -8)); 
                    shipBody.setPoint(2, Vector2f(-8, -8));  
                    shipBody.setPoint(3, Vector2f(-8, 8));   
                    shipBody.setPoint(4, Vector2f(-10, 8));  
                    shipBody.setPosition(shipPos);
                    shipBody.setRotation(angle);
                    shipBody.setFillColor(Color(60, 80, 120));
                    shipBody.setOutlineThickness(2);
                    shipBody.setOutlineColor(Color(100, 150, 220));
                    window.draw(shipBody);
                    
                    
                    RectangleShape cabin(Vector2f(8, 10));
                    cabin.setOrigin(4, 5);
                    cabin.setPosition(shipPos.x - 3 * cos(angleRad), shipPos.y - 3 * sin(angleRad));
                    cabin.setRotation(angle);
                    cabin.setFillColor(Color(80, 60, 50));
                    cabin.setOutlineThickness(1);
                    cabin.setOutlineColor(Color(120, 100, 80));
                    window.draw(cabin);
                    
                    
                    for (int s = 1; s <= 3; s++) {
                        float smokeT = t - s * 0.03f;
                        if (smokeT < 0) smokeT += 1.0f;
                        Vector2f smokePos = p1 + (p2 - p1) * smokeT;
                        CircleShape smoke(3 + s);
                        smoke.setOrigin(3 + s, 3 + s);
                        smoke.setPosition(smokePos);
                        smoke.setFillColor(Color(200, 200, 200, 100 - s * 30));
                        window.draw(smoke);
                    }
                } else if (!legValid) {
                    
                    float t = fmod(animTime * 0.3f, 1.0f);
                    Vector2f xPos = p1 + (p2 - p1) * t;
                    Text xMark("X", font, 16);
                    xMark.setFillColor(Color(255, 100, 100, 200));
                    xMark.setStyle(Text::Bold);
                    xMark.setPosition(xPos.x - 6, xPos.y - 10);
                    window.draw(xMark);
                }
                
                
                Vector2f mid = (p1 + p2) / 2.0f;
                ConvexShape arrow;
                arrow.setPointCount(3);
                arrow.setPoint(0, Vector2f(12, 0));
                arrow.setPoint(1, Vector2f(-6, 7));
                arrow.setPoint(2, Vector2f(-6, -7));
                arrow.setPosition(mid);
                arrow.setRotation(angle);
                arrow.setFillColor(lineColor);
                window.draw(arrow);
                
                
                if (!legValid) {
                    Text noRoute("NO ROUTE", font, 12);
                    noRoute.setFillColor(Color(255, 100, 100));
                    noRoute.setStyle(Text::Bold);
                    noRoute.setPosition(mid.x - 30, mid.y - 20);
                    window.draw(noRoute);
                }
            }
            routeNode = routeNode->next;
            legIndex++;
        }
        
        
        RectangleShape topBar(Vector2f(1920, 70));
        topBar.setFillColor(Color(35, 25, 35, 250));
        topBar.setPosition(0, 0);
        window.draw(topBar);
        
        
        RectangleShape topGlow(Vector2f(1920, 2));
        topGlow.setFillColor(Color(180, 100, 100, 150));
        topGlow.setPosition(0, 68);
        window.draw(topGlow);
        
        
        RectangleShape backBtn(Vector2f(120, 45));
        bool backHover = backBtn.getGlobalBounds().contains(mousePos);
        backBtn.setFillColor(backHover ? Color(80, 50, 60) : Color(60, 35, 45));
        backBtn.setOutlineThickness(2);
        backBtn.setOutlineColor(backHover ? Color(200, 150, 150) : Color(150, 100, 100));
        backBtn.setPosition(20, 12);
        window.draw(backBtn);
        
        Text backText("< BACK", font, 16);
        backText.setStyle(Text::Bold);
        backText.setFillColor(Color(220, 200, 200));
        backText.setPosition(42, 24);
        window.draw(backText);
        
        
        RectangleShape clearBtn(Vector2f(140, 45));
        bool clearHover = clearBtn.getGlobalBounds().contains(mousePos);
        clearBtn.setFillColor(clearHover ? Color(140, 50, 50) : Color(100, 40, 45));
        clearBtn.setOutlineThickness(2);
        clearBtn.setOutlineColor(clearHover ? Color(220, 150, 150) : Color(180, 100, 100));
        clearBtn.setPosition(160, 12);
        window.draw(clearBtn);
        
        Text clearText("CLEAR (C)", font, 16);
        clearText.setStyle(Text::Bold);
        clearText.setFillColor(Color(255, 220, 220));
        clearText.setPosition(185, 24);
        window.draw(clearText);
        
        
        Text title("MULTI-LEG ROUTE BUILDER", font, 26);
        title.setStyle(Text::Bold);
        title.setFillColor(Color(220, 160, 160));
        FloatRect titleBounds = title.getLocalBounds();
        title.setPosition(960 - titleBounds.width / 2, 20);
        window.draw(title);
        
        
        Text instr("CLICK: Add/Insert | RIGHT-CLICK: Remove | ESC: Cancel/Exit", font, 14);
        instr.setFillColor(Color(180, 150, 160));
        instr.setPosition(1280, 28);
        window.draw(instr);
        
        
        if (insertMode) {
            float modeFlash = 0.6f + 0.4f * sin(animTime * 5.0f);
            RectangleShape modeBg(Vector2f(280, 35));
            modeBg.setFillColor(Color(60, 50, 10, static_cast<Uint8>(200 * modeFlash)));
            modeBg.setOutlineThickness(2);
            modeBg.setOutlineColor(Color(255, 200, 50, static_cast<Uint8>(255 * modeFlash)));
            modeBg.setPosition(820, 55);
            window.draw(modeBg);
            
            Text modeText("INSERT MODE: Click port to add", font, 14);
            modeText.setStyle(Text::Bold);
            modeText.setFillColor(Color(255, 220, 80));
            modeText.setPosition(830, 62);
            window.draw(modeText);
        }
        
        
        RectangleShape statusBar(Vector2f(1920, 50));
        statusBar.setFillColor(routeValid ? Color(25, 40, 30, 250) : Color(50, 25, 25, 250));
        statusBar.setPosition(0, 1030);
        window.draw(statusBar);
        
        
        RectangleShape statusLine(Vector2f(1920, 3));
        statusLine.setFillColor(routeValid ? Color(80, 200, 120) : Color(200, 80, 80));
        statusLine.setPosition(0, 1030);
        window.draw(statusLine);
        
        Text statusText(validationMessage, font, 18);
        statusText.setFillColor(routeValid ? Color(120, 220, 150) : Color(220, 140, 140));
        FloatRect sb = statusText.getLocalBounds();
        statusText.setPosition(960 - sb.width / 2, 1045);
        window.draw(statusText);
        
        
        if (customRoute.head) {
            
            int totalStops = 0;
            int totalCost = 0;
            int validLegs = 0;
            int invalidLegs = 0;
            CustomRouteNode* cur = customRoute.head;
            
            while (cur) {
                totalStops++;
                if (cur->next) {
                    if (routeExists(cur->portName, cur->next->portName)) {
                        totalCost += getRouteCost(cur->portName, cur->next->portName);
                        validLegs++;
                    } else {
                        invalidLegs++;
                    }
                }
                cur = cur->next;
            }
            
            int totalTime = calculateTotalTime(customRoute);
            int hours = totalTime / 60;
            int mins = totalTime % 60;
            
            
            RectangleShape summaryPanel(Vector2f(280, 220));
            summaryPanel.setFillColor(Color(25, 30, 40, 240));
            summaryPanel.setOutlineThickness(2);
            summaryPanel.setOutlineColor(Color(100, 140, 180));
            summaryPanel.setPosition(20, 790);
            window.draw(summaryPanel);
            
            
            RectangleShape summaryHeader(Vector2f(280, 35));
            summaryHeader.setFillColor(Color(50, 70, 100));
            summaryHeader.setPosition(20, 790);
            window.draw(summaryHeader);
            
            Text summaryTitle("ROUTE SUMMARY", font, 16);
            summaryTitle.setStyle(Text::Bold);
            summaryTitle.setFillColor(Color(180, 210, 255));
            summaryTitle.setPosition(85, 798);
            window.draw(summaryTitle);
            
            
            float yPos = 835;
            
            
            Text stopsLabel("Total Stops:", font, 14);
            stopsLabel.setFillColor(Color(180, 180, 180));
            stopsLabel.setPosition(35, yPos);
            window.draw(stopsLabel);
            Text stopsValue(to_string(totalStops), font, 14);
            stopsValue.setFillColor(Color(255, 220, 150));
            stopsValue.setStyle(Text::Bold);
            stopsValue.setPosition(240, yPos);
            window.draw(stopsValue);
            
            yPos += 25;
            
            
            Text validLabel("Valid Legs:", font, 14);
            validLabel.setFillColor(Color(180, 180, 180));
            validLabel.setPosition(35, yPos);
            window.draw(validLabel);
            Text validValue(to_string(validLegs), font, 14);
            validValue.setFillColor(Color(100, 255, 130));
            validValue.setStyle(Text::Bold);
            validValue.setPosition(240, yPos);
            window.draw(validValue);
            
            yPos += 25;
            
            
            if (invalidLegs > 0) {
                Text invalidLabel("Invalid Legs:", font, 14);
                invalidLabel.setFillColor(Color(180, 180, 180));
                invalidLabel.setPosition(35, yPos);
                window.draw(invalidLabel);
                Text invalidValue(to_string(invalidLegs), font, 14);
                invalidValue.setFillColor(Color(255, 100, 100));
                invalidValue.setStyle(Text::Bold);
                invalidValue.setPosition(240, yPos);
                window.draw(invalidValue);
                yPos += 25;
            }
            
            
            RectangleShape divider(Vector2f(250, 1));
            divider.setFillColor(Color(80, 100, 120));
            divider.setPosition(35, yPos + 5);
            window.draw(divider);
            yPos += 15;
            
            
            Text costLabel("Total Cost:", font, 15);
            costLabel.setFillColor(Color(180, 180, 180));
            costLabel.setPosition(35, yPos);
            window.draw(costLabel);
            Text costValue("$" + to_string(totalCost), font, 15);
            costValue.setFillColor(Color(100, 255, 150));
            costValue.setStyle(Text::Bold);
            costValue.setPosition(220, yPos);
            window.draw(costValue);
            
            yPos += 28;
            
            
            Text timeLabel("Est. Time:", font, 15);
            timeLabel.setFillColor(Color(180, 180, 180));
            timeLabel.setPosition(35, yPos);
            window.draw(timeLabel);
            string timeStr = to_string(hours) + "h " + to_string(mins) + "m";
            Text timeValue(timeStr, font, 15);
            timeValue.setFillColor(Color(150, 200, 255));
            timeValue.setStyle(Text::Bold);
            timeValue.setPosition(210, yPos);
            window.draw(timeValue);
            
            yPos += 28;
            
            
            RectangleShape statusIndicator(Vector2f(250, 25));
            statusIndicator.setFillColor(routeValid ? Color(30, 80, 50) : Color(80, 40, 40));
            statusIndicator.setOutlineThickness(1);
            statusIndicator.setOutlineColor(routeValid ? Color(80, 180, 100) : Color(180, 80, 80));
            statusIndicator.setPosition(35, yPos);
            window.draw(statusIndicator);
            
            Text statusLabel(routeValid ? "ROUTE VALID" : "ROUTE INVALID", font, 13);
            statusLabel.setStyle(Text::Bold);
            statusLabel.setFillColor(routeValid ? Color(120, 255, 150) : Color(255, 120, 120));
            FloatRect slb = statusLabel.getLocalBounds();
            statusLabel.setPosition(160 - slb.width / 2, yPos + 4);
            window.draw(statusLabel);
        }
        
        window.display();
        
        
        if (routeWindow.isOpen())
        {
            routeWindow.clear(Color(25, 20, 28));  
            
            
            RectangleShape titleBar(Vector2f(420, 65));
            titleBar.setFillColor(Color(40, 30, 40));
            titleBar.setPosition(0, 0);
            routeWindow.draw(titleBar);
            
            
            RectangleShape titleGlow(Vector2f(420, 2));
            titleGlow.setFillColor(Color(180, 100, 100, 180));
            titleGlow.setPosition(0, 63);
            routeWindow.draw(titleGlow);
            
            Text routeTitle("LINKED LIST ROUTE", font, 20);
            routeTitle.setStyle(Text::Bold);
            routeTitle.setFillColor(Color(200, 150, 150));
            routeTitle.setPosition(110, 20);
            routeWindow.draw(routeTitle);
            
            
            float listY = 85 - scrollOffset;
            routeNode = customRoute.head;
            int nodeIndex = 0;
            
            while (routeNode)
            {
                if (listY > -100 && listY < 750)
                {
                    bool hasNext = routeNode->next != nullptr;
                    bool legValid = hasNext ? routeExists(routeNode->portName, routeNode->next->portName) : true;
                    
                    
                    RectangleShape nodeBox(Vector2f(375, 52));
                    nodeBox.setFillColor(Color(45, 35, 45, 250));
                    nodeBox.setOutlineThickness(2);
                    nodeBox.setOutlineColor(legValid ? Color(100, 180, 120) : Color(200, 100, 100));
                    nodeBox.setPosition(22, listY);
                    routeWindow.draw(nodeBox);
                    
                    
                    CircleShape numCircle(16);
                    numCircle.setOrigin(16, 16);
                    numCircle.setFillColor(Color(150, 80, 80));
                    numCircle.setOutlineThickness(2);
                    numCircle.setOutlineColor(Color(200, 140, 140));
                    numCircle.setPosition(58, listY + 26);
                    routeWindow.draw(numCircle);
                    
                    Text numText(to_string(nodeIndex + 1), font, 16);
                    numText.setStyle(Text::Bold);
                    numText.setFillColor(Color(255, 230, 220));
                    FloatRect nb = numText.getLocalBounds();
                    numText.setOrigin(nb.width / 2, nb.height / 2 + 2);
                    numText.setPosition(58, listY + 26);
                    routeWindow.draw(numText);
                    
                    
                    Text portName(routeNode->portName, font, 16);
                    portName.setFillColor(Color(230, 220, 220));
                    portName.setPosition(90, listY + 15);
                    routeWindow.draw(portName);
                    
                    
                    if (hasNext) {
                        int cost = getRouteCost(routeNode->portName, routeNode->next->portName);
                        Text costText(legValid ? ("$" + to_string(cost)) : "NO ROUTE", font, 13);
                        costText.setFillColor(legValid ? Color(100, 220, 130) : Color(220, 100, 100));
                        costText.setStyle(legValid ? Text::Regular : Text::Bold);
                        costText.setPosition(300, listY + 17);
                        routeWindow.draw(costText);
                    }
                    
                    
                    if (hasNext) {
                        RectangleShape arrowLine(Vector2f(3, 22));
                        arrowLine.setFillColor(legValid ? validColor : invalidColor);
                        arrowLine.setPosition(205, listY + 54);
                        routeWindow.draw(arrowLine);
                        
                        ConvexShape arrowHead;
                        arrowHead.setPointCount(3);
                        arrowHead.setPoint(0, Vector2f(0, 8));
                        arrowHead.setPoint(1, Vector2f(-7, 0));
                        arrowHead.setPoint(2, Vector2f(7, 0));
                        arrowHead.setFillColor(legValid ? validColor : invalidColor);
                        arrowHead.setPosition(207, listY + 76);
                        routeWindow.draw(arrowHead);
                    } else {
                        
                        Text nullText("-> NULL", font, 13);
                        nullText.setFillColor(Color(140, 120, 130));
                        nullText.setPosition(300, listY + 17);
                        routeWindow.draw(nullText);
                    }
                }
                
                listY += routeNode->next ? 80 : 60;
                routeNode = routeNode->next;
                nodeIndex++;
            }
            
            if (nodeIndex == 0) {
                Text emptyText("No ports added yet", font, 17);
                emptyText.setFillColor(Color(180, 150, 160));
                emptyText.setPosition(125, 140);
                routeWindow.draw(emptyText);
                
                Text hintText("Click on ports in the map", font, 14);
                hintText.setFillColor(Color(150, 130, 140));
                hintText.setPosition(115, 175);
                routeWindow.draw(hintText);
                
                Text hintText2("to build your route", font, 14);
                hintText2.setFillColor(Color(150, 130, 140));
                hintText2.setPosition(135, 200);
                routeWindow.draw(hintText2);
            }
            
            
            RectangleShape bottomBar(Vector2f(420, 50));
            bottomBar.setFillColor(Color(40, 30, 40));
            bottomBar.setPosition(0, 700);
            routeWindow.draw(bottomBar);
            
            Text scrollHint("Scroll to see more | Right-click to remove", font, 12);
            scrollHint.setFillColor(Color(160, 140, 150));
            scrollHint.setPosition(75, 718);
            routeWindow.draw(scrollHint);
            
            routeWindow.display();
        }
    }
}

#include "BookingInterface.h"

Maps readPorts(string fileName)
{
    Maps map;

    ifstream file(fileName);
    if (!file.is_open())
    {
        cout << "Error opening file: " << fileName << endl;
        return map;
    }

    string port;
    int charge;

    while (file >> port >> charge)
    {
        map.addPort(port, charge);
    }

    file.close();
    return map;
}


void addRoutesFromFile(Maps &graph, string fileName)
{
    ifstream file(fileName);
    if (!file.is_open())
    {
        cout << "Error opening " << fileName << "!" << endl;
        return;
    }

    string start, dest, depDate, depTime, arrTime, company;
    int cost;

    while (file >> start >> dest >> depDate >> depTime >> arrTime >> cost >> company)
    {
        graph.addRoute(start, dest, depDate, depTime, arrTime, cost, company);
    }

    file.close();
}


void openShipPreferencesUI(Maps &graph, Font &font)
{
    RenderWindow window(VideoMode(1920, 1080), "Ship Preferences - OceanRoute Navigator", Style::Fullscreen);
    window.setFramerateLimit(60);

    
    Vector<string> companies;
    companies.push_back("MaerskLine");
    companies.push_back("MSC");
    companies.push_back("COSCO");
    companies.push_back("CMA_CGM");
    companies.push_back("Evergreen");
    companies.push_back("HapagLloyd");
    companies.push_back("ONE");
    companies.push_back("YangMing");
    companies.push_back("ZIM");
    companies.push_back("PIL");

    
    Color companyColors[] = {
        Color(0, 91, 187),  
        Color(255, 204, 0), 
        Color(0, 51, 102),  
        Color(0, 114, 198), 
        Color(0, 128, 0),   
        Color(255, 102, 0), 
        Color(255, 0, 127), 
        Color(255, 165, 0), 
        Color(0, 102, 153), 
        Color(153, 0, 0)    
    };

    
    Vector<string> allPorts;
    for (int i = 0; i < graph.ports.getSize(); i++)
    {
        allPorts.push_back(graph.ports[i].portName);
    }

    
    string maxTimeInput = "";
    bool maxTimeActive = false;
    int scrollOffset = 0;
    int avoidScrollOffset = 0;

    Clock animClock;

    
    struct Particle
    {
        float x, y, speed, size;
        int type; 
    };
    Vector<Particle> particles;
    for (int i = 0; i < 80; i++)
    {
        Particle p;
        p.x = rand() % 1920;
        p.y = rand() % 1080;
        p.speed = 0.3f + (rand() % 100) / 100.0f;
        p.size = 1 + rand() % 4;
        p.type = rand() % 3;
        particles.push_back(p);
    }

    while (window.isOpen())
    {
        float time = animClock.getElapsedTime().asSeconds();
        Vector2i mousePos = Mouse::getPosition(window);
        Vector2f mouse(mousePos.x, mousePos.y);

        Event e;
        while (window.pollEvent(e))
        {
            if (e.type == Event::Closed)
                window.close();

            if (e.type == Event::KeyPressed)
            {
                if (e.key.code == Keyboard::Escape)
                    window.close();
            }

            if (e.type == Event::TextEntered && maxTimeActive)
            {
                if (e.text.unicode == '\b' && maxTimeInput.length() > 0)
                {
                    maxTimeInput.pop_back();
                }
                else if (e.text.unicode >= '0' && e.text.unicode <= '9' && maxTimeInput.length() < 5)
                {
                    maxTimeInput += static_cast<char>(e.text.unicode);
                }
            }

            if (e.type == Event::MouseWheelScrolled)
            {
                if (mouse.x < 640)
                    scrollOffset -= e.mouseWheelScroll.delta * 30;
                else if (mouse.x > 640 && mouse.x < 1280)
                    avoidScrollOffset -= e.mouseWheelScroll.delta * 30;
            }
        }

        
        RectangleShape bgTop(Vector2f(1920, 540));
        bgTop.setFillColor(Color(5, 15, 35));
        bgTop.setPosition(0, 0);
        window.draw(bgTop);

        RectangleShape bgBottom(Vector2f(1920, 540));
        bgBottom.setFillColor(Color(15, 30, 60));
        bgBottom.setPosition(0, 540);
        window.draw(bgBottom);

        
        for (int w = 0; w < 5; w++)
        {
            for (int i = 0; i < 40; i++)
            {
                float waveY = 950 + w * 25 + sin(time * 1.5f + i * 0.3f + w) * 15;
                CircleShape wave(8 - w);
                wave.setFillColor(Color(20 + w * 10, 60 + w * 15, 120 + w * 20, 100 - w * 15));
                wave.setPosition(i * 50 + fmod(time * 20, 50.0f), waveY);
                window.draw(wave);
            }
        }

        
        for (int i = 0; i < particles.getSize(); i++)
        {
            particles[i].x += particles[i].speed;
            if (particles[i].x > 1920)
                particles[i].x = 0;

            float twinkle = 0.5f + 0.5f * sin(time * 3 + i);

            if (particles[i].type == 0) 
            {
                CircleShape star(particles[i].size);
                star.setFillColor(Color(255, 255, 255, 50 + twinkle * 80));
                star.setPosition(particles[i].x, particles[i].y);
                window.draw(star);
            }
            else if (particles[i].type == 1) 
            {
                RectangleShape ship(Vector2f(particles[i].size * 3, particles[i].size));
                ship.setFillColor(Color(200, 200, 255, 30 + twinkle * 40));
                ship.setPosition(particles[i].x, particles[i].y + sin(time + i) * 3);
                window.draw(ship);
            }
        }

        
        RectangleShape headerBar(Vector2f(1920, 130));
        headerBar.setFillColor(Color(10, 25, 50, 220));
        headerBar.setPosition(0, 0);
        window.draw(headerBar);

        
        for (int i = 0; i < 1920; i += 2)
        {
            float hue = fmod(time * 50 + i * 0.2f, 360.0f);
            int r = 128 + 127 * sin(hue * 3.14159f / 180.0f);
            int g = 128 + 127 * sin((hue + 120) * 3.14159f / 180.0f);
            int b = 128 + 127 * sin((hue + 240) * 3.14159f / 180.0f);

            RectangleShape gradLine(Vector2f(2, 4));
            gradLine.setFillColor(Color(r, g, b, 200));
            gradLine.setPosition(i, 126);
            window.draw(gradLine);
        }

        
        
        for (int g = 3; g >= 0; g--)
        {
            Text titleGlow("SHIP PREFERENCES", font, 52);
            titleGlow.setStyle(Text::Bold);
            titleGlow.setFillColor(Color(100, 200, 255, 20 - g * 5));
            FloatRect glowBounds = titleGlow.getLocalBounds();
            titleGlow.setOrigin(glowBounds.width / 2, 0);
            titleGlow.setPosition(960 + g, 25 + g);
            window.draw(titleGlow);
        }

        Text title("SHIP PREFERENCES", font, 52);
        title.setStyle(Text::Bold);
        title.setFillColor(Color(100, 220, 255));
        title.setOutlineThickness(2);
        title.setOutlineColor(Color(20, 80, 150));
        FloatRect titleBounds = title.getLocalBounds();
        title.setOrigin(titleBounds.width / 2, 0);
        title.setPosition(960, 25);
        window.draw(title);

        
        Text shipIcon("<*>", font, 30);
        shipIcon.setFillColor(Color(255, 200, 100));
        shipIcon.setPosition(titleBounds.width / 2 + 960 + 20, 35);
        window.draw(shipIcon);

        Text shipIcon2("<*>", font, 30);
        shipIcon2.setFillColor(Color(255, 200, 100));
        shipIcon2.setPosition(960 - titleBounds.width / 2 - 60, 35);
        window.draw(shipIcon2);

        Text subtitle("Customize your voyage - Select companies, avoid ports, set time limits", font, 20);
        subtitle.setFillColor(Color(180, 200, 230));
        FloatRect subBounds = subtitle.getLocalBounds();
        subtitle.setOrigin(subBounds.width / 2, 0);
        subtitle.setPosition(960, 85);
        window.draw(subtitle);

        

        
        
        RectangleShape col1Shadow(Vector2f(590, 710));
        col1Shadow.setFillColor(Color(0, 0, 0, 80));
        col1Shadow.setPosition(35, 155);
        window.draw(col1Shadow);

        RectangleShape col1Bg(Vector2f(580, 700));
        col1Bg.setFillColor(Color(15, 35, 70, 240));
        col1Bg.setOutlineThickness(3);
        col1Bg.setOutlineColor(Color(50, 150, 255));
        col1Bg.setPosition(30, 150);
        window.draw(col1Bg);

        
        RectangleShape col1Header(Vector2f(580, 55));
        col1Header.setFillColor(Color(30, 80, 150, 200));
        col1Header.setPosition(30, 150);
        window.draw(col1Header);

        
        float iconBounce = sin(time * 3) * 3;
        Text companyIcon(">", font, 24);
        companyIcon.setPosition(45, 158 + iconBounce);
        window.draw(companyIcon);

        Text col1Title("PREFERRED COMPANIES", font, 22);
        col1Title.setStyle(Text::Bold);
        col1Title.setFillColor(Color(100, 200, 255));
        col1Title.setPosition(90, 165);
        window.draw(col1Title);

        Text col1Hint("Select shipping lines for your voyage", font, 13);
        col1Hint.setFillColor(Color(150, 180, 220));
        col1Hint.setPosition(50, 210);
        window.draw(col1Hint);

        
        int selectedCount = shipPrefs.preferredCompanies.getSize();
        if (selectedCount > 0)
        {
            CircleShape badge(15);
            badge.setFillColor(Color(50, 200, 100));
            badge.setPosition(550, 160);
            window.draw(badge);

            Text badgeText(to_string(selectedCount), font, 16);
            badgeText.setFillColor(Color::White);
            badgeText.setStyle(Text::Bold);
            badgeText.setPosition(selectedCount < 10 ? 558 : 554, 162);
            window.draw(badgeText);
        }

        
        float companyY = 240;
        static bool companyClicked = false;
        for (int i = 0; i < companies.getSize(); i++)
        {
            float yPos = companyY + i * 48 - scrollOffset;
            if (yPos < 235 || yPos > 810)
                continue;

            bool isSelected = shipPrefs.isCompanyPreferred(companies[i]) && shipPrefs.preferredCompanies.getSize() > 0;

            if (shipPrefs.preferredCompanies.getSize() == 0)
                isSelected = false;

            FloatRect cardBounds(45, yPos, 550, 44);
            bool hovered = cardBounds.contains(mouse);

            
            RectangleShape card(Vector2f(550, 42));
            card.setPosition(45, yPos);

            if (isSelected)
            {
                card.setFillColor(Color(companyColors[i].r / 3, companyColors[i].g / 3, companyColors[i].b / 3, 200));
                card.setOutlineThickness(2);
                card.setOutlineColor(companyColors[i]);
            }
            else if (hovered)
            {
                card.setFillColor(Color(40, 60, 100, 180));
                card.setOutlineThickness(1);
                card.setOutlineColor(Color(100, 150, 200));
            }
            else
            {
                card.setFillColor(Color(25, 45, 80, 150));
                card.setOutlineThickness(1);
                card.setOutlineColor(Color(50, 80, 120));
            }
            window.draw(card);

            
            RectangleShape colorBar(Vector2f(6, 42));
            colorBar.setFillColor(companyColors[i]);
            colorBar.setPosition(45, yPos);
            window.draw(colorBar);

            
            RectangleShape checkbox(Vector2f(28, 28));
            checkbox.setPosition(60, yPos + 7);
            checkbox.setOutlineThickness(2);

            if (isSelected)
            {
                float pulse = 1.0f + 0.1f * sin(time * 5);
                checkbox.setScale(pulse, pulse);
                checkbox.setFillColor(Color(50, 180, 100));
                checkbox.setOutlineColor(Color(100, 255, 150));
            }
            else
            {
                checkbox.setFillColor(Color(40, 60, 90));
                checkbox.setOutlineColor(hovered ? Color(150, 200, 255) : Color(70, 100, 140));
            }
            window.draw(checkbox);

            if (isSelected)
            {
                Text checkmark("*", font, 22);
                checkmark.setFillColor(Color::White);
                checkmark.setStyle(Text::Bold);
                checkmark.setPosition(64, yPos + 5);
                window.draw(checkmark);
            }

            
            Text companyName(companies[i], font, 20);
            companyName.setFillColor(isSelected ? Color(255, 255, 255) : (hovered ? Color(255, 255, 200) : Color(200, 210, 230)));
            companyName.setPosition(100, yPos + 10);
            window.draw(companyName);

            
            if (isSelected)
            {
                Text status("SELECTED", font, 11);
                status.setFillColor(Color(100, 255, 150));
                status.setPosition(520, yPos + 15);
                window.draw(status);
            }

            
            if (hovered && Mouse::isButtonPressed(Mouse::Button::Left) && !companyClicked)
            {
                if (isSelected)
                    shipPrefs.removePreferredCompany(companies[i]);
                else
                    shipPrefs.addPreferredCompany(companies[i]);
                companyClicked = true;
            }
        }
        if (!Mouse::isButtonPressed(Mouse::Button::Left))
            companyClicked = false;

        
        RectangleShape clearCompBtn(Vector2f(180, 38));
        clearCompBtn.setPosition(230, 815);
        bool clearCompHover = clearCompBtn.getGlobalBounds().contains(mouse);

        if (clearCompHover)
        {
            clearCompBtn.setFillColor(Color(200, 80, 80));
            clearCompBtn.setOutlineColor(Color(255, 150, 150));
        }
        else
        {
            clearCompBtn.setFillColor(Color(120, 50, 50));
            clearCompBtn.setOutlineColor(Color(180, 100, 100));
        }
        clearCompBtn.setOutlineThickness(2);
        window.draw(clearCompBtn);

        Text clearCompText("✕ Clear All", font, 16);
        clearCompText.setFillColor(Color::White);
        clearCompText.setPosition(270, 822);
        window.draw(clearCompText);

        static bool clearCompClicked = false;
        if (clearCompHover && Mouse::isButtonPressed(Mouse::Button::Left) && !clearCompClicked)
        {
            shipPrefs.clearCompanyPreferences();
            clearCompClicked = true;
        }
        if (!Mouse::isButtonPressed(Mouse::Button::Left))
            clearCompClicked = false;

        
        RectangleShape col2Shadow(Vector2f(590, 710));
        col2Shadow.setFillColor(Color(0, 0, 0, 80));
        col2Shadow.setPosition(675, 155);
        window.draw(col2Shadow);

        RectangleShape col2Bg(Vector2f(580, 700));
        col2Bg.setFillColor(Color(50, 20, 25, 240));
        col2Bg.setOutlineThickness(3);
        col2Bg.setOutlineColor(Color(255, 80, 80));
        col2Bg.setPosition(670, 150);
        window.draw(col2Bg);

        
        RectangleShape col2Header(Vector2f(580, 55));
        col2Header.setFillColor(Color(120, 40, 50, 200));
        col2Header.setPosition(670, 150);
        window.draw(col2Header);

        
        float warningPulse = 0.8f + 0.2f * sin(time * 4);
        Text warningIcon("⚠", font, 28);
        warningIcon.setFillColor(Color(255, 200, 50, 200 * warningPulse));
        warningIcon.setPosition(685, 160);
        window.draw(warningIcon);

        Text col2Title("PORTS TO AVOID", font, 22);
        col2Title.setStyle(Text::Bold);
        col2Title.setFillColor(Color(255, 150, 120));
        col2Title.setPosition(730, 165);
        window.draw(col2Title);

        Text col2Hint("Mark storm-affected or restricted ports", font, 13);
        col2Hint.setFillColor(Color(220, 150, 150));
        col2Hint.setPosition(690, 210);
        window.draw(col2Hint);

        
        int avoidedCount = shipPrefs.avoidedPorts.getSize();
        if (avoidedCount > 0)
        {
            CircleShape avoidBadge(15);
            avoidBadge.setFillColor(Color(220, 80, 80));
            avoidBadge.setPosition(1190, 160);
            window.draw(avoidBadge);

            Text avoidBadgeText(to_string(avoidedCount), font, 16);
            avoidBadgeText.setFillColor(Color::White);
            avoidBadgeText.setStyle(Text::Bold);
            avoidBadgeText.setPosition(avoidedCount < 10 ? 1198 : 1194, 162);
            window.draw(avoidBadgeText);
        }

        
        float portY = 240;
        static bool portClicked = false;
        for (int i = 0; i < allPorts.getSize(); i++)
        {
            float yPos = portY + i * 38 - avoidScrollOffset;
            if (yPos < 235 || yPos > 810)
                continue;

            bool isAvoided = shipPrefs.isPortAvoided(allPorts[i]);

            FloatRect cardBounds(685, yPos, 550, 35);
            bool hovered = cardBounds.contains(mouse);

            
            RectangleShape portCard(Vector2f(550, 34));
            portCard.setPosition(685, yPos);

            if (isAvoided)
            {
                portCard.setFillColor(Color(100, 30, 30, 200));
                portCard.setOutlineThickness(2);
                portCard.setOutlineColor(Color(255, 100, 100));
            }
            else if (hovered)
            {
                portCard.setFillColor(Color(80, 50, 50, 180));
                portCard.setOutlineThickness(1);
                portCard.setOutlineColor(Color(200, 120, 120));
            }
            else
            {
                portCard.setFillColor(Color(60, 35, 35, 150));
                portCard.setOutlineThickness(1);
                portCard.setOutlineColor(Color(100, 60, 60));
            }
            window.draw(portCard);

            
            RectangleShape checkbox(Vector2f(24, 24));
            checkbox.setPosition(695, yPos + 5);
            checkbox.setOutlineThickness(2);

            if (isAvoided)
            {
                checkbox.setFillColor(Color(200, 60, 60));
                checkbox.setOutlineColor(Color(255, 120, 120));

                
                RectangleShape strike(Vector2f(540, 2));
                strike.setFillColor(Color(255, 100, 100, 100));
                strike.setPosition(690, yPos + 16);
                window.draw(strike);
            }
            else
            {
                checkbox.setFillColor(Color(60, 40, 40));
                checkbox.setOutlineColor(hovered ? Color(200, 150, 150) : Color(100, 70, 70));
            }
            window.draw(checkbox);

            if (isAvoided)
            {
                Text xmark("X", font, 18);
                xmark.setFillColor(Color::White);
                xmark.setStyle(Text::Bold);
                xmark.setPosition(699, yPos + 3);
                window.draw(xmark);
            }

            
            Text portIcon(isAvoided ? "🚫" : "📍", font, 14);
            portIcon.setPosition(730, yPos + 7);
            window.draw(portIcon);

            Text portName(allPorts[i], font, 17);
            portName.setFillColor(isAvoided ? Color(255, 150, 150) : (hovered ? Color(255, 220, 200) : Color(210, 190, 190)));
            portName.setPosition(755, yPos + 7);
            window.draw(portName);

            
            if (isAvoided)
            {
                Text avoidLabel("BLOCKED", font, 10);
                avoidLabel.setFillColor(Color(255, 100, 100));
                avoidLabel.setPosition(1170, yPos + 10);
                window.draw(avoidLabel);
            }

            
            if (hovered && Mouse::isButtonPressed(Mouse::Button::Left) && !portClicked)
            {
                if (isAvoided)
                    shipPrefs.removeAvoidedPort(allPorts[i]);
                else
                    shipPrefs.addAvoidedPort(allPorts[i]);
                portClicked = true;
            }
        }
        if (!Mouse::isButtonPressed(Mouse::Button::Left))
            portClicked = false;

        
        RectangleShape clearPortBtn(Vector2f(180, 38));
        clearPortBtn.setPosition(870, 815);
        bool clearPortHover = clearPortBtn.getGlobalBounds().contains(mouse);

        if (clearPortHover)
        {
            clearPortBtn.setFillColor(Color(200, 80, 80));
            clearPortBtn.setOutlineColor(Color(255, 150, 150));
        }
        else
        {
            clearPortBtn.setFillColor(Color(120, 50, 50));
            clearPortBtn.setOutlineColor(Color(180, 100, 100));
        }
        clearPortBtn.setOutlineThickness(2);
        window.draw(clearPortBtn);

        Text clearPortText("✕ Clear All", font, 16);
        clearPortText.setFillColor(Color::White);
        clearPortText.setPosition(915, 822);
        window.draw(clearPortText);

        static bool clearPortClicked = false;
        if (clearPortHover && Mouse::isButtonPressed(Mouse::Button::Left) && !clearPortClicked)
        {
            shipPrefs.clearAvoidedPorts();
            clearPortClicked = true;
        }
        if (!Mouse::isButtonPressed(Mouse::Button::Left))
            clearPortClicked = false;

        
        RectangleShape col3Shadow(Vector2f(650, 710));
        col3Shadow.setFillColor(Color(0, 0, 0, 80));
        col3Shadow.setPosition(1265, 155);
        window.draw(col3Shadow);

        RectangleShape col3Bg(Vector2f(640, 700));
        col3Bg.setFillColor(Color(25, 35, 55, 240));
        col3Bg.setOutlineThickness(3);
        col3Bg.setOutlineColor(Color(255, 180, 50));
        col3Bg.setPosition(1260, 150);
        window.draw(col3Bg);

        
        RectangleShape col3Header(Vector2f(640, 55));
        col3Header.setFillColor(Color(100, 80, 30, 200));
        col3Header.setPosition(1260, 150);
        window.draw(col3Header);

        
        Text clockIcon("⏱", font, 28);
        clockIcon.setFillColor(Color(255, 220, 100));
        clockIcon.setPosition(1275, 160);
        window.draw(clockIcon);

        Text col3Title("VOYAGE SETTINGS", font, 22);
        col3Title.setStyle(Text::Bold);
        col3Title.setFillColor(Color(255, 220, 100));
        col3Title.setPosition(1320, 165);
        window.draw(col3Title);

        
        RectangleShape timeSection(Vector2f(600, 120));
        timeSection.setFillColor(Color(35, 50, 80, 200));
        timeSection.setOutlineThickness(2);
        timeSection.setOutlineColor(Color(100, 130, 180));
        timeSection.setPosition(1280, 220);
        window.draw(timeSection);

        Text timeSectionTitle("Maximum Voyage Duration", font, 18);
        timeSectionTitle.setStyle(Text::Bold);
        timeSectionTitle.setFillColor(Color(180, 200, 255));
        timeSectionTitle.setPosition(1300, 230);
        window.draw(timeSectionTitle);

        
        RectangleShape timeInputBg(Vector2f(120, 50));
        timeInputBg.setPosition(1300, 265);
        timeInputBg.setOutlineThickness(3);

        FloatRect timeInputBounds = timeInputBg.getGlobalBounds();
        bool timeHovered = timeInputBounds.contains(mouse);

        if (maxTimeActive)
        {
            timeInputBg.setFillColor(Color(50, 70, 120));
            timeInputBg.setOutlineColor(Color(255, 200, 50));
        }
        else
        {
            timeInputBg.setFillColor(Color(40, 55, 90));
            timeInputBg.setOutlineColor(timeHovered ? Color(180, 160, 80) : Color(80, 100, 140));
        }
        window.draw(timeInputBg);

        
        static bool timeInputClicked = false;
        if (timeHovered && Mouse::isButtonPressed(Mouse::Button::Left) && !timeInputClicked)
        {
            maxTimeActive = true;
            timeInputClicked = true;
        }
        else if (!timeHovered && Mouse::isButtonPressed(Mouse::Button::Left))
        {
            maxTimeActive = false;
        }
        if (!Mouse::isButtonPressed(Mouse::Button::Left))
            timeInputClicked = false;

        Text timeText(maxTimeInput.empty() ? "0" : maxTimeInput, font, 28);
        timeText.setFillColor(maxTimeInput.empty() ? Color(120, 120, 120) : Color(255, 255, 255));
        timeText.setStyle(Text::Bold);
        timeText.setPosition(1330, 272);
        window.draw(timeText);

        Text hoursLabel("hours", font, 18);
        hoursLabel.setFillColor(Color(150, 170, 200));
        hoursLabel.setPosition(1430, 280);
        window.draw(hoursLabel);

        Text timeHint("(Enter 0 for unlimited)", font, 13);
        timeHint.setFillColor(Color(130, 140, 160));
        timeHint.setPosition(1300, 320);
        window.draw(timeHint);

        
        RectangleShape applyTimeBtn(Vector2f(130, 45));
        applyTimeBtn.setPosition(1730, 265);
        bool applyTimeHover = applyTimeBtn.getGlobalBounds().contains(mouse);

        if (applyTimeHover)
        {
            applyTimeBtn.setFillColor(Color(80, 180, 100));
            applyTimeBtn.setOutlineColor(Color(150, 255, 170));
        }
        else
        {
            applyTimeBtn.setFillColor(Color(60, 140, 80));
            applyTimeBtn.setOutlineColor(Color(100, 200, 130));
        }
        applyTimeBtn.setOutlineThickness(2);
        window.draw(applyTimeBtn);

        Text applyTimeText("Apply", font, 18);
        applyTimeText.setFillColor(Color::White);
        applyTimeText.setPosition(1755, 275);
        window.draw(applyTimeText);

        static bool applyTimeClicked = false;
        if (applyTimeHover && Mouse::isButtonPressed(Mouse::Button::Left) && !applyTimeClicked)
        {
            int hours = maxTimeInput.empty() ? 0 : stoi(maxTimeInput);
            shipPrefs.setMaxVoyageTime(hours);
            applyTimeClicked = true;
        }
        if (!Mouse::isButtonPressed(Mouse::Button::Left))
            applyTimeClicked = false;

        
        RectangleShape summaryBg(Vector2f(600, 330));
        summaryBg.setFillColor(Color(20, 30, 50, 220));
        summaryBg.setOutlineThickness(2);
        summaryBg.setOutlineColor(Color(80, 120, 180));
        summaryBg.setPosition(1280, 360);
        window.draw(summaryBg);

        
        RectangleShape summaryHeader(Vector2f(600, 40));
        summaryHeader.setFillColor(Color(40, 60, 100, 200));
        summaryHeader.setPosition(1280, 360);
        window.draw(summaryHeader);

        Text summaryIcon("#", font, 22);
        summaryIcon.setPosition(1295, 365);
        window.draw(summaryIcon);

        Text summaryTitle("ACTIVE PREFERENCES", font, 18);
        summaryTitle.setStyle(Text::Bold);
        summaryTitle.setFillColor(Color(150, 200, 255));
        summaryTitle.setPosition(1330, 370);
        window.draw(summaryTitle);

        
        float summaryY = 415;

        
        Text compIcon(">", font, 16);
        compIcon.setPosition(1300, summaryY);
        window.draw(compIcon);

        if (shipPrefs.preferredCompanies.getSize() > 0)
        {
            Text compLabel("Preferred Companies:", font, 15);
            compLabel.setFillColor(Color(100, 220, 180));
            compLabel.setPosition(1330, summaryY);
            window.draw(compLabel);
            summaryY += 25;

            string compList = "";
            for (int i = 0; i < shipPrefs.preferredCompanies.getSize() && i < 5; i++)
            {
                compList += "• " + shipPrefs.preferredCompanies[i] + "  ";
            }
            if (shipPrefs.preferredCompanies.getSize() > 5)
            {
                compList += "+" + to_string(shipPrefs.preferredCompanies.getSize() - 5) + " more";
            }
            Text compNames(compList, font, 13);
            compNames.setFillColor(Color(180, 220, 200));
            compNames.setPosition(1310, summaryY);
            window.draw(compNames);
            summaryY += 30;
        }
        else
        {
            Text compLabel("Companies: All (no filter)", font, 15);
            compLabel.setFillColor(Color(140, 140, 160));
            compLabel.setPosition(1330, summaryY);
            window.draw(compLabel);
            summaryY += 35;
        }

        
        Text avoidIcon("⚠", font, 16);
        avoidIcon.setPosition(1300, summaryY);
        window.draw(avoidIcon);

        if (shipPrefs.avoidedPorts.getSize() > 0)
        {
            Text avoidLabel("Ports to Avoid:", font, 15);
            avoidLabel.setFillColor(Color(255, 150, 100));
            avoidLabel.setPosition(1330, summaryY);
            window.draw(avoidLabel);
            summaryY += 25;

            string avoidList = "";
            for (int i = 0; i < shipPrefs.avoidedPorts.getSize() && i < 5; i++)
            {
                avoidList += "• " + shipPrefs.avoidedPorts[i] + "  ";
            }
            if (shipPrefs.avoidedPorts.getSize() > 5)
            {
                avoidList += "+" + to_string(shipPrefs.avoidedPorts.getSize() - 5) + " more";
            }
            Text avoidNames(avoidList, font, 13);
            avoidNames.setFillColor(Color(255, 200, 180));
            avoidNames.setPosition(1310, summaryY);
            window.draw(avoidNames);
            summaryY += 30;
        }
        else
        {
            Text avoidLabel("Avoided Ports: None", font, 15);
            avoidLabel.setFillColor(Color(140, 140, 160));
            avoidLabel.setPosition(1330, summaryY);
            window.draw(avoidLabel);
            summaryY += 35;
        }

        
        Text timeIcon("⏱", font, 16);
        timeIcon.setPosition(1300, summaryY);
        window.draw(timeIcon);

        if (shipPrefs.maxVoyageMinutes > 0)
        {
            int hours = shipPrefs.maxVoyageMinutes / 60;
            Text timeLimit("Time Limit: " + to_string(hours) + " hours", font, 15);
            timeLimit.setFillColor(Color(255, 220, 100));
            timeLimit.setPosition(1330, summaryY);
            window.draw(timeLimit);
        }
        else
        {
            Text timeLimit("Time Limit: Unlimited", font, 15);
            timeLimit.setFillColor(Color(140, 140, 160));
            timeLimit.setPosition(1330, summaryY);
            window.draw(timeLimit);
        }
        summaryY += 40;

        
        RectangleShape statusBar(Vector2f(580, 35));
        statusBar.setPosition(1290, summaryY);

        if (shipPrefs.hasActiveFilters())
        {
            statusBar.setFillColor(Color(40, 100, 60, 200));
            statusBar.setOutlineColor(Color(80, 180, 100));
            statusBar.setOutlineThickness(1);
            window.draw(statusBar);

            Text statusText("[ON] FILTERS ACTIVE - Routes will be filtered", font, 14);
            statusText.setFillColor(Color(150, 255, 180));
            statusText.setPosition(1320, summaryY + 8);
            window.draw(statusText);
        }
        else
        {
            statusBar.setFillColor(Color(60, 60, 80, 200));
            statusBar.setOutlineColor(Color(100, 100, 130));
            statusBar.setOutlineThickness(1);
            window.draw(statusBar);

            Text statusText("○ NO FILTERS - All routes available", font, 14);
            statusText.setFillColor(Color(150, 150, 170));
            statusText.setPosition(1340, summaryY + 8);
            window.draw(statusText);
        }

        

        
        RectangleShape clearAllBtn(Vector2f(280, 55));
        clearAllBtn.setPosition(1440, 720);
        bool clearAllHover = clearAllBtn.getGlobalBounds().contains(mouse);

        if (clearAllHover)
        {
            clearAllBtn.setFillColor(Color(200, 60, 60));
            clearAllBtn.setOutlineColor(Color(255, 120, 120));
        }
        else
        {
            clearAllBtn.setFillColor(Color(150, 50, 50));
            clearAllBtn.setOutlineColor(Color(200, 90, 90));
        }
        clearAllBtn.setOutlineThickness(3);
        window.draw(clearAllBtn);

        Text clearAllIcon("🗑", font, 22);
        clearAllIcon.setPosition(1460, 730);
        window.draw(clearAllIcon);

        Text clearAllText("RESET ALL FILTERS", font, 18);
        clearAllText.setStyle(Text::Bold);
        clearAllText.setFillColor(Color::White);
        clearAllText.setPosition(1500, 735);
        window.draw(clearAllText);

        static bool clearAllClicked = false;
        if (clearAllHover && Mouse::isButtonPressed(Mouse::Button::Left) && !clearAllClicked)
        {
            shipPrefs.clearAllPreferences();
            maxTimeInput = "";
            clearAllClicked = true;
        }
        if (!Mouse::isButtonPressed(Mouse::Button::Left))
            clearAllClicked = false;

        
        RectangleShape backBtnShadow(Vector2f(165, 55));
        backBtnShadow.setFillColor(Color(0, 0, 0, 80));
        backBtnShadow.setPosition(35, 875);
        window.draw(backBtnShadow);

        RectangleShape backBtn(Vector2f(160, 50));
        backBtn.setPosition(30, 870);
        bool backHover = backBtn.getGlobalBounds().contains(mouse);

        if (backHover)
        {
            backBtn.setFillColor(Color(80, 100, 180));
            backBtn.setOutlineColor(Color(150, 180, 255));
        }
        else
        {
            backBtn.setFillColor(Color(50, 70, 130));
            backBtn.setOutlineColor(Color(100, 130, 200));
        }
        backBtn.setOutlineThickness(2);
        window.draw(backBtn);

        Text backText("← BACK", font, 22);
        backText.setFillColor(Color::White);
        backText.setStyle(Text::Bold);
        backText.setPosition(60, 880);
        window.draw(backText);

        if (backHover && Mouse::isButtonPressed(Mouse::Button::Left))
        {
            window.close();
        }

        
        Text instruction("Scroll to see more items | Press ESC to return", font, 16);
        instruction.setFillColor(Color(150, 180, 200));
        instruction.setPosition(750, 1020);
        window.draw(instruction);

        window.display();
    }
}

int main()
{
    Font font;
    if (!font.loadFromFile("Roboto.ttf"))
    {
        cout << "Font loading failed!" << endl;
        return -1;
    }

    
    if (!clickSoundBuffer.loadFromFile("click.wav"))
    {
        cout << "Warning: Could not load click.wav" << endl;
    }
    clickSound.setBuffer(clickSoundBuffer);
    clickSound.setVolume(50);

    
    if (!transitionSoundBuffer.loadFromFile("transition.wav"))
    {
        cout << "Warning: Could not load transition.wav" << endl;
    }
    transitionSound.setBuffer(transitionSoundBuffer);
    transitionSound.setVolume(60);
    
    
    
    if (!hornSoundBuffer.loadFromFile("ship.wav"))
    {
        cout << "Note: ship.wav not found - using transition sound for horn" << endl;
        
        hornSound.setBuffer(transitionSoundBuffer);
    }
    else
    {
        hornSound.setBuffer(hornSoundBuffer);
    }
    hornSound.setVolume(70);

    Maps graph = readPorts("PortCharges.txt");
    addRoutesFromFile(graph, "Routes.txt");

    bookingManager.loadFromFile();

    while (true)
    {
        int choice = openMainMenu(font);

        if (choice == 1)
        {
            graph.showMap(graph);
        }
        else if (choice == 2)
        {
            bookingInterface(graph, font);
        }
        else if (choice == 3)
        {
            viewBookings(font);
        }
        else if (choice == 4)
        {
            showMultiLegRouteBuilder(graph, font);
        }
        else if (choice == 5)
        {
            exit(0);
        }
    }

    return 0;
}

