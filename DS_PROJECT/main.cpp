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
// #include "Dijkstra.h"
using namespace sf;
using namespace std;

class RouteNode
{
public:
    string startingPort;
    string destinationPort;
    string departureDate;
    string departureTime;
    string arrivalTime;
    int handlingMinutes;
    int cost;
    string shippingCompany;
    RouteNode *next;

    RouteNode(string start, string des, string depDate, string depTime, string arrTime, int cost, string sCompany)
    {
        startingPort = start;
        destinationPort = des;
        departureDate = depDate;
        departureTime = depTime;
        arrivalTime = arrTime;
        this->cost = cost;
        shippingCompany = sCompany;
        next = nullptr;
    }
};

class PortNode
{
public:
    string portName;
    int charge;
    RouteNode *routeHead;

    PortNode()
    {
        portName = "";
        charge = 0;
        routeHead = nullptr;
    }

    ~PortNode()
    {
        RouteNode *current = routeHead;
        while (current != nullptr)
        {
            RouteNode *next = current->next;
            delete current;
            current = next;
        }
        routeHead = nullptr;
    }
};

struct PortLocation
{
    string name;
    float x;
    float y;
};

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

class InputBox
{
public:
    RectangleShape box;
    Text text;
    string inputString;
    bool isActive;
    string label;
    Text labelText;

    InputBox(Font &font, string lbl, Vector2f position)
    {
        label = lbl;
        box.setSize(Vector2f(300, 40));
        box.setFillColor(Color(0, 20, 60, 240));
        box.setOutlineThickness(2);
        box.setOutlineColor(Color(100, 200, 255));
        box.setPosition(position);

        labelText.setFont(font);
        labelText.setString(label + ":");
        labelText.setCharacterSize(16);
        labelText.setFillColor(Color(200, 230, 255));
        labelText.setPosition(position.x, position.y - 25);

        text.setFont(font);
        text.setCharacterSize(18);
        text.setFillColor(Color::White);
        text.setPosition(position.x + 10, position.y + 8);

        inputString = "";
        isActive = false;
    }

