#ifndef DIJKSTRA_H
#define DIJKSTRA_H

#include "Vector.h"
#include "priorityqueue.h"
#include "ShipPreferences.h"
#include "shipdocking.h"
#include <limits>
#include <string>
#include <climits>

using namespace std;

extern int getPortDockWaitMinutes(int portIndex);
extern int getPortDockWaitMinutesByName(const string& portName);

class DijkstraAlgorithm
{
public:
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
        if (t.length() < 5) return 0;
        int h = (t[0] - '0') * 10 + (t[1] - '0');
        int m = (t[3] - '0') * 10 + (t[4] - '0');
        return h * 60 + m;
    }

    
    bool isLeapYear(int year)
    {
        return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
    }

    
    bool isValidDate(string date, string& errorMsg)
    {
        
        if (date.empty())
        {
            errorMsg = "Date cannot be empty";
            return false;
        }

        
        size_t slash1 = date.find('/');
        size_t slash2 = date.find('/', slash1 + 1);
        
        if (slash1 == string::npos || slash2 == string::npos)
        {
            errorMsg = "Invalid date format. Use DD/MM/YYYY";
            return false;
        }

        string dayStr = date.substr(0, slash1);
        string monthStr = date.substr(slash1 + 1, slash2 - slash1 - 1);
        string yearStr = date.substr(slash2 + 1);

        if (dayStr.empty() || monthStr.empty() || yearStr.empty())
        {
            errorMsg = "Invalid date format. Use DD/MM/YYYY";
            return false;
        }

        for (char c : dayStr)
        {
            if (!isdigit(c))
            {
                errorMsg = "Day must be numeric";
                return false;
            }
        }
        for (char c : monthStr)
        {
            if (!isdigit(c))
            {
                errorMsg = "Month must be numeric";
                return false;
            }
        }
        for (char c : yearStr)
        {
            if (!isdigit(c))
            {
                errorMsg = "Year must be numeric";
                return false;
            }
        }

        
        int day = stoi(dayStr);
        int month = stoi(monthStr);
        int year = stoi(yearStr);

        
        if (yearStr.length() != 4)
        {
            errorMsg = "Year must be 4 digits (YYYY)";
            return false;
        }
        if (year != 2024)
        {
            errorMsg = "Year must be 2024";
            return false;
        }

        
        if (month < 1 || month > 12)
        {
            errorMsg = "Month must be between 1 and 12";
            return false;
        }

        
        int daysInMonth[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
        
        
        if (isLeapYear(year))
        {
            daysInMonth[2] = 29;
        }

        if (day < 1 || day > daysInMonth[month])
        {
            errorMsg = "Invalid day for the given month";
            return false;
        }

        return true;
    }

    
    string normalizeDate(string date)
    {
        if (date.length() == 10 && date[2] == '/' && date[5] == '/')
        {
            return date; 
        }

        size_t s1 = date.find('/');
        if (s1 == string::npos) return date;

        size_t s2 = date.find('/', s1 + 1);
        if (s2 == string::npos) return date;

        string d = date.substr(0, s1);
        string m = date.substr(s1 + 1, s2 - s1 - 1);
        string y = date.substr(s2 + 1);

        if (d.length() == 1) d = "0" + d;
        if (m.length() == 1) m = "0" + m;

        return d + "/" + m + "/" + y;
    }

    bool isDateGreaterOrEqual(string date1, string date2)
    {
        string norm1 = normalizeDate(date1);
        string norm2 = normalizeDate(date2);

        int d1 = stoi(norm1.substr(0,2));
        int m1 = stoi(norm1.substr(3,2));
        int y1 = stoi(norm1.substr(6,4));

        int d2 = stoi(norm2.substr(0,2));
        int m2 = stoi(norm2.substr(3,2));
        int y2 = stoi(norm2.substr(6,4));

        if(y1 != y2) return y1 > y2;
        if(m1 != m2) return m1 > m2;
        return d1 >= d2;
    }

    int calculateDaysBetween(string date1, string date2)
    {
        string norm1 = normalizeDate(date1);
        string norm2 = normalizeDate(date2);

        int d1 = stoi(norm1.substr(0,2));
        int m1 = stoi(norm1.substr(3,2));
        int y1 = stoi(norm1.substr(6,4));

        int d2 = stoi(norm2.substr(0,2));
        int m2 = stoi(norm2.substr(3,2));
        int y2 = stoi(norm2.substr(6,4));

        int total1 = y1*365 + m1*30 + d1;
        int total2 = y2*365 + m2*30 + d2;
        return total2 - total1;
    }

    int calculateTravelTime(string dep, string arr, bool &crossesMidnight)
    {
        int depMin = toMinutes(dep);
        int arrMin = toMinutes(arr);

        if(arrMin >= depMin)
        {
            crossesMidnight = false;
            return arrMin - depMin;
        }
        else
        {
            crossesMidnight = true;
            return (24*60 - depMin) + arrMin;
        }
    }

    int calculateTravelTime(string dep, string arr)
    {
        bool dummy;
        return calculateTravelTime(dep, arr, dummy);
    }

    string addDaysToDate(string date, int days)
    {
        string norm = normalizeDate(date);
        int day = stoi(norm.substr(0,2));
        int month = stoi(norm.substr(3,2));
        int year = stoi(norm.substr(6,4));

        int daysInMonth[] = {31,28,31,30,31,30,31,31,30,31,30,31};
        if ((year%4==0 && year%100!=0) || year%400==0)
            daysInMonth[1] = 29;

        day += days;
        while(day > daysInMonth[month-1])
        {
            day -= daysInMonth[month-1];
            month++;
            if(month>12)
            {
                month = 1;
                year++;
                if ((year%4==0 && year%100!=0) || year%400==0)
                    daysInMonth[1] = 29;
                else
                    daysInMonth[1] = 28;
            }
        }

        string dayStr = (day < 10 ? "0" : "") + to_string(day);
        string monthStr = (month < 10 ? "0" : "") + to_string(month);
        string yearStr = to_string(year);

        return dayStr + "/" + monthStr + "/" + yearStr;
    }

    Vector<int> findCheapestPath(Vector<PortNode>& ports, int origin, int destination,
                                 string startDate = "01/01/2024", ShipPreferences* prefs = nullptr, int defaultDocks = 2)
    {
        int n = ports.getSize();
        

        struct PortState {
            float cost;
            int totalMinutes;
            string arrivalDate;
            string arrivalTime;
            int prevPort;
            bool visited;

            PortState() : cost(numeric_limits<float>::infinity()), totalMinutes(0), prevPort(-1), visited(false) {}
        };

        Vector<PortState> states(n);
        PriorityQueue pq;

        if (origin == -1 || destination == -1 || origin >= n || destination >= n)
            return Vector<int>();

        states[origin].cost = 0;
        states[origin].totalMinutes = 0;
        states[origin].arrivalDate = startDate;
        states[origin].arrivalTime = "00:00";
        pq.enqueue(origin, 0);

        int iterations = 0;
        while(!pq.isEmpty() && iterations<50000)
        {
            iterations++;
            PriorityQueue::Node current = pq.front();
            pq.dequeue();
            int u = current.index;

            
            int layoverMinutes = getPortDockWaitMinutes(u);

            if(states[u].visited) continue;
            states[u].visited = true;
            if(u == destination) break;

            RouteNode* route = ports[u].routeHead;
            while(route != nullptr)
            {
                int v = -1;
                for(int i=0;i<n;i++)
                {
                    if(ports[i].portName == route->destinationPort)
                    {
                        v = i;
                        break;
                    }
                }
                if(v==-1 || states[v].visited){ route = route->next; continue; }

                if(prefs!=nullptr && prefs->filterActive)
                {
                    if(!prefs->isCompanyPreferred(route->shippingCompany)){ route = route->next; continue; }
                    if(prefs->isPortAvoided(route->destinationPort)){ route = route->next; continue; }
                }

                string routeDate = route->departureDate;
                if(!isDateGreaterOrEqual(routeDate, states[u].arrivalDate)){ route = route->next; continue; }

                int waitMinutes = 0;
                int waitDays = 0;
                bool canTakeRoute = false;

                if(routeDate == states[u].arrivalDate)
                {
                    int arrivalMin = toMinutes(states[u].arrivalTime);
                    int departureMin = toMinutes(route->departureTime);
                    if(departureMin >= arrivalMin+120)
                    {
                        waitMinutes = departureMin - arrivalMin;
                        waitDays = 0;
                        canTakeRoute = true;
                    }
                }
                else
                {
                    waitDays = calculateDaysBetween(states[u].arrivalDate, routeDate);
                    int arrivalMin = toMinutes(states[u].arrivalTime);
                    int departureMin = toMinutes(route->departureTime);
                    waitMinutes = (24*60 - arrivalMin) + (waitDays-1)*24*60 + departureMin;
                    canTakeRoute = true;
                }

                if(!canTakeRoute){ route = route->next; continue; }

                bool crossesMidnight = false;
                int travelMinutes = calculateTravelTime(route->departureTime, route->arrivalTime, crossesMidnight);
                int totalWaitMinutes = waitMinutes + layoverMinutes;
                string actualArrivalDate = routeDate;
                if(crossesMidnight) actualArrivalDate = addDaysToDate(routeDate, 1);

                float additionalCost = route->cost;
                
                if(waitMinutes>0) additionalCost += ports[u].charge * (waitMinutes / 1440.0f);
                
                
                
                if(layoverMinutes>0) additionalCost += ports[u].charge * (layoverMinutes / 1440.0f);

                float newCost = states[u].cost + additionalCost;
                int newTotalMinutes = states[u].totalMinutes + totalWaitMinutes + travelMinutes;

                if(prefs!=nullptr && prefs->maxVoyageMinutes>0)
                    if(newTotalMinutes>prefs->maxVoyageMinutes){ route = route->next; continue; }

                if(newCost < states[v].cost)
                {
                    states[v].cost = newCost;
                    states[v].totalMinutes = newTotalMinutes;
                    states[v].arrivalDate = actualArrivalDate;
                    states[v].arrivalTime = route->arrivalTime;
                    states[v].prevPort = u;
                    pq.enqueue(v, newCost);
                }

                route = route->next;
            }
        }

        Vector<int> path;
        if(states[destination].cost == numeric_limits<float>::infinity()) return path;

        int current = destination;
        while(current != -1)
        {
            path.push_back(current);
            current = states[current].prevPort;
        }

        reversePath(path);
        return path;
    }

    
    Vector<int> findFastestPath(Vector<PortNode>& ports, int origin, int destination,
                                string startDate = "01/01/2024", ShipPreferences* prefs = nullptr)
    {
        int n = ports.getSize();
        struct PortState {
            int totalMinutes;
            string arrivalDate;
            string arrivalTime;
            int prevPort;
            bool visited;

            PortState() : totalMinutes(INT_MAX), prevPort(-1), visited(false) {}
        };

        Vector<PortState> states(n);
        PriorityQueue pq;

        if(origin==-1 || destination==-1 || origin>=n || destination>=n) return Vector<int>();

        states[origin].totalMinutes = 0;
        states[origin].arrivalDate = startDate;
        states[origin].arrivalTime = "00:00";
        pq.enqueue(origin, 0);

        int iterations = 0;
        while(!pq.isEmpty() && iterations<50000)
        {
            iterations++;
            PriorityQueue::Node current = pq.front();
            pq.dequeue();
            int u = current.index;

            if(states[u].visited) continue;
            states[u].visited = true;
            if(u==destination) break;

            RouteNode* route = ports[u].routeHead;
            while(route!=nullptr)
            {
                int v=-1;
                for(int i=0;i<n;i++)
                {
                    if(ports[i].portName==route->destinationPort){ v=i; break; }
                }
                if(v==-1 || states[v].visited){ route=route->next; continue; }

                if(prefs!=nullptr && prefs->filterActive)
                {
                    if(!prefs->isCompanyPreferred(route->shippingCompany)){ route=route->next; continue; }
                    if(prefs->isPortAvoided(route->destinationPort)){ route=route->next; continue; }
                }

                string routeDate = route->departureDate;
                if(!isDateGreaterOrEqual(routeDate, states[u].arrivalDate)){ route=route->next; continue; }

                int waitMinutes=0;
                bool canTakeRoute=false;

                if(routeDate==states[u].arrivalDate)
                {
                    int arrivalMin = toMinutes(states[u].arrivalTime);
                    int departureMin = toMinutes(route->departureTime);
                    if(departureMin >= arrivalMin+120){ waitMinutes = departureMin-arrivalMin; canTakeRoute=true; }
                }
                else
                {
                    int daysBetween = calculateDaysBetween(states[u].arrivalDate, routeDate);
                    int arrivalMin = toMinutes(states[u].arrivalTime);
                    int departureMin = toMinutes(route->departureTime);
                    waitMinutes = (24*60 - arrivalMin) + (daysBetween-1)*24*60 + departureMin;
                    canTakeRoute=true;
                }

                if(!canTakeRoute){ route=route->next; continue; }

                bool crossesMidnight=false;
                int travelMinutes = calculateTravelTime(route->departureTime, route->arrivalTime, crossesMidnight);
                string actualArrivalDate = routeDate;
                if(crossesMidnight) actualArrivalDate = addDaysToDate(routeDate,1);

                int newTotalMinutes = states[u].totalMinutes + waitMinutes + travelMinutes;
                if(prefs!=nullptr && prefs->maxVoyageMinutes>0 && newTotalMinutes>prefs->maxVoyageMinutes){ route=route->next; continue; }

                if(newTotalMinutes < states[v].totalMinutes)
                {
                    states[v].totalMinutes = newTotalMinutes;
                    states[v].arrivalDate = actualArrivalDate;
                    states[v].arrivalTime = route->arrivalTime;
                    states[v].prevPort = u;
                    pq.enqueue(v, newTotalMinutes);
                }

                route = route->next;
            }
        }

        Vector<int> path;
        if(states[destination].totalMinutes==INT_MAX) return path;

        int current = destination;
        while(current!=-1){ path.push_back(current); current=states[current].prevPort; }

        reversePath(path);
        return path;
    }
};

#endif
