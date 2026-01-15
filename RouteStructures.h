#ifndef ROUTESTRUCTURES_H
#define ROUTESTRUCTURES_H

#include <string>
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

#endif
