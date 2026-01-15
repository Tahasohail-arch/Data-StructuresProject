#ifndef ASTAR_H
#define ASTAR_H

#include "Vector.h"
#include "priorityqueue.h"
#include "ShipPreferences.h"
#include "shipdocking.h"
#include <cmath>
#include <limits>
#include <string>
#include <climits>

using namespace std;


extern int getPortDockWaitMinutes(int portIndex);
extern int getPortDockWaitMinutesByName(const string& portName);

extern PortLocation portLocations[];
extern const int numLocations;

class AStar
{
private:
    int toMinutes(string t)
    {
        if (t.length() < 5) return 0;
        int h = (t[0] - '0') * 10 + (t[1] - '0');
        int m = (t[3] - '0') * 10 + (t[4] - '0');
        return h * 60 + m;
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

    void reversePath(Vector<int>& temp)
    {
        int left = 0, right = temp.getSize() - 1;
        while(left < right)
        {
            int t = temp[left];
            temp[left] = temp[right];
            temp[right] = t;
            left++;
            right--;
        }
    }

    bool getCoordinates(const string &name, float &x, float &y)
    {
        for(int i = 0; i < numLocations; i++)
        {
            if(portLocations[i].name == name)
            {
                x = portLocations[i].x;
                y = portLocations[i].y;
                return true;
            }
        }
        return false;
    }

    float heuristic(const string &a, const string &b)
    {
        float x1, y1, x2, y2;
        if(!getCoordinates(a, x1, y1)) return 0;
        if(!getCoordinates(b, x2, y2)) return 0;
        float dx = x1 - x2, dy = y1 - y2;
        return sqrt(dx*dx + dy*dy);
    }

public:
    Vector<int> findAStarPathCost(Vector<PortNode>& ports, int origin, int destination, string userDate, ShipPreferences* prefs = nullptr)
    {
        int n = ports.getSize();

        Vector<float> gScore;
        Vector<float> fScore;
        Vector<int> cameFrom;
        Vector<bool> visited;
        Vector<string> currentDates;
        Vector<string> currentTimes;
        Vector<int> totalMinutes;

        for(int i=0;i<n;i++)
        {
            gScore.push_back(numeric_limits<float>::infinity());
            fScore.push_back(numeric_limits<float>::infinity());
            cameFrom.push_back(-1);
            visited.push_back(false);
            currentDates.push_back("");
            currentTimes.push_back("");
            totalMinutes.push_back(0);
        }

        

        if(origin==-1 || destination==-1 || origin>=n || destination>=n) return Vector<int>();

        PriorityQueue PQ;

        gScore[origin] = 0;
        currentDates[origin] = userDate;
        currentTimes[origin] = "00:00";
        totalMinutes[origin] = 0;
        fScore[origin] = heuristic(ports[origin].portName, ports[destination].portName)*10;

        PQ.enqueue(origin,fScore[origin]);

        int iterations=0;
        while(!PQ.isEmpty() && iterations<50000)
        {
            iterations++;
            PriorityQueue::Node current = PQ.front();
            PQ.dequeue();

            int u = current.index;

            
            int layoverMinutes = getPortDockWaitMinutes(u);

            if(visited[u]) continue;
            visited[u] = true;

            if(u==destination) break;

            RouteNode* route = ports[u].routeHead;
            while(route!=nullptr)
            {
                int v=-1;
                for(int i=0;i<n;i++)
                {
                    if(ports[i].portName==route->destinationPort)
                    {
                        v=i;
                        break;
                    }
                }
                if(v==-1 || visited[v]) {route=route->next; continue;}

                if(prefs!=nullptr && prefs->filterActive)
                {
                    if(!prefs->isCompanyPreferred(route->shippingCompany)) {route=route->next; continue;}
                    if(prefs->isPortAvoided(route->destinationPort)) {route=route->next; continue;}
                }

                string routeDate = route->departureDate;

                if(!isDateGreaterOrEqual(routeDate,currentDates[u])) {route=route->next; continue;}

                int waitMinutes=0;
                int waitDays=0;
                bool canTakeRoute=false;

                if(routeDate==currentDates[u])
                {
                    int arrivalMin = toMinutes(currentTimes[u]);
                    int departureMin = toMinutes(route->departureTime);
                    if(departureMin >= arrivalMin+120) {waitMinutes=departureMin-arrivalMin; canTakeRoute=true;}
                }
                else
                {
                    waitDays = calculateDaysBetween(currentDates[u], routeDate);
                    int arrivalMin = toMinutes(currentTimes[u]);
                    int departureMin = toMinutes(route->departureTime);
                    waitMinutes = (24*60 - arrivalMin) + (waitDays-1)*24*60 + departureMin;
                    canTakeRoute=true;
                }

                if(!canTakeRoute) {route=route->next; continue;}

                bool crossesMidnight=false;
                int travelMinutes = calculateTravelTime(route->departureTime, route->arrivalTime, crossesMidnight);

                string actualArrivalDate = routeDate;
                if(crossesMidnight) actualArrivalDate = addDaysToDate(routeDate,1);

                int totalWaitMinutes = waitMinutes + layoverMinutes;
                int newTotalMinutes = totalMinutes[u] + totalWaitMinutes + travelMinutes;

                if(prefs!=nullptr && prefs->maxVoyageMinutes>0 && newTotalMinutes>prefs->maxVoyageMinutes)
                {
                    route=route->next;
                    continue;
                }

                float additionalCost = route->cost;
                
                if(waitMinutes>0) additionalCost += ports[u].charge*(waitMinutes/1440.0f);
                
                
                
                if(layoverMinutes>0) additionalCost += ports[u].charge*(layoverMinutes/1440.0f);

                float tentative_g = gScore[u]+additionalCost;
                if(tentative_g < gScore[v])
                {
                    gScore[v] = tentative_g;
                    cameFrom[v] = u;
                    currentDates[v] = actualArrivalDate;
                    currentTimes[v] = route->arrivalTime;
                    totalMinutes[v] = newTotalMinutes;
                    float h = heuristic(ports[v].portName, ports[destination].portName)*10;
                    fScore[v] = tentative_g+h;
                    PQ.enqueue(v,fScore[v]);
                }

                route=route->next;
            }
        }

        Vector<int> path;
        if(gScore[destination]==numeric_limits<float>::infinity()) return path;

        int curr = destination;
        while(curr!=-1) {path.push_back(curr); curr=cameFrom[curr];}
        reversePath(path);
        return path;
    }

    Vector<int> findAStarPathTime(Vector<PortNode>& ports, int origin, int destination, string userDate, ShipPreferences* prefs = nullptr)
    {
        int n = ports.getSize();

        Vector<float> gScore;
        Vector<float> fScore;
        Vector<int> cameFrom;
        Vector<bool> visited;
        Vector<string> currentDates;
        Vector<string> currentTimes;

        for(int i=0;i<n;i++)
        {
            gScore.push_back(numeric_limits<float>::infinity());
            fScore.push_back(numeric_limits<float>::infinity());
            cameFrom.push_back(-1);
            visited.push_back(false);
            currentDates.push_back("");
            currentTimes.push_back("");
        }

        if(origin==-1 || destination==-1 || origin>=n || destination>=n) return Vector<int>();

        PriorityQueue PQ;

        gScore[origin] = 0;
        currentDates[origin] = userDate;
        currentTimes[origin] = "00:00";
        fScore[origin] = heuristic(ports[origin].portName, ports[destination].portName)*0.5;

        PQ.enqueue(origin,fScore[origin]);

        int iterations=0;
        while(!PQ.isEmpty() && iterations<50000)
        {
            iterations++;
            PriorityQueue::Node current = PQ.front();
            PQ.dequeue();

            int u = current.index;
            
            int layoverMinutes = getPortDockWaitMinutes(u);

            if(visited[u]) continue;
            visited[u]=true;

            if(u==destination) break;

            RouteNode* route = ports[u].routeHead;
            while(route!=nullptr)
            {
                int v=-1;
                for(int i=0;i<n;i++)
                {
                    if(ports[i].portName==route->destinationPort)
                    {
                        v=i; break;
                    }
                }
                if(v==-1 || visited[v]) {route=route->next; continue;}

                if(prefs!=nullptr && prefs->filterActive)
                {
                    if(!prefs->isCompanyPreferred(route->shippingCompany)) {route=route->next; continue;}
                    if(prefs->isPortAvoided(route->destinationPort)) {route=route->next; continue;}
                }

                string routeDate = route->departureDate;
                if(!isDateGreaterOrEqual(routeDate,currentDates[u])) {route=route->next; continue;}

                int waitMinutes=0;
                bool canTakeRoute=false;

                if(routeDate==currentDates[u])
                {
                    int arrivalMin = toMinutes(currentTimes[u]);
                    int departureMin = toMinutes(route->departureTime);
                    if(departureMin >= arrivalMin+120) {waitMinutes=departureMin-arrivalMin; canTakeRoute=true;}
                }
                else
                {
                    int daysBetween = calculateDaysBetween(currentDates[u], routeDate);
                    int arrivalMin = toMinutes(currentTimes[u]);
                    int departureMin = toMinutes(route->departureTime);
                    waitMinutes=(24*60 - arrivalMin)+(daysBetween-1)*24*60+departureMin;
                    canTakeRoute=true;
                }

                if(!canTakeRoute) {route=route->next; continue;}

                bool crossesMidnight=false;
                int travelMinutes = calculateTravelTime(route->departureTime, route->arrivalTime, crossesMidnight);

                string actualArrivalDate = routeDate;
                if(crossesMidnight) actualArrivalDate=addDaysToDate(routeDate,1);

                
                float tentative_g = gScore[u]+waitMinutes+travelMinutes+layoverMinutes;

                if(prefs!=nullptr && prefs->maxVoyageMinutes>0 && tentative_g>prefs->maxVoyageMinutes)
                {
                    route=route->next; continue;
                }

                if(tentative_g<gScore[v])
                {
                    gScore[v]=tentative_g;
                    cameFrom[v]=u;
                    currentDates[v]=actualArrivalDate;
                    currentTimes[v]=route->arrivalTime;
                    float h = heuristic(ports[v].portName, ports[destination].portName)*0.5;
                    fScore[v]=tentative_g+h;
                    PQ.enqueue(v,fScore[v]);
                }

                route=route->next;
            }
        }

        Vector<int> path;
        if(gScore[destination]==numeric_limits<float>::infinity()) return path;

        int curr = destination;
        while(curr!=-1) {path.push_back(curr); curr=cameFrom[curr];}
        reversePath(path);
        return path;
    }

    Vector<int> findAStarPath(Vector<PortNode>& ports, int origin, int destination)
    {
        return findAStarPathCost(ports, origin, destination, "01/01/2024");
    }
};

#endif