    void handleEvent(Event &event, RenderWindow &window)
    {
        if (event.type == Event::MouseButtonPressed)
        {
            Vector2f mousePos = window.mapPixelToCoords(Mouse::getPosition(window));
            if (box.getGlobalBounds().contains(mousePos))
            {
                isActive = true;
                box.setOutlineColor(Color::Yellow);
            }
            else
            {
                isActive = false;
                box.setOutlineColor(Color(100, 200, 255));
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
            else if (event.text.unicode < 128 && event.text.unicode != '\r' && event.text.unicode != '\t')
            {
                inputString += static_cast<char>(event.text.unicode);
            }
            text.setString(inputString);
        }
    }

    void draw(RenderWindow &window)
    {
        window.draw(labelText);
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

class Maps
{
public:
    Vector<PortNode> ports;
    // Dijkstra dijkstra;

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
        if (date.length() == 10 && date[2] == '/' && date[5] == '/')
        {
            return date;
        }

        size_t firstSlash = date.find('/');
        size_t secondSlash = date.find('/', firstSlash + 1);

        if (firstSlash != string::npos && secondSlash != string::npos)
        {
            string dayStr = date.substr(0, firstSlash);
            string monthStr = date.substr(firstSlash + 1, secondSlash - firstSlash - 1);
            string yearStr = date.substr(secondSlash + 1);

            if (dayStr.length() == 1)
                dayStr = "0" + dayStr;
            if (monthStr.length() == 1)
                monthStr = "0" + monthStr;

            return dayStr + "/" + monthStr + "/" + yearStr;
        }

        return date;
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

    bool findAndDisplayRoute(string origin, string destination, string date,
                             Vector<int> &currentPath, Vector<string> &waitPorts,
                             Vector<string> &waitDurations, string &resultText, Color &resultColor, string &errorDetails)
    {
        currentPath.clear();
        waitPorts.clear();
        waitDurations.clear();
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

        Vector<int> directPath;

        bool directFound = checkDirectRoutes(origin, destination, normalizedDate, directPath);
        bool connectingFound = checkConnectingRoutes(origin, destination, normalizedDate, currentPath, waitPorts, waitDurations);
        if (directFound && connectingFound)
        {
            resultText = "Both direct and connecting routes found!";
            resultColor = Color::Green;
            errorDetails = "Showing connecting route with " + to_string(waitPorts.getSize()) + " wait points.";
            return true;
        }
        else if (directFound)
        {
            currentPath = directPath;
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

    //* Global variable to track menu selection
    int menuSelection = 0;

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

    void drawMainMenu(RenderWindow &window, Font &font)
    {
        float centerX = 960;
        Vector2i mousePos = Mouse::getPosition(window);
        Vector2f mouse(mousePos.x, mousePos.y);

        static Clock animClock;
        float time = animClock.getElapsedTime().asSeconds();

        // ===== ANIMATED PARTICLES BACKGROUND =====
        static Vector<CircleShape> particles;
        static bool particlesInitialized = false;
        
        if (!particlesInitialized)
        {
            for (int i = 0; i < 60; i++)
            {
                CircleShape particle(rand() % 4 + 1);
                particle.setFillColor(Color(30 + rand() % 100, 150 + rand() % 100, 255, 20 + rand() % 60));
                particle.setPosition(rand() % 1920, rand() % 1080);
                particles.push_back(particle);
            }
            particlesInitialized = true;
        }

        for (int i = 0; i < particles.getSize(); i++)
        {
            float drift = sin(time * 0.3f + i) * 0.8f;
            float rise = time * 15.0f + i * 8.0f;
            
            CircleShape p = particles[i];
            float yPos = fmod(rise, 1400.0f) - 200.0f;
            float xPos = particles[i].getPosition().x + drift;
            
            p.setPosition(xPos, yPos);
            
            float alphaPulse = 30 + 50 * sin(time * 2.0f + i);
            Color c = p.getFillColor();
            c.a = alphaPulse;
            p.setFillColor(c);
            
            window.draw(p);
        }

        // ===== ANIMATED BACKGROUND ORBS =====
        for (int orb = 0; orb < 3; orb++)
        {
            CircleShape orbGlow(200 + orb * 50);
            orbGlow.setFillColor(Color(0, 100 + orb * 50, 200, 15));
            
            float orbX = 300 + orb * 700 + sin(time * 0.5f + orb) * 100;
            float orbY = 400 + cos(time * 0.3f + orb * 2) * 150;
            
            orbGlow.setOrigin(200 + orb * 50, 200 + orb * 50);
            orbGlow.setPosition(orbX, orbY);
            window.draw(orbGlow);
        }

        // ===== MAIN TITLE WITH ADVANCED EFFECTS =====
        Text title("OceanRoute Navigator", font, 110);
        title.setStyle(Text::Bold);

        float glowPulse = sin(time * 2.2f) * 0.5f + 0.5f;
        float titleAlpha = 200 + 55 * glowPulse;

        int titleR = 0;
        int titleG = 180 + static_cast<int>(80 * glowPulse);
        int titleB = 255;

        title.setFillColor(Color(titleR, titleG, titleB));
        title.setOutlineThickness(3);
        title.setOutlineColor(Color(0, 255, 255, static_cast<int>(titleAlpha)));

        FloatRect tb = title.getLocalBounds();
        title.setOrigin(tb.width / 2, tb.height / 2);

        float floatOffset = sin(time * 1.5f) * 12.0f;
        title.setPosition(centerX, 140 + floatOffset);

        // Triple glow layers
        Text glow1 = title;
        glow1.setOutlineThickness(15);
        glow1.setOutlineColor(Color(0, 255, 255, static_cast<int>(30 * glowPulse)));
        glow1.setFillColor(Color::Transparent);
        window.draw(glow1);

        Text glow2 = title;
        glow2.setOutlineThickness(10);
        glow2.setOutlineColor(Color(0, 200, 255, static_cast<int>(60 * glowPulse)));
        glow2.setFillColor(Color::Transparent);
        window.draw(glow2);

        Text glow3 = title;
        glow3.setOutlineThickness(5);
        glow3.setOutlineColor(Color(100, 255, 255, static_cast<int>(100 * glowPulse)));
        glow3.setFillColor(Color::Transparent);
        window.draw(glow3);

        window.draw(title);

        // ===== LOAD BUTTON TEXTURE =====
        Texture btnTex;
        if (!btnTex.loadFromFile("ButtonMainMenu.png"))
            cout << "Error loading ButtonMainMenu.png" << endl;

        Sprite buttonSprite(btnTex);

        // ===== ENHANCED BUTTON MAKER WITH ALL EFFECTS =====
        auto makeButton = [&](string txt, float yPos, bool &hovered, Color btnColor, int btnId)
        {
            FloatRect spriteBounds(
                centerX - 300,
                yPos - 70,
                600, 140
            );

            if (spriteBounds.contains(mouse))
                hovered = true;
            else
                hovered = false;

            Sprite icon = buttonSprite;

            float scalX = 600.0f / icon.getLocalBounds().width;
            float scalY = 140.0f / icon.getLocalBounds().height;
            icon.setScale(scalX, scalY);

            icon.setOrigin(icon.getLocalBounds().width / 2, icon.getLocalBounds().height / 2);
            icon.setPosition(centerX, yPos);

            // ===== HOVER ANIMATIONS =====
            if (hovered)
            {
                icon.setColor(Color(230, 250, 255));
                float scaleBoost = 1.12f;
                icon.setScale(scalX * scaleBoost, scalY * scaleBoost);

                // Multi-layer glow on hover
                for (int g = 0; g < 3; g++)
                {
                    CircleShape hoverGlow(150.0f - g * 30);
                    hoverGlow.setOrigin(150.0f - g * 30, 150.0f - g * 30);
                    float glowAlpha = 80 - g * 20;
                    hoverGlow.setFillColor(Color(100, 200 + g * 30, 255, glowAlpha));
                    hoverGlow.setPosition(centerX, yPos);
                    window.draw(hoverGlow);
                }

                // Pulsing ring
                CircleShape pulseRing(160.0f);
                pulseRing.setOrigin(160, 160);
                float pulseAlpha = 100 * sin(time * 6.0f);
                pulseRing.setFillColor(Color::Transparent);
                pulseRing.setOutlineThickness(3);
                pulseRing.setOutlineColor(Color(0, 255, 200, static_cast<int>(pulseAlpha)));
                pulseRing.setPosition(centerX, yPos);
                window.draw(pulseRing);
            }
            else
            {
                icon.setColor(Color(180, 210, 235));
            }

            // ===== ENHANCED SHADOWS =====
            Sprite shadowDeep = icon;
            shadowDeep.setColor(Color(0, 0, 0, 200));
            shadowDeep.setPosition(icon.getPosition().x + 12, icon.getPosition().y + 12);
            window.draw(shadowDeep);

            Sprite shadowMid = icon;
            shadowMid.setColor(Color(0, 0, 0, 140));
            shadowMid.setPosition(icon.getPosition().x + 8, icon.getPosition().y + 8);
            window.draw(shadowMid);

            Sprite shadowLight = icon;
            shadowLight.setColor(Color(0, 0, 0, 70));
            shadowLight.setPosition(icon.getPosition().x + 4, icon.getPosition().y + 4);
            window.draw(shadowLight);

            window.draw(icon);

            // ===== BUTTON TEXT WITH EFFECTS =====
            Text label(txt, font, 52);
            label.setStyle(Text::Bold);
            label.setOutlineThickness(4.f);
            label.setOutlineColor(Color(0, 0, 0, 200));

            FloatRect lb = label.getLocalBounds();
            label.setOrigin(lb.width / 2, lb.height / 2);
            label.setPosition(centerX, yPos - 5);

            if (hovered)
            {
                label.setFillColor(Color(255, 255, 100));
                label.setOutlineThickness(5.f);
                label.setOutlineColor(Color(0, 0, 0, 240));
            }
            else
            {
                label.setFillColor(Color::White);
            }

            window.draw(label);

            // ===== HOVER ACCENT ELEMENTS =====
            if (hovered)
            {
                // Top accent line
                RectangleShape topLine(Vector2f(400, 4));
                topLine.setFillColor(btnColor);
                topLine.setPosition(centerX - 200, yPos - 90);
                window.draw(topLine);

                // Bottom accent line
                RectangleShape bottomLine(Vector2f(400, 4));
                bottomLine.setFillColor(btnColor);
                bottomLine.setPosition(centerX - 200, yPos + 90);
                window.draw(bottomLine);

                // Side circles
                CircleShape leftCircle(10.f);
                leftCircle.setFillColor(btnColor);
                leftCircle.setPosition(centerX - 330, yPos - 10);
                window.draw(leftCircle);

                CircleShape rightCircle(10.f);
                rightCircle.setFillColor(btnColor);
                rightCircle.setPosition(centerX + 310, yPos - 10);
                window.draw(rightCircle);
            }
        };

        bool hoverMap = false, hoverBook = false, hoverExit = false, hoverView = false;

        makeButton("Show World Map", 380, hoverMap, Color(100, 200, 255), 1);
        makeButton("Book Cargo", 530, hoverBook, Color(100, 255, 150), 2);
        makeButton("View Bookings", 680, hoverView, Color(150, 200, 255), 3);
        makeButton("Exit", 830, hoverExit, Color(255, 150, 150), 4);

        // ===== DIVIDER LINE =====
        RectangleShape divider(Vector2f(800, 2));
        divider.setFillColor(Color(0, 255, 255, 100));
        divider.setPosition(centerX - 400, 920);
        window.draw(divider);

        // ===== BOTTOM DECORATIVE ELEMENTS =====
        CircleShape decorLeft(40.f);
        decorLeft.setFillColor(Color(0, 255, 255, 40));
        float decorPulse = sin(time * 2.5f) * 35.0f + 45.0f;
        decorLeft.setRadius(decorPulse);
        decorLeft.setOrigin(decorPulse, decorPulse);
        decorLeft.setPosition(80, 1030);
        window.draw(decorLeft);

        CircleShape decorRight(40.f);
        decorRight.setFillColor(Color(0, 255, 255, 40));
        decorRight.setRadius(decorPulse);
        decorRight.setOrigin(decorPulse, decorPulse);
        decorRight.setPosition(1840, 1030);
        window.draw(decorRight);


        // ===== HANDLE CLICKS =====
        static bool clicked = false;
        if (Mouse::isButtonPressed(Mouse::Left))
        {
            if (!clicked)
            {
                if (hoverMap) menuSelection = 1;
                if (hoverBook) menuSelection = 2;
                if (hoverView) menuSelection = 3;
                if (hoverExit) menuSelection = 4;
                clicked = true;
            }
        }
        else
            clicked = false;
    }

    int openMainMenu(Font &font)
    {
        RenderWindow window(VideoMode(1920, 1080), "Main Menu - OceanRoute Navigator");

        Texture bgTexture;
        if (!bgTexture.loadFromFile("mainmenu.jpg"))
            cout << "Error loading mainmenu.jpg " << endl;

        Sprite bgSprite;
        bgSprite.setTexture(bgTexture);

        bgSprite.setScale(
            1920.0f / bgTexture.getSize().x,
            1080.0f / bgTexture.getSize().y);

        while (window.isOpen())
        {
            Event e;
            while (window.pollEvent(e))
            {
                if (e.type == Event::Closed)
                    window.close();
            }

            window.clear();

            window.draw(bgSprite);
            RectangleShape overlay(sf::Vector2f(1920, 1080));
            overlay.setFillColor(Color(0, 0, 0, 80));
            window.draw(overlay);

            drawMainMenu(window, font);

            window.display();

            if (menuSelection != 0)
            {
                int chosen = menuSelection;
                menuSelection = 0;
                window.close();
                return chosen;
            }
        }
        return 3;
    }

   
    // Dijkstra methods
    void reversePath(Vector<int>& temp)
    {
        int left = 0;
        int right = temp.getSize() - 1;
        while (left < right)
        {
            int t = temp[left];
            temp[left] = temp[right];
            temp[right] = t;
            left++;
            right--;
        }
    }

    int toMinutes(string t)
    {
        int h = (t[0] - '0') * 10 + (t[1] - '0');
        int m = (t[3] - '0') * 10 + (t[4] - '0');
        return h * 60 + m;
    }

    Vector<int> findCheapestPath(string origin, string destination)
    {
        int n = ports.getSize();

        Vector<float> dist;
        Vector<int> path;
        
        for (int i = 0; i < n; i++)
        {
            dist.push_back(numeric_limits<float>::infinity());
            path.push_back(-1);
        }
        PriorityQueue PQ;

        int originIndex = findPortIndex(origin);
        int destinationIndex = findPortIndex(destination);

        if (originIndex == -1 || destinationIndex == -1)
        {
            cout << "Origin or destination port not found!" << endl;
            return Vector<int>();
        }

        dist[originIndex] = 0;
        PQ.enqueue(originIndex, dist[originIndex]);

        while (!PQ.isEmpty())
        {
            PriorityQueue::Node current = PQ.front();
            PQ.dequeue();

            int currentIndex = current.index;
            float currentCost = current.cost;

            if (currentCost > dist[currentIndex])
            {
                continue;
            }

            RouteNode* temp = ports[currentIndex].routeHead;
            while (temp != nullptr)
            {
                int v = findPortIndex(temp->destinationPort);
                if (v >= 0 && v < n)
                {
                    float costV = temp->cost;

                    if (dist[currentIndex] + costV < dist[v])
                    {
                        dist[v] = dist[currentIndex] + costV;
                        path[v] = currentIndex;
                        PQ.enqueue(v, dist[v]);
                    }
                }
                temp = temp->next;
            }
        }

        Vector<int> shortest;
        int v = destinationIndex;
        int iterations = 0;
        const int MAX_ITERATIONS = n + 10;
        
        while (v != -1 && iterations < MAX_ITERATIONS)
        {
            if (v >= 0 && v < n)
            {
                shortest.push_back(v);
                v = path[v];
            }
            else
            {
                break;
            }
            iterations++;
        }
        
        reversePath(shortest);
        return shortest;
    }

    Vector<int> findFastestPath(string origin, string destination)
    {
        int n = ports.getSize();

        Vector<float> dist;
        Vector<int> path;
        
        for (int i = 0; i < n; i++)
        {
            dist.push_back(numeric_limits<float>::infinity());
            path.push_back(-1);
        }

        PriorityQueue PQ;

        int originIndex = findPortIndex(origin);
        int destinationIndex = findPortIndex(destination);

        if (originIndex == -1 || destinationIndex == -1)
        {
            cout << "Origin or destination port not found!" << endl;
            return Vector<int>();
        }

        dist[originIndex] = 0;
        PQ.enqueue(originIndex, dist[originIndex]);

        while (!PQ.isEmpty())
        {
            PriorityQueue::Node current = PQ.front();
            PQ.dequeue();

            int currentIndex = current.index;
            float currentTime = current.cost;

            if (currentTime > dist[currentIndex])
            {
                continue;
            }

            RouteNode* temp = ports[currentIndex].routeHead;

            while (temp != nullptr)
            {
                int dep = toMinutes(temp->departureTime);
                int arr = toMinutes(temp->arrivalTime);

                if (currentTime > dep)
                {
                    temp = temp->next;
                    continue;
                }

                int wait = dep - currentTime;
                int travel = arr - dep;
                int newTime = currentTime + wait + travel;

                int v = findPortIndex(temp->destinationPort);
                if (v >= 0 && v < n)
                {
                    if (newTime < dist[v])
                    {
                        dist[v] = newTime;
                        path[v] = currentIndex;
                        PQ.enqueue(v, newTime);
                    }
                }

                temp = temp->next;
            }
        }

        Vector<int> shortest;
        int v = destinationIndex;
        int iterations = 0;
        const int MAX_ITERATIONS = n + 10;
        
        while (v != -1 && iterations < MAX_ITERATIONS)
        {
            if (v >= 0 && v < n)
            {
                shortest.push_back(v);
                v = path[v];
            }
            else
            {
                break;
            }
            iterations++;
        }
        
        reversePath(shortest);
        return shortest;
    }

    // Calculate Euclidean distance 
    float calculateEuclideanDistance(float startX, float startY, float endX, float endY)
    {
        float horizontalDifference = endX - startX;
        float verticalDifference = endY - startY;
        return sqrt(horizontalDifference * horizontalDifference + verticalDifference * verticalDifference) * 0.001f;
    }

    int convertTimeToMinutes(string timeString)
    {
        int hourValue = (timeString[0] - '0') * 10 + (timeString[1] - '0');
        int minuteValue = (timeString[3] - '0') * 10 + (timeString[4] - '0');
        return hourValue * 60 + minuteValue;
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

    RouteNode* findRouteBetweenPorts(Maps &graph, string fromPort, string toPort)
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

    int calculateTotalTime(Maps &graph, Vector<int> path, PortLocation* portLocations)
    {
        int totalTime = 0;
        for (int i = 0; i < path.getSize() - 1; i++) {
            string fromPort = portLocations[path[i]].name;
            string toPort = portLocations[path[i + 1]].name;
            RouteNode* route = findRouteBetweenPorts(graph, fromPort, toPort);
            if (route != nullptr) {
                totalTime += graph.calculateJourneyTime(route->departureTime, route->arrivalTime);
            }
        }
        return totalTime;
    }

    string formatTime(int totalMinutes)
    {
        int days = totalMinutes / (24 * 60);
        int hours = (totalMinutes % (24 * 60)) / 60;
        int minutes = totalMinutes % 60;
        
        if (days > 0) {
            return to_string(days) + "d " + to_string(hours) + "h " + to_string(minutes) + "m";
        } else {
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

    // A* for cheapest path using cost 
    Vector<int> findCheapestPathUsingAStar(const Vector<pair<string, pair<float, float>>>& portCoordinates, 
                                           string departurePort, 
                                           string arrivalPort)
    {
        int numberOfPorts = ports.getSize();
        Vector<float> costFromStart;
        Vector<float> estimatedTotalCost;
        Vector<int> parentNodeInPath;
        Vector<bool> visitedPortsList;

        // Initialize all ports
        for (int portCounter = 0; portCounter < numberOfPorts; portCounter++)
        {
            costFromStart.push_back(numeric_limits<float>::infinity());
            estimatedTotalCost.push_back(numeric_limits<float>::infinity());
            parentNodeInPath.push_back(-1);
            visitedPortsList.push_back(false);
        }

        int startPortIndex = findPortIndex(departurePort);
        int endPortIndex = findPortIndex(arrivalPort);

        if (startPortIndex == -1 || endPortIndex == -1)
        {
            cout << "Starting or ending port not found!" << endl;
            return Vector<int>();
        }

        costFromStart[startPortIndex] = 0;
        
        float destinationXCoordinate = portCoordinates[endPortIndex].second.first;
        float destinationYCoordinate = portCoordinates[endPortIndex].second.second;
        
        estimatedTotalCost[startPortIndex] = calculateEuclideanDistance(
            portCoordinates[startPortIndex].second.first, 
            portCoordinates[startPortIndex].second.second, 
            destinationXCoordinate, 
            destinationYCoordinate);

        PriorityQueue openList;
        openList.enqueue(startPortIndex, estimatedTotalCost[startPortIndex]);

        int iterationCount = 0;
        const int MAXIMUM_ALLOWED_ITERATIONS = numberOfPorts * numberOfPorts;

        while (!openList.isEmpty() && iterationCount < MAXIMUM_ALLOWED_ITERATIONS)
        {
            iterationCount++;
            PriorityQueue::Node currentNodeData = openList.front();
            openList.dequeue();

            int currentPortIndex = currentNodeData.index;

            if (currentPortIndex == endPortIndex)
            {
                Vector<int> optimumPath;
                int nodeTraversal = endPortIndex;
                while (nodeTraversal != -1)
                {
                    optimumPath.push_back(nodeTraversal);
                    nodeTraversal = parentNodeInPath[nodeTraversal];
                }
                reversePath(optimumPath);
                return optimumPath;
            }

            if (visitedPortsList[currentPortIndex])
                continue;

            visitedPortsList[currentPortIndex] = true;

            RouteNode* currentPortRoute = ports[currentPortIndex].routeHead;
            while (currentPortRoute != nullptr)
            {
                int neighborPortIndex = findPortIndex(currentPortRoute->destinationPort);
                
                if (neighborPortIndex >= 0 && neighborPortIndex < numberOfPorts && !visitedPortsList[neighborPortIndex])
                {
                    float costToNeighbor = costFromStart[currentPortIndex] + currentPortRoute->cost;

                    if (costToNeighbor < costFromStart[neighborPortIndex])
                    {
                        parentNodeInPath[neighborPortIndex] = currentPortIndex;
                        costFromStart[neighborPortIndex] = costToNeighbor;

                        float heuristicEstimate = calculateEuclideanDistance(
                            portCoordinates[neighborPortIndex].second.first, 
                            portCoordinates[neighborPortIndex].second.second, 
                            destinationXCoordinate, 
                            destinationYCoordinate);
                        estimatedTotalCost[neighborPortIndex] = costFromStart[neighborPortIndex] + heuristicEstimate;

                        openList.enqueue(neighborPortIndex, estimatedTotalCost[neighborPortIndex]);
                    }
                }
                currentPortRoute = currentPortRoute->next;
            }
        }

        return Vector<int>();
    }

    // A* for fastest path using time 
    Vector<int> findFastestPathUsingAStar(const Vector<pair<string, pair<float, float>>>& portCoordinates, 
                                          string departurePort, 
                                          string arrivalPort)
    {
        int numberOfPorts = ports.getSize();
        Vector<float> timeFromStart;
        Vector<float> estimatedTotalTime;
        Vector<int> parentNodeInPath;
        Vector<bool> visitedPortsList;

        // Initialize all ports
        for (int portCounter = 0; portCounter < numberOfPorts; portCounter++)
        {
            timeFromStart.push_back(numeric_limits<float>::infinity());
            estimatedTotalTime.push_back(numeric_limits<float>::infinity());
            parentNodeInPath.push_back(-1);
            visitedPortsList.push_back(false);
        }

        int startPortIndex = findPortIndex(departurePort);
        int endPortIndex = findPortIndex(arrivalPort);

        if (startPortIndex == -1 || endPortIndex == -1)
        {
            cout << "Starting or ending port not found!" << endl;
            return Vector<int>();
        }

        timeFromStart[startPortIndex] = 0;
        
        float destinationXCoordinate = portCoordinates[endPortIndex].second.first;
        float destinationYCoordinate = portCoordinates[endPortIndex].second.second;
        
        estimatedTotalTime[startPortIndex] = calculateEuclideanDistance(
            portCoordinates[startPortIndex].second.first,
            portCoordinates[startPortIndex].second.second,
            destinationXCoordinate, 
            destinationYCoordinate);

        PriorityQueue openList;
        openList.enqueue(startPortIndex, estimatedTotalTime[startPortIndex]);

        int iterationCount = 0;
        const int MAXIMUM_ALLOWED_ITERATIONS = numberOfPorts * numberOfPorts;

        while (!openList.isEmpty() && iterationCount < MAXIMUM_ALLOWED_ITERATIONS)
        {
            iterationCount++;
            PriorityQueue::Node currentNodeData = openList.front();
            openList.dequeue();

            int currentPortIndex = currentNodeData.index;

            if (currentPortIndex == endPortIndex)
            {
                Vector<int> optimumPath;
                int nodeTraversal = endPortIndex;
                while (nodeTraversal != -1)
                {
                    optimumPath.push_back(nodeTraversal);
                    nodeTraversal = parentNodeInPath[nodeTraversal];
                }
                reversePath(optimumPath);
                return optimumPath;
            }

            if (visitedPortsList[currentPortIndex])
                continue;

            visitedPortsList[currentPortIndex] = true;

            RouteNode* currentPortRoute = ports[currentPortIndex].routeHead;
            while (currentPortRoute != nullptr)
            {
                int neighborPortIndex = findPortIndex(currentPortRoute->destinationPort);
                
                if (neighborPortIndex >= 0 && neighborPortIndex < numberOfPorts && !visitedPortsList[neighborPortIndex])
                {
                    int travelTimeInMinutes = calculateJourneyTime(currentPortRoute->departureTime, currentPortRoute->arrivalTime);
                    float timeToNeighbor = timeFromStart[currentPortIndex] + travelTimeInMinutes;

                    if (timeToNeighbor < timeFromStart[neighborPortIndex])
                    {
                        parentNodeInPath[neighborPortIndex] = currentPortIndex;
                        timeFromStart[neighborPortIndex] = timeToNeighbor;

                        float heuristicEstimate = calculateEuclideanDistance(
                            portCoordinates[neighborPortIndex].second.first,
                            portCoordinates[neighborPortIndex].second.second,
                            destinationXCoordinate, 
                            destinationYCoordinate);
                        estimatedTotalTime[neighborPortIndex] = timeFromStart[neighborPortIndex] + heuristicEstimate;

                        openList.enqueue(neighborPortIndex, estimatedTotalTime[neighborPortIndex]);
                    }
                }
                currentPortRoute = currentPortRoute->next;
            }
        }

        return Vector<int>();
    }

 void showMap(Maps &graph)
{
    RenderWindow window(VideoMode(1920, 1080), "OceanRoute Navigator");
    window.setFramerateLimit(60);

    Texture mapTexture;
    if (!mapTexture.loadFromFile("pics/map2.png"))
    {
        cout << "Map image not loaded!" << endl;
        return;
    }
    Sprite mapSprite(mapTexture);
    mapSprite.setColor(Color(180, 220, 255, 255));

    Font font;
    if (!font.loadFromFile("Roboto.ttf"))
    {
        cout << "Font load failed!" << endl;
        return;
    }

    struct RouteAnimation {
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
        Vector<RouteNode*> pathRoutes;
        int totalCost;
        int totalTime; // in minutes
    };

    Vector<RouteAnimation> activeAnimations;
    float animationSpeed = 0.8f;
    const float SEGMENT_PAUSE = 1.5f;

    float animationStartTime = 0.0f;
    bool animationStarted = false;
    Clock clock;
    bool isAnimating = false;
    float animationTime = 0.0f;

    // Main search button
    RectangleShape searchButton(Vector2f(220, 60));
    searchButton.setFillColor(Color(0, 100, 200, 200));
    searchButton.setOutlineThickness(3);
    searchButton.setOutlineColor(Color(100, 200, 255));
    searchButton.setPosition(40, 880);

    CircleShape buttonGlow(40);
    buttonGlow.setFillColor(Color(100, 200, 255, 80));
    buttonGlow.setPosition(10, 970);

    Text searchButtonText;
    searchButtonText.setFont(font);
    searchButtonText.setString("Search Route");
    searchButtonText.setCharacterSize(22);
    searchButtonText.setFillColor(Color::White);
    searchButtonText.setStyle(Text::Bold);
    searchButtonText.setPosition(55, 895);

    // Search mode selection buttons - Enhanced Design (ShowRoutes & Dijkstra only)
    RectangleShape showRoutesButton(Vector2f(210, 50));
    showRoutesButton.setFillColor(Color(60, 120, 200, 220));
    showRoutesButton.setOutlineThickness(3);
    showRoutesButton.setOutlineColor(Color(150, 200, 255));
    showRoutesButton.setPosition(30, 700);

    RectangleShape dijkstraButton(Vector2f(210, 50));
    dijkstraButton.setFillColor(Color(80, 180, 80, 220));
    dijkstraButton.setOutlineThickness(3);
    dijkstraButton.setOutlineColor(Color(150, 255, 150));
    dijkstraButton.setPosition(30, 770);

    Text showRoutesText;
    showRoutesText.setFont(font);
    showRoutesText.setString("Show Routes");
    showRoutesText.setCharacterSize(16);
    showRoutesText.setFillColor(Color::White);
    showRoutesText.setStyle(Text::Bold);
    showRoutesText.setPosition(55, 715);

    Text dijkstraText;
    dijkstraText.setFont(font);
    dijkstraText.setString("Dijkstra Search");
    dijkstraText.setCharacterSize(16);
    dijkstraText.setFillColor(Color::White);
    dijkstraText.setStyle(Text::Bold);
    dijkstraText.setPosition(50, 785);

    // Cost/Time selection buttons - Enhanced Design
    RectangleShape cheapestButton(Vector2f(210, 50));
    cheapestButton.setFillColor(Color(0, 160, 120, 220));
    cheapestButton.setOutlineThickness(3);
    cheapestButton.setOutlineColor(Color(100, 255, 200));
    cheapestButton.setPosition(30, 860);

    RectangleShape fastestButton(Vector2f(210, 50));
    fastestButton.setFillColor(Color(180, 130, 0, 220));
    fastestButton.setOutlineThickness(3);
    fastestButton.setOutlineColor(Color(255, 220, 100));
    fastestButton.setPosition(30, 930);

    Text cheapestButtonText;
    cheapestButtonText.setFont(font);
    cheapestButtonText.setString("Cheapest Route");
    cheapestButtonText.setCharacterSize(16);
    cheapestButtonText.setFillColor(Color::White);
    cheapestButtonText.setStyle(Text::Bold);
    cheapestButtonText.setPosition(50, 875);

    Text fastestButtonText;
    fastestButtonText.setFont(font);
    fastestButtonText.setString("Fastest Route");
    fastestButtonText.setCharacterSize(16);
    fastestButtonText.setFillColor(Color::White);
    fastestButtonText.setStyle(Text::Bold);
    fastestButtonText.setPosition(52, 945);

    bool showInputWindow = false;
    RectangleShape inputWindow(Vector2f(500, 550));
    inputWindow.setFillColor(Color(0, 30, 70, 250));
    inputWindow.setOutlineThickness(4);
    inputWindow.setOutlineColor(Color(100, 200, 255));
    inputWindow.setPosition(710, 265);

    Text windowTitle;
    windowTitle.setFont(font);
    windowTitle.setString("Route Search");
    windowTitle.setCharacterSize(28);
    windowTitle.setFillColor(Color(100, 255, 255));
    windowTitle.setStyle(Text::Bold);
    windowTitle.setOutlineThickness(2);
    windowTitle.setOutlineColor(Color(0, 0, 0, 150));
    windowTitle.setPosition(810, 285);

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
            label.setCharacterSize(18);
            label.setFillColor(Color(200, 230, 255));
            label.setStyle(Text::Bold);
            label.setPosition(position.x, position.y - 30);

            box.setSize(Vector2f(460, 50));
            box.setFillColor(Color(0, 20, 50, 240));
            box.setOutlineThickness(2);
            box.setOutlineColor(Color(150, 200, 255));
            box.setPosition(position.x, position.y);

            text.setFont(font);
            text.setCharacterSize(20);
            text.setFillColor(Color::White);
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
                    box.setOutlineColor(Color::Yellow);
                }
                else
                {
                    isActive = false;
                    box.setOutlineColor(Color(150, 200, 255));
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
                float pulse = sin(time * 8.0f) * 0.3f + 0.7f;
                box.setOutlineColor(Color(255, 255, 150, 255 * pulse));
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

    // Algorithm selection in popup
    Text algorithmLabel;
    algorithmLabel.setFont(font);
    algorithmLabel.setString("Select Route Type:");
    algorithmLabel.setCharacterSize(18);
    algorithmLabel.setFillColor(Color(200, 230, 255));
    algorithmLabel.setPosition(730, 590);

    RectangleShape popupSearchButton(Vector2f(180, 50));
    popupSearchButton.setFillColor(Color(0, 180, 120));
    popupSearchButton.setOutlineThickness(2);
    popupSearchButton.setOutlineColor(Color(100, 255, 180));
    popupSearchButton.setPosition(750, 670);

    Text popupSearchText;
    popupSearchText.setFont(font);
    popupSearchText.setString("FIND ROUTE");
    popupSearchText.setCharacterSize(20);
    popupSearchText.setFillColor(Color::White);
    popupSearchText.setStyle(Text::Bold);
    popupSearchText.setPosition(780, 682);

    RectangleShape closeButton(Vector2f(180, 50));
    closeButton.setFillColor(Color(180, 80, 80));
    closeButton.setOutlineThickness(2);
    closeButton.setOutlineColor(Color(255, 150, 150));
    closeButton.setPosition(950, 670);

    Text closeText;
    closeText.setFont(font);
    closeText.setString("CANCEL");
    closeText.setCharacterSize(20);
    closeText.setFillColor(Color::White);
    closeText.setStyle(Text::Bold);
    closeText.setPosition(985, 682);

    CircleShape searchButtonGlow(30);
    searchButtonGlow.setFillColor(Color(100, 255, 180, 100));
    searchButtonGlow.setPosition(745, 665);

    CircleShape closeButtonGlow(30);
    closeButtonGlow.setFillColor(Color(255, 150, 150, 100));
    closeButtonGlow.setPosition(945, 665);

    // Algorithm and route type selection state
    string searchMode = "ShowRoutes"; // Default: "ShowRoutes" or "Dijkstra"
    string selectedRouteType = "Cheapest"; 
    bool requiresDate = true; // ShowRoutes requires date, others don't

    int selectedPortIndex = -1;
    RouteNode *hoveredRoute = nullptr;
    bool showRoutes = false;
    float time = 0.0f;
    Vector<float> portPulse;

    Vector<int> currentPath;
    Vector<string> waitPorts;
    Vector<string> waitDurations;
    Vector<RouteNode *> pathRoutes;

    int currentSegment = 0;
    float segmentProgress = 0.0f;
    bool isPaused = false;
    float pauseTimer = 0.0f;

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

    auto addRouteAnimation = [&](Vector<int> path, Vector<string> waitPorts, 
                                Vector<string> waitDurations, Color color, string type) 
    {
        RouteAnimation newAnim;
        newAnim.path = path;
        newAnim.waitPorts = waitPorts;
        newAnim.waitDurations = waitDurations;
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
        
        for (int i = 0; i < path.getSize() - 1; i++)
        {
            string fromPort = portLocations[path[i]].name;
            string toPort = portLocations[path[i + 1]].name;
            RouteNode *route = findRouteBetweenPorts(graph, fromPort, toPort);
            if (route != nullptr)
            {
                newAnim.pathRoutes.push_back(route);
                newAnim.totalCost += route->cost;
                newAnim.totalTime += graph.calculateJourneyTime(route->departureTime, route->arrivalTime);
            }
        }
        
        activeAnimations.push_back(newAnim);
    };

    while (window.isOpen())
    {
        Event event;
        Vector2f mousePos = window.mapPixelToCoords(Mouse::getPosition(window));
        float deltaTime = 0.016f;
        time += deltaTime;
        animationTime += deltaTime;

        while (window.pollEvent(event))
        {
            if (event.type == Event::Closed)
                window.close();

            if (event.type == Event::MouseButtonPressed && event.mouseButton.button == Mouse::Left)
            {
                Vector2i clickPos = Mouse::getPosition(window);

                // Handle search mode selection buttons
                if (!showInputWindow) {
                    if (showRoutesButton.getGlobalBounds().contains(mousePos)) {
                        searchMode = "ShowRoutes";
                        requiresDate = true;
                    }
                    if (dijkstraButton.getGlobalBounds().contains(mousePos)) {
                        searchMode = "Dijkstra";
                        requiresDate = false;
                    }
                    if (cheapestButton.getGlobalBounds().contains(mousePos)) {
                        selectedRouteType = "Cheapest";
                    }
                    if (fastestButton.getGlobalBounds().contains(mousePos)) {
                        selectedRouteType = "Fastest";
                    }
                }

                if (searchButton.getGlobalBounds().contains(mousePos) && !showInputWindow)
                {
                    showInputWindow = true;
                    originInput.setText("");
                    destinationInput.setText("");
                    dateInput.setText("");
                    showSearchResult = false;
                    activeAnimations.clear();
                    originInput.isActive = true;
                    destinationInput.isActive = false;
                    dateInput.isActive = false;
                    animationTime = 0.0f;
                    errorDetails = "";
                }

                if (showInputWindow)
                {
                    originInput.handleEvent(event, window);
                    destinationInput.handleEvent(event, window);
                    if (requiresDate) {
                        dateInput.handleEvent(event, window);
                    }

                    if (popupSearchButton.getGlobalBounds().contains(mousePos))
                    {
                        string origin = originInput.getText();
                        string destination = destinationInput.getText();
                        string date = dateInput.getText();

                        activeAnimations.clear();
                        waitPorts.clear();
                        waitDurations.clear();
                        pathRoutes.clear();
                        errorDetails = "";

                        // Validate input based on search mode
                        if (origin.empty() || destination.empty() || (requiresDate && date.empty()))
                        {
                            searchResultText = "Input Error!";
                            searchResultColor = Color::Red;
                            if (requiresDate) {
                                errorDetails = "Please fill all fields: Departure Port, Destination Port, and Date.";
                            } else {
                                errorDetails = "Please fill: Departure Port and Destination Port.";
                            }
                            showSearchResult = true;
                        }
                        else
                        {
                            bool pathFound = false;

                            // Handle different search modes
                            if (searchMode == "ShowRoutes") {
                                // ShowRoutes mode: date-validated direct/connecting routes
                                Vector<int> directPath, connectingPath;
                                Vector<string> connWaitPorts, connWaitDurations;
                                
                                bool directFound = graph.checkDirectRoutes(origin, destination, date, directPath);
                                bool connectingFound = graph.checkConnectingRoutes(origin, destination, date, connectingPath, connWaitPorts, connWaitDurations);
                                
                                if (directFound && connectingFound) {
                                    searchResultText = "Both routes found!";
                                    searchResultColor = Color::Green;
                                    errorDetails = "Showing direct and connecting routes.";
                                    
                                    addRouteAnimation(directPath, Vector<string>(), Vector<string>(), 
                                                    Color::Green, "Direct");
                                    addRouteAnimation(connectingPath, connWaitPorts, connWaitDurations, 
                                                    Color::Cyan, "Connecting");
                                    pathFound = true;
                                } else if (directFound) {
                                    searchResultText = "Direct route found!";
                                    searchResultColor = Color::Green;
                                    addRouteAnimation(directPath, Vector<string>(), Vector<string>(),
                                                    Color::Green, "Direct");
                                    pathFound = true;
                                } else if (connectingFound) {
                                    searchResultText = "Connecting route found!";
                                    searchResultColor = Color::Green;
                                    addRouteAnimation(connectingPath, connWaitPorts, connWaitDurations,
                                                    Color::Cyan, "Connecting");
                                    pathFound = true;
                                } else {
                                    searchResultText = "No route found!";
                                    searchResultColor = Color::Red;
                                    errorDetails = "No routes available for given ports and date.";
                                    pathFound = false;
                                }
                            }
                            else if (searchMode == "Dijkstra") {
                                // Dijkstra mode: find cheapest and fastest paths, highlight selected
                                Vector<int> cheapestPath = graph.findCheapestPath(origin, destination);
                                Vector<int> fastestPath = graph.findFastestPath(origin, destination);
                                
                                Vector<int> pathToHighlight = (selectedRouteType == "Cheapest") ? cheapestPath : fastestPath;
                                
                                if (pathToHighlight.getSize() > 1) {
                                    searchResultText = "Dijkstra " + selectedRouteType + " path found!";
                                    searchResultColor = Color::Green;
                                    errorDetails = selectedRouteType + " route with " + 
                                                 to_string(pathToHighlight.getSize() - 1) + " segments";
                                    
                                    Color routeColor = (selectedRouteType == "Cheapest") ? Color::Green : Color::Cyan;
                                    addRouteAnimation(pathToHighlight, Vector<string>(), Vector<string>(),
                                                    routeColor, "Dijkstra " + selectedRouteType);
                                    pathFound = true;
                                } else {
                                    searchResultText = "No Dijkstra path found!";
                                    searchResultColor = Color::Red;
                                    errorDetails = "No routes available for given ports.";
                                    pathFound = false;
                                }
                            }
                            
                            showSearchResult = true;
                            showRoutes = true;
                        }
                    }
                    if (closeButton.getGlobalBounds().contains(mousePos))
                    {
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
                        float dist = sqrt(pow(mousePos.x - px, 2) + pow(mousePos.y - py, 2));
                        if (dist < 12)
                        {
                            selectedPortIndex = i;
                            showRoutes = true;
                            portPulse[i] = 1.0f;
                            showSearchResult = false;
                            animationTime = 0.0f;
                            errorDetails = "";
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
                if (showInputWindow)
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
                    animationTime = 0.0f;
                    errorDetails = "";
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
            RouteAnimation& anim = activeAnimations[animIndex];
            
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

        for (int i = 0; i < waterRipples.getSize(); i++)
        {
            window.draw(waterRipples[i]);
        }
        window.draw(mapSprite);

        // Draw algorithm selection panel - Enhanced Design
        RectangleShape algorithmPanel(Vector2f(280, 300));
        algorithmPanel.setFillColor(Color(0, 20, 50, 220));
        algorithmPanel.setOutlineThickness(3);
        algorithmPanel.setOutlineColor(Color(100, 200, 255, 200));
        algorithmPanel.setPosition(20, 680);
        window.draw(algorithmPanel);

        // Add panel background gradient effect
        RectangleShape panelGradient(Vector2f(280, 300));
        panelGradient.setFillColor(Color(10, 40, 80, 100));
        panelGradient.setPosition(20, 680);
        window.draw(panelGradient);

        Text panelTitle;
        panelTitle.setFont(font);
        panelTitle.setString("Search Options");
        panelTitle.setCharacterSize(20);
        panelTitle.setFillColor(Color(100, 255, 255));
        panelTitle.setStyle(Text::Bold);
        panelTitle.setPosition(45, 690);
        window.draw(panelTitle);

        // Search Mode Separator
        Text searchModeLabel;
        searchModeLabel.setFont(font);
        searchModeLabel.setString("Search Mode");
        searchModeLabel.setCharacterSize(14);
        searchModeLabel.setFillColor(Color(150, 220, 255));
        searchModeLabel.setStyle(Text::Bold);
        searchModeLabel.setPosition(35, 730);
        window.draw(searchModeLabel);

        // Route Type Separator
        Text routeTypeLabel;
        routeTypeLabel.setFont(font);
        routeTypeLabel.setString("Route Type");
        routeTypeLabel.setCharacterSize(14);
        routeTypeLabel.setFillColor(Color(150, 255, 200));
        routeTypeLabel.setStyle(Text::Bold);
        routeTypeLabel.setPosition(35, 850);
        window.draw(routeTypeLabel);

        // Update button colors based on selection with smooth transitions
        showRoutesButton.setFillColor(searchMode == "ShowRoutes" ? 
            Color(100, 170, 255, 240) : Color(60, 120, 200, 220));
        dijkstraButton.setFillColor(searchMode == "Dijkstra" ? 
            Color(150, 230, 120, 240) : Color(80, 180, 80, 220));
        
        cheapestButton.setFillColor(selectedRouteType == "Cheapest" ? 
            Color(100, 220, 180, 240) : Color(0, 160, 120, 220));
        fastestButton.setFillColor(selectedRouteType == "Fastest" ? 
            Color(255, 200, 100, 240) : Color(180, 130, 0, 220));

        // Add hover glow effects
        if (showRoutesButton.getGlobalBounds().contains(mousePos) && !showInputWindow) {
            showRoutesButton.setOutlineThickness(4);
            showRoutesButton.setOutlineColor(Color(200, 255, 255, 255));
        } else {
            showRoutesButton.setOutlineThickness(3);
            showRoutesButton.setOutlineColor(Color(150, 200, 255, 200));
        }

        if (dijkstraButton.getGlobalBounds().contains(mousePos) && !showInputWindow) {
            dijkstraButton.setOutlineThickness(4);
            dijkstraButton.setOutlineColor(Color(200, 255, 200, 255));
        } else {
            dijkstraButton.setOutlineThickness(3);
            dijkstraButton.setOutlineColor(Color(150, 255, 150, 200));
        }

        if (cheapestButton.getGlobalBounds().contains(mousePos) && !showInputWindow) {
            cheapestButton.setOutlineThickness(4);
            cheapestButton.setOutlineColor(Color(200, 255, 220, 255));
        } else {
            cheapestButton.setOutlineThickness(3);
            cheapestButton.setOutlineColor(Color(100, 255, 200, 200));
        }

        if (fastestButton.getGlobalBounds().contains(mousePos) && !showInputWindow) {
            fastestButton.setOutlineThickness(4);
            fastestButton.setOutlineColor(Color(255, 240, 150, 255));
        } else {
            fastestButton.setOutlineThickness(3);
            fastestButton.setOutlineColor(Color(255, 220, 100, 200));
        }

        // Draw search mode buttons with enhanced styling
        window.draw(showRoutesButton);
        window.draw(showRoutesText);
        window.draw(dijkstraButton);
        window.draw(dijkstraText);
        window.draw(cheapestButton);
        window.draw(cheapestButtonText);
        window.draw(fastestButton);
        window.draw(fastestButtonText);

        // Draw current selection info
        Text currentSelection;
        currentSelection.setFont(font);
        currentSelection.setString("Mode: " + searchMode + " | Route: " + selectedRouteType);
        currentSelection.setCharacterSize(14);
        currentSelection.setFillColor(Color(200, 230, 255));
        currentSelection.setPosition(35, 660);
        window.draw(currentSelection);

        for (int animIndex = 0; animIndex < activeAnimations.getSize(); animIndex++)
        {
            RouteAnimation& anim = activeAnimations[animIndex];
            
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
                    RectangleShape routeInfoBox(Vector2f(500, 160));
                    routeInfoBox.setFillColor(Color(0, 30, 60, 200));
                    routeInfoBox.setOutlineThickness(3);
                    routeInfoBox.setOutlineColor(anim.routeColor);
                    routeInfoBox.setPosition(250, 900);
                    window.draw(routeInfoBox);

                    Text routeTitle;
                    routeTitle.setFont(font);
                    routeTitle.setString(anim.routeType + " - $" + to_string(anim.totalCost) + 
                                       " - " + formatTime(anim.totalTime));
                    routeTitle.setCharacterSize(18);
                    routeTitle.setFillColor(anim.routeColor);
                    routeTitle.setStyle(Text::Bold);
                    routeTitle.setPosition(260, 910);
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
                    pathText.setPosition(260, 940);
                    window.draw(pathText);

                    if (anim.isComplete)
                    {
                        Text completeText;
                        completeText.setFont(font);
                        completeText.setString("✓ Route Complete");
                        completeText.setCharacterSize(14);
                        completeText.setFillColor(Color::Green);
                        completeText.setPosition(260, 965);
                        window.draw(completeText);
                    }
                }
            }
        }

        // Rest of the existing drawing code remains the same...
        hoveredRoute = nullptr;
        if (activeAnimations.getSize() > 0)
        {
            for (int animIndex = 0; animIndex < activeAnimations.getSize(); animIndex++)
            {
                RouteAnimation& anim = activeAnimations[animIndex];
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
                        break;
                    }
                }
                if (hoveredRoute != nullptr) break;
            }
        }

        if (showRoutes && selectedPortIndex != -1 && !showSearchResult)
        {
            RouteNode *route = graph.ports[selectedPortIndex].routeHead;
            int routeIndex = 0;

            while (route)
            {
                int destIdx = -1;
                for (int k = 0; k < numLocations; k++)
                {
                    if (portLocations[k].name == route->destinationPort)
                        destIdx = k;
                }
                if (destIdx != -1)
                {
                    Vector2f start(portLocations[selectedPortIndex].x, portLocations[selectedPortIndex].y);
                    Vector2f end(portLocations[destIdx].x, portLocations[destIdx].y);

                    float length = sqrt(pow(end.x - start.x, 2) + pow(end.y - start.y, 2));
                    float angle = atan2(end.y - start.y, end.x - start.x) * 180 / 3.14159f;

                    float baseThickness = (route->cost / 5000.0f) + 1.5f;
                    if (baseThickness > 6.0f)
                        baseThickness = 6.0f;

                    float pulse = 0.2f * sin(time * 3.0f + routeIndex);
                    float thickness = baseThickness * (1.0f + pulse);

                    RectangleShape line(Vector2f(length, thickness));
                    line.setPosition(start);
                    line.setRotation(angle);

                    Color lineColor;
                    if (route->cost < 7000)
                        lineColor = Color(100, 255, 150);
                    else if (route->cost < 10000)
                        lineColor = Color(255, 255, 100);
                    else
                        lineColor = Color(255, 100, 100);

                    RectangleShape glowLine(Vector2f(length, thickness + 4.0f));
                    glowLine.setPosition(start);
                    glowLine.setRotation(angle);
                    Color glowColor = lineColor;
                    glowColor.a = 80;
                    glowLine.setFillColor(glowColor);
                    window.draw(glowLine);

                    line.setFillColor(lineColor);
                    window.draw(line);

                    float distance = abs((end.y - start.y) * mousePos.x - (end.x - start.x) * mousePos.y + end.x * start.y - end.y * start.x) / sqrt(pow(end.y - start.y, 2) + pow(end.x - start.x, 2));
                    if (distance < 20.0f && hoveredRoute == nullptr)
                    {
                        hoveredRoute = route;
                    }
                }
                route = route->next;
                routeIndex++;
            }
        }

        for (int i = 0; i < numLocations; i++)
        {
            CircleShape glow(16.f);
            glow.setOrigin(16, 16);
            glow.setFillColor(Color(100, 180, 255, 100));
            glow.setPosition(portLocations[i].x, portLocations[i].y);
            window.draw(glow);

            float pulseScale = 1.0f + 0.2f * portPulse[i];
            CircleShape dot(8.f * pulseScale);
            dot.setOrigin(8.f * pulseScale, 8.f * pulseScale);

            bool inPath = false;
            for (int animIndex = 0; animIndex < activeAnimations.getSize(); animIndex++)
            {
                RouteAnimation& anim = activeAnimations[animIndex];
                for (int j = 0; j < anim.path.getSize(); j++)
                {
                    if (anim.path[j] == i)
                    {
                        inPath = true;
                        break;
                    }
                }
                if (inPath) break;
            }

            if (inPath)
            {
                dot.setFillColor(Color::Cyan);
            }
            else if (i == selectedPortIndex)
            {
                dot.setFillColor(Color::Red);
                CircleShape selectionGlow(20.f);
                selectionGlow.setOrigin(20, 20);
                selectionGlow.setFillColor(Color(255, 50, 50, 100));
                selectionGlow.setPosition(portLocations[i].x, portLocations[i].y);
                window.draw(selectionGlow);
            }
            else if (showRoutes && selectedPortIndex != -1)
            {
                dot.setFillColor(Color(100, 200, 255));
            }
            else
            {
                float gentlePulse = 0.5f + 0.5f * sin(time * 2.0f + i);
                dot.setFillColor(Color(0, 150 * gentlePulse, 255));
            }

            dot.setOutlineColor(Color::White);
            dot.setOutlineThickness(1.5f);
            dot.setPosition(portLocations[i].x, portLocations[i].y);
            window.draw(dot);
        }

        if (selectedPortIndex != -1 || activeAnimations.getSize() > 0)
        {
            if (selectedPortIndex != -1)
            {
                drawLabel(window, font, selectedPortIndex, Color::Red, 18);

                RouteNode *route = graph.ports[selectedPortIndex].routeHead;
                while (route)
                {
                    int destIdx = -1;
                    for (int k = 0; k < numLocations; k++)
                        if (portLocations[k].name == route->destinationPort)
                            destIdx = k;
                    if (destIdx != -1)
                        drawLabel(window, font, destIdx, Color::Yellow, 16);
                    route = route->next;
                }
            }

            for (int animIndex = 0; animIndex < activeAnimations.getSize(); animIndex++)
            {
                RouteAnimation& anim = activeAnimations[animIndex];
                for (int i = 0; i < anim.path.getSize(); i++)
                {
                    drawLabel(window, font, anim.path[i], anim.routeColor, 16);
                }
            }
        }

        bool searchHover = searchButton.getGlobalBounds().contains(mousePos) && !showInputWindow;

        float buttonPulse = sin(time * 3.0f) * 0.2f + 0.8f;
        buttonGlow.setRadius(40 * buttonPulse);
        buttonGlow.setOrigin(40 * buttonPulse, 40 * buttonPulse);
        buttonGlow.setPosition(30, 1010);

        if (searchHover)
        {
            searchButton.setFillColor(Color(0, 150, 255, 220));
            searchButton.setOutlineColor(Color(150, 230, 255));
            window.draw(buttonGlow);
        }
        else
        {
            searchButton.setFillColor(Color(0, 100, 200, 200));
            searchButton.setOutlineColor(Color(100, 200, 255));
        }

        window.draw(searchButton);
        window.draw(searchButtonText);

        if (showInputWindow)
        {
            RectangleShape overlay(Vector2f(1920, 1080));
            overlay.setFillColor(Color(0, 0, 0, 120));
            window.draw(overlay);

            window.draw(inputWindow);

            window.draw(windowTitle);

            originInput.draw(window);
            destinationInput.draw(window);
            
            // Only draw date input if required by search mode
            if (requiresDate) {
                dateInput.draw(window);
            }

            window.draw(algorithmLabel);

            // Update popup cost selection button colors
            cheapestButton.setFillColor(selectedRouteType == "Cheapest" ? 
                Color(0, 200, 150, 220) : Color(0, 150, 100, 200));
            fastestButton.setFillColor(selectedRouteType == "Fastest" ? 
                Color(200, 150, 0, 220) : Color(150, 100, 0, 200));

            window.draw(cheapestButton);
            window.draw(cheapestButtonText);
            window.draw(fastestButton);
            window.draw(fastestButtonText);

            bool popupSearchHover = popupSearchButton.getGlobalBounds().contains(mousePos);
            bool closeHover = closeButton.getGlobalBounds().contains(mousePos);

            if (popupSearchHover)
            {
                popupSearchButton.setFillColor(Color(0, 220, 150));
                popupSearchButton.setOutlineColor(Color(150, 255, 200));
                window.draw(searchButtonGlow);
            }
            else
            {
                popupSearchButton.setFillColor(Color(0, 180, 120));
                popupSearchButton.setOutlineColor(Color(100, 255, 180));
            }

            if (closeHover)
            {
                closeButton.setFillColor(Color(220, 100, 100));
                closeButton.setOutlineColor(Color(255, 180, 180));
                window.draw(closeButtonGlow);
            }
            else
            {
                closeButton.setFillColor(Color(180, 80, 80));
                closeButton.setOutlineColor(Color(255, 150, 150));
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
                resultText.setPosition(750, 730);
                window.draw(resultText);

                if (!errorDetails.empty())
                {
                    Text detailsText;
                    detailsText.setFont(font);
                    detailsText.setString(errorDetails);
                    detailsText.setCharacterSize(16);
                    detailsText.setFillColor(searchResultColor == Color::Green ? Color::Cyan : Color::Yellow);
                    detailsText.setStyle(Text::Bold);
                    detailsText.setPosition(730, 760);
                    window.draw(detailsText);
                }
            }
        }

        if (hoveredRoute != nullptr && !showInputWindow)
        {
            float boxWidth = 420.f, boxHeight = 220.f;
            float boxX = mousePos.x + 20.f;
            float boxY = mousePos.y - 200.f;
            if (boxX + boxWidth > 1920)
                boxX = 1920 - boxWidth - 20;
            if (boxY < 0)
                boxY = 20;

            RectangleShape infoBox(Vector2f(boxWidth, boxHeight));
            infoBox.setFillColor(Color(0, 20, 60, 240));
            infoBox.setOutlineThickness(3.f);
            infoBox.setOutlineColor(Color(100, 200, 255));
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
            header.setString("ROUTE DETAILS");
            header.setCharacterSize(18);
            header.setFillColor(Color(100, 255, 255));
            header.setStyle(Text::Bold);
            header.setPosition(boxX + 15.f, boxY + 10.f);
            window.draw(header);

            Text details;
            details.setFont(font);
            details.setCharacterSize(16);
            details.setFillColor(Color(200, 230, 255));
            details.setStyle(Text::Bold);
            details.setPosition(boxX + 15.f, boxY + 40.f);

            string infoText = "From: " + hoveredRoute->startingPort + "\n" +
                              "To: " + hoveredRoute->destinationPort + "\n" +
                              "Cost: $" + to_string(hoveredRoute->cost) + "\n" +
                              "Date: " + hoveredRoute->departureDate + "\n" +
                              "Departure: " + hoveredRoute->departureTime + "\n" +
                              "Arrival: " + hoveredRoute->arrivalTime + "\n" +
                              "Company: " + hoveredRoute->shippingCompany;

            details.setString(infoText);
            window.draw(details);

            static float boxPulse = 0.0f;
            boxPulse += 0.1f;
            float pulseValue = 0.5f + 0.5f * sin(boxPulse);
            infoBox.setOutlineColor(Color(100, 200, 255, 150 + 105 * pulseValue));
        }

        Text title;
        title.setFont(font);
        title.setString("OceanRoute Navigator");
        title.setCharacterSize(28);
        title.setFillColor(Color(200, 230, 255));
        title.setStyle(Text::Bold);
        title.setPosition(20, 20);
        window.draw(title);

        Text legend;
        legend.setFont(font);
        legend.setString("Route Cost:\nGreen: < $7000\nYellow: $7000-$10000\nRed: > $10000");
        legend.setCharacterSize(14);
        legend.setFillColor(Color(180, 220, 255));
        legend.setPosition(20, 900);
        window.draw(legend);

        window.display();
    }
}
};

//* Global Variable for the Booking Manager
BookingManager bookingManager;
void bookingInterface(Maps &graph, Font &font)
{
    RenderWindow window(VideoMode(1920, 1080), "Cargo Booking - OceanRoute Navigator");
    window.setFramerateLimit(60);

    Texture bgTexture;
    if (!bgTexture.loadFromFile("mainmenu.jpg"))
    {
        cout << "Error loading background!" << endl;
    }
    Sprite bgSprite(bgTexture);
    bgSprite.setScale(1920.0f / bgTexture.getSize().x, 1080.0f / bgTexture.getSize().y);
    bgSprite.setColor(Color(180, 200, 220, 255));

    Clock clock;
    float time = 0.0f;

    enum BookingState
    {
        ENTER_DETAILS,
        DISPLAY_ROUTE,
        SUCCESS
    };
    BookingState currentState = ENTER_DETAILS;

    Vector<int> currentPath;
    Vector<string> waitPorts;
    Vector<string> waitDurations;
    string searchResultText = "";
    Color searchResultColor = Color::White;
    string errorDetails = "";
    bool routeFound = false;

    Booking currentBooking;

    int currentSegment = 0;
    float segmentProgress = 0.0f;
    float animationSpeed = 0.8f;
    bool isPaused = false;
    float pauseTimer = 0.0f;
    const float SEGMENT_PAUSE = 1.5f;
    bool animationStarted = false;

    InputBox nameInput(font, "Customer Name", Vector2f(710, 250));
    InputBox originInput(font, "Origin Port", Vector2f(710, 340));
    InputBox destInput(font, "Destination Port", Vector2f(710, 430));
    InputBox dateInput(font, "Departure Date (DD/MM/YYYY)", Vector2f(710, 520));

    nameInput.isActive = true;

    RectangleShape searchButton(Vector2f(220, 50));
    searchButton.setFillColor(Color(0, 150, 255));
    searchButton.setOutlineThickness(3);
    searchButton.setOutlineColor(Color(100, 200, 255));
    searchButton.setPosition(800, 620);

    Text searchBtnText("BOOK ROUTE", font, 20);
    searchBtnText.setStyle(Text::Bold);
    searchBtnText.setPosition(830, 632);

    RectangleShape backButton(Vector2f(150, 50));
    backButton.setFillColor(Color(180, 80, 80));
    backButton.setOutlineThickness(3);
    backButton.setOutlineColor(Color(255, 150, 150));
    backButton.setPosition(50, 950);

    Text backBtnText("BACK", font, 20);
    backBtnText.setStyle(Text::Bold);
    backBtnText.setPosition(85, 962);

    RectangleShape confirmButton(Vector2f(220, 50));
    confirmButton.setFillColor(Color(0, 200, 100));
    confirmButton.setOutlineThickness(3);
    confirmButton.setOutlineColor(Color(100, 255, 180));
    confirmButton.setPosition(1650, 950);

    Text confirmBtnText("CONFIRM BOOKING", font, 18);
    confirmBtnText.setStyle(Text::Bold);
    confirmBtnText.setPosition(1660, 962);

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
            {
                window.close();
            }

            if (currentState == ENTER_DETAILS)
            {
                nameInput.handleEvent(event, window);
                originInput.handleEvent(event, window);
                destInput.handleEvent(event, window);
                dateInput.handleEvent(event, window);

                if (event.type == Event::KeyPressed && event.key.code == Keyboard::Tab)
                {
                    if (nameInput.isActive)
                    {
                        nameInput.isActive = false;
                        originInput.isActive = true;
                        destInput.isActive = false;
                        dateInput.isActive = false;
                    }
                    else if (originInput.isActive)
                    {
                        nameInput.isActive = false;
                        originInput.isActive = false;
                        destInput.isActive = true;
                        dateInput.isActive = false;
                    }
                    else if (destInput.isActive)
                    {
                        nameInput.isActive = false;
                        originInput.isActive = false;
                        destInput.isActive = false;
                        dateInput.isActive = true;
                    }
                    else
                    {
                        nameInput.isActive = true;
                        originInput.isActive = false;
                        destInput.isActive = false;
                        dateInput.isActive = false;
                    }
                }
            }

            if (event.type == Event::MouseButtonPressed)
            {
                if (backButton.getGlobalBounds().contains(mousePos))
                {
                    window.close();
                }

                if (currentState == ENTER_DETAILS && searchButton.getGlobalBounds().contains(mousePos))
                {
                    string name = nameInput.getText();
                    string origin = originInput.getText();
                    string dest = destInput.getText();
                    string date = dateInput.getText();

                    if (name.empty() || origin.empty() || dest.empty() || date.empty())
                    {
                        searchResultText = "Please fill all fields!";
                        searchResultColor = Color::Red;
                        errorDetails = "";
                    }
                    else
                    {
                        currentPath.clear();
                        waitPorts.clear();
                        waitDurations.clear();

                        routeFound = graph.findAndDisplayRoute(origin, dest, date, currentPath,
                                                               waitPorts, waitDurations,
                                                               searchResultText, searchResultColor, errorDetails);

                        if (routeFound)
                        {
                            currentBooking.customerName = name;
                            currentBooking.origin = origin;
                            currentBooking.destination = dest;
                            currentBooking.departureDate = date;
                            currentBooking.routePath.clear();
                            for (int i = 0; i < currentPath.getSize(); i++)
                            {
                                currentBooking.routePath.push_back(portLocations[currentPath[i]].name);
                            }
                            currentBooking.waitPorts = waitPorts;
                            currentBooking.waitDurations = waitDurations;

                            int baseCost = 5000;
                            int routeCost = (currentPath.getSize() - 1) * 3000;

                            int portCharges = 0;
                            for (int i = 0; i < currentPath.getSize(); i++)
                            {
                                portCharges += graph.ports[currentPath[i]].charge;
                            }

                            currentBooking.totalCost = baseCost + routeCost + portCharges;

                            currentState = DISPLAY_ROUTE;
                            animationStarted = true;
                            currentSegment = 0;
                            segmentProgress = 0.0f;
                            isPaused = false;
                        }
                    }
                }

                if (currentState == DISPLAY_ROUTE && confirmButton.getGlobalBounds().contains(mousePos))
                {
                    currentBooking.bookingID = bookingManager.GenerateBookingID();

                    Booking *newBooking = new Booking();
                    *newBooking = currentBooking;
                    bookingManager.AddBooking(newBooking);

                    currentState = SUCCESS;
                }
            }
        }

        window.clear();
        window.draw(bgSprite);

        RectangleShape overlay(Vector2f(1920, 1080));
        overlay.setFillColor(Color(0, 0, 0, 100));
        window.draw(overlay);

        if (currentState == ENTER_DETAILS)
        {
            RectangleShape mainBox(Vector2f(900, 700));
            mainBox.setFillColor(Color(0, 30, 70, 240));
            mainBox.setOutlineThickness(4);
            mainBox.setOutlineColor(Color(100, 200, 255));
            mainBox.setPosition(510, 140);
            window.draw(mainBox);

            Text title;
            title.setFont(font);
            title.setString("Book Cargo Route");
            title.setCharacterSize(36);
            title.setStyle(Text::Bold);
            title.setFillColor(Color(100, 255, 255));
            title.setOutlineThickness(2);
            title.setOutlineColor(Color(0, 0, 0, 150));
            title.setPosition(720, 160);
            window.draw(title);

            nameInput.draw(window);
            originInput.draw(window);
            destInput.draw(window);
            dateInput.draw(window);

            if (searchButton.getGlobalBounds().contains(mousePos))
            {
                searchButton.setFillColor(Color(0, 180, 255));
            }
            else
            {
                searchButton.setFillColor(Color(0, 150, 255));
            }
            window.draw(searchButton);
            window.draw(searchBtnText);

            if (!searchResultText.empty())
            {
                Text resultText;
                resultText.setFont(font);
                resultText.setString(searchResultText);
                resultText.setCharacterSize(24);
                resultText.setFillColor(searchResultColor);
                resultText.setStyle(Text::Bold);
                resultText.setPosition(720, 720);
                window.draw(resultText);

                if (!errorDetails.empty())
                {
                    Text detailText;
                    detailText.setFont(font);
                    detailText.setString(errorDetails);
                    detailText.setCharacterSize(16);
                    detailText.setFillColor(Color::Yellow);
                    detailText.setPosition(720, 760);
                    window.draw(detailText);
                }
            }
        }
        else if (currentState == DISPLAY_ROUTE)
        {
            static Texture mapTexture;
            static bool mapLoaded = false;
            if (!mapLoaded)
            {
                if (mapTexture.loadFromFile("pics/map2.png"))
                {
                    mapLoaded = true;
                }
            }

            if (mapLoaded)
            {
                Sprite mapSprite(mapTexture);
                mapSprite.setColor(Color(180, 220, 255, 255));
                window.draw(mapSprite);
            }

            if (animationStarted)
            {
                if (!isPaused)
                {
                    segmentProgress += deltaTime * animationSpeed;

                    if (segmentProgress >= 1.0f)
                    {
                        segmentProgress = 1.0f;
                        isPaused = true;
                        pauseTimer = 0.0f;
                    }
                }
                else
                {
                    pauseTimer += deltaTime;
                    if (pauseTimer >= SEGMENT_PAUSE)
                    {
                        currentSegment++;
                        segmentProgress = 0.0f;
                        isPaused = false;

                        if (currentSegment >= currentPath.getSize() - 1)
                        {
                            currentSegment = 0;
                            segmentProgress = 0.0f;
                        }
                    }
                }
            }

            for (int i = 0; i < currentPath.getSize() - 1; i++)
            {
                int startIdx = currentPath[i];
                int endIdx = currentPath[i + 1];
                Vector2f start(portLocations[startIdx].x, portLocations[startIdx].y);
                Vector2f end(portLocations[endIdx].x, portLocations[endIdx].y);

                float length = sqrt(pow(end.x - start.x, 2) + pow(end.y - start.y, 2));
                float angle = atan2(end.y - start.y, end.x - start.x) * 180 / 3.14159f;

                if (i == currentSegment)
                {
                    float currentLength = length * segmentProgress;

                    RectangleShape currentLine(Vector2f(currentLength, 6.0f));
                    currentLine.setPosition(start);
                    currentLine.setRotation(angle);
                    currentLine.setFillColor(Color::Cyan);
                    window.draw(currentLine);

                    RectangleShape currentGlow(Vector2f(currentLength, 12.0f));
                    currentGlow.setPosition(start);
                    currentGlow.setRotation(angle);
                    currentGlow.setFillColor(Color(0, 255, 255, 100));
                    window.draw(currentGlow);

                    Vector2f currentPos = start + (end - start) * segmentProgress;
                    float pulse = 0.7f + 0.3f * sin(time * 8.0f);

                    CircleShape endGlow(15.f * pulse);
                    endGlow.setFillColor(Color(0, 255, 255, 150));
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
                else if (i < currentSegment)
                {
                    RectangleShape completedLine(Vector2f(length, 5.0f));
                    completedLine.setPosition(start);
                    completedLine.setRotation(angle);
                    completedLine.setFillColor(Color(0, 200, 200));
                    window.draw(completedLine);
                }
                else
                {
                    RectangleShape upcomingLine(Vector2f(length, 3.0f));
                    upcomingLine.setPosition(start);
                    upcomingLine.setRotation(angle);
                    upcomingLine.setFillColor(Color(0, 100, 100, 100));
                    window.draw(upcomingLine);
                }
            }

            for (int i = 0; i < currentPath.getSize(); i++)
            {
                int portIdx = currentPath[i];
                Vector2f portPos(portLocations[portIdx].x, portLocations[portIdx].y);

                CircleShape glow(16.f);
                glow.setOrigin(16, 16);
                glow.setFillColor(Color(100, 180, 255, 100));
                glow.setPosition(portPos);
                window.draw(glow);

                CircleShape dot(8.f);
                dot.setOrigin(8.f, 8.f);
                dot.setFillColor(Color::Cyan);
                dot.setOutlineColor(Color::White);
                dot.setOutlineThickness(1.5f);
                dot.setPosition(portPos);
                window.draw(dot);

                Text label;
                label.setFont(font);
                label.setString(portLocations[portIdx].name);
                label.setCharacterSize(16);
                label.setStyle(Text::Bold);
                label.setOutlineThickness(1.f);
                label.setOutlineColor(Color(0, 0, 0, 255));
                label.setFillColor(Color::Cyan);
                label.setPosition(portPos.x, portPos.y - 25.f);
                window.draw(label);
            }

            RectangleShape routeInfoBox(Vector2f(500, 220));
            routeInfoBox.setFillColor(Color(0, 30, 60, 230));
            routeInfoBox.setOutlineThickness(3);
            routeInfoBox.setOutlineColor(Color(100, 200, 255));
            routeInfoBox.setPosition(50, 50);
            window.draw(routeInfoBox);

            Text routeInfo;
            routeInfo.setFont(font);
            routeInfo.setCharacterSize(16);
            routeInfo.setFillColor(Color(200, 230, 255));
            routeInfo.setPosition(70, 70);

            string infoText = "Customer: " + currentBooking.customerName + "\n";
            infoText += "Route: " + currentBooking.origin + " ->" + currentBooking.destination + "\n";
            infoText += "Complete Path:\n";
            for (int i = 0; i < currentBooking.routePath.getSize(); i++)
            {
                infoText += "  " + currentBooking.routePath[i];
                if (i < currentBooking.routePath.getSize() - 1)
                    infoText += " -> \n";
            }

            if (currentBooking.waitPorts.getSize() > 0)
            {
                infoText += "\nDocking Times:\n";
                for (int i = 0; i < currentBooking.waitPorts.getSize(); i++)
                {
                    infoText += "  " + currentBooking.waitPorts[i] + ": " + currentBooking.waitDurations[i] + "\n";
                }
            }

            routeInfo.setString(infoText);
            window.draw(routeInfo);

            RectangleShape costBox(Vector2f(300, 80));
            costBox.setFillColor(Color(0, 50, 30, 230));
            costBox.setOutlineThickness(3);
            costBox.setOutlineColor(Color(100, 255, 180));
            costBox.setPosition(50, 850);
            window.draw(costBox);

            Text costText;
            costText.setFont(font);
            costText.setString("Total Cost: $" + to_string(currentBooking.totalCost));
            costText.setCharacterSize(24);
            costText.setFillColor(Color(100, 255, 180));
            costText.setStyle(Text::Bold);
            costText.setPosition(70, 875);
            window.draw(costText);

            if (confirmButton.getGlobalBounds().contains(mousePos))
            {
                confirmButton.setFillColor(Color(0, 230, 120));
            }
            else
            {
                confirmButton.setFillColor(Color(0, 200, 100));
            }
            window.draw(confirmButton);
            window.draw(confirmBtnText);
        }
        else if (currentState == SUCCESS)
        {
            RectangleShape successBox(Vector2f(800, 500));
            successBox.setFillColor(Color(0, 50, 30, 240));
            successBox.setOutlineThickness(4);
            successBox.setOutlineColor(Color(100, 255, 180));
            successBox.setPosition(560, 290);
            window.draw(successBox);

            Text title;
            title.setFont(font);
            title.setString("Booking Confirmed!");
            title.setCharacterSize(42);
            title.setStyle(Text::Bold);
            title.setFillColor(Color(100, 255, 180));
            title.setOutlineThickness(2);
            title.setOutlineColor(Color(0, 0, 0, 150));
            title.setPosition(680, 320);
            window.draw(title);

            Text successMsg;
            successMsg.setFont(font);
            successMsg.setCharacterSize(20);
            successMsg.setFillColor(Color(200, 255, 230));
            successMsg.setPosition(620, 400);

            string msg = "Booking ID: " + currentBooking.bookingID + "\n\n";
            msg += "Customer: " + currentBooking.customerName + "\n\n";
            msg += "Route: " + currentBooking.origin + " -> " + currentBooking.destination + "\n\n";
            msg += "Date: " + currentBooking.departureDate + "\n\n";
            msg += "Total Cost: $" + to_string(currentBooking.totalCost) + "\n\n\n";
            msg += "Your booking has been saved successfully!\n";
            msg += "Press BACK to return to main menu";

            successMsg.setString(msg);
            window.draw(successMsg);
        }

        if (backButton.getGlobalBounds().contains(mousePos))
        {
            backButton.setFillColor(Color(220, 100, 100));
        }
        else
        {
            backButton.setFillColor(Color(180, 80, 80));
        }
        window.draw(backButton);
        window.draw(backBtnText);

        window.display();
    }
}

void viewBookings(Font &font)
{
    RenderWindow window(VideoMode(1920, 1080), "View Bookings");

    Texture bgTexture;
    if (!bgTexture.loadFromFile("mainmenu.jpg"))
    {
        cout << "Error loading background!" << endl;
    }
    Sprite bgSprite(bgTexture);
    bgSprite.setScale(1920.0f / bgTexture.getSize().x, 1080.0f / bgTexture.getSize().y);
    bgSprite.setColor(Color(180, 200, 220, 255));

    while (window.isOpen())
    {
        Event event;
        while (window.pollEvent(event))
        {
            if (event.type == Event::Closed)
                window.close();
            if (event.type == Event::KeyPressed && event.key.code == Keyboard::Escape)
            {
                window.close();
            }
        }

        window.clear();
        window.draw(bgSprite);

        RectangleShape overlay(Vector2f(1920, 1080));
        overlay.setFillColor(Color(0, 0, 0, 120));
        window.draw(overlay);

        RectangleShape mainBox(Vector2f(1400, 900));
        mainBox.setFillColor(Color(0, 30, 70, 240));
        mainBox.setOutlineThickness(4);
        mainBox.setOutlineColor(Color(100, 200, 255));
        mainBox.setPosition(260, 90);
        window.draw(mainBox);

        Text title;
        title.setFont(font);
        title.setString("Booking History");
        title.setCharacterSize(36);
        title.setStyle(Text::Bold);
        title.setFillColor(Color(100, 255, 255));
        title.setOutlineThickness(2);
        title.setOutlineColor(Color(0, 0, 0, 150));
        title.setPosition(800, 110);
        window.draw(title);

        if (bookingManager.head == nullptr)
        {
            Text noBookings;
            noBookings.setFont(font);
            noBookings.setString("No bookings found.");
            noBookings.setCharacterSize(24);
            noBookings.setFillColor(Color::Yellow);
            noBookings.setPosition(800, 500);
            window.draw(noBookings);
        }
        else
        {
            Booking *current = bookingManager.head;
            float yPos = 180;
            int count = 1;
            //* Will show max 8 bookings
            while (current != nullptr && count <= 8)
            {
                RectangleShape bookingBox(Vector2f(1300, 90));
                bookingBox.setFillColor(Color(0, 50, 100, 200));
                bookingBox.setOutlineThickness(2);
                bookingBox.setOutlineColor(Color(100, 200, 255));
                bookingBox.setPosition(300, yPos);
                window.draw(bookingBox);

                Text bookingText;
                bookingText.setFont(font);
                bookingText.setCharacterSize(18);
                bookingText.setFillColor(Color(200, 230, 255));
                bookingText.setPosition(320, yPos + 10);

                string info = "ID: " + current->bookingID +
                              "  |  Customer: " + current->customerName +
                              "  |  Route: " + current->origin + " " + current->destination +
                              "\nDate: " + current->departureDate +
                              "  |  Cost: $" + to_string(current->totalCost);

                bookingText.setString(info);
                window.draw(bookingText);

                current = current->next;
                yPos += 105;
                count++;
            }
        }

        RectangleShape backButton(Vector2f(150, 50));
        backButton.setFillColor(Color(180, 80, 80));
        backButton.setOutlineThickness(3);
        backButton.setOutlineColor(Color(255, 150, 150));
        backButton.setPosition(50, 950);

        Text backBtnText("BACK", font, 20);
        backBtnText.setStyle(Text::Bold);
        backBtnText.setPosition(85, 962);

        Vector2f mousePos = window.mapPixelToCoords(Mouse::getPosition(window));
        if (backButton.getGlobalBounds().contains(mousePos))
        {
            backButton.setFillColor(Color(220, 100, 100));
            if (Mouse::isButtonPressed(Mouse::Left))
            {
                window.close();
            }
        }
        else
        {
            backButton.setFillColor(Color(180, 80, 80));
        }

        window.draw(backButton);
        window.draw(backBtnText);

        Text instruction;
        instruction.setFont(font);
        instruction.setString("Press ESC or click BACK to return to menu");
        instruction.setCharacterSize(18);
        instruction.setFillColor(Color(150, 200, 255));
        instruction.setPosition(720, 1000);
        window.draw(instruction);

        window.display();
    }
}

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

int main()
{
    Font font;
    if (!font.loadFromFile("Roboto.ttf"))
    {
        cout << "Font loading failed!" << endl;
        return -1;
    }

    Maps graph = readPorts("PortCharges.txt");
    addRoutesFromFile(graph, "Routes.txt");

    bookingManager.loadFromFile();

    while (true)
    {
        int choice = graph.openMainMenu(font);

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
            exit(0);
        }
    }

    return 0;
}