#ifndef SHIPPREFERENCES_H
#define SHIPPREFERENCES_H

#include "Vector.h"
#include <string>
#include <climits>
#include <cstdio>

using namespace std;

class ShipPreferences
{
public:
    
    Vector<string> preferredCompanies;    
    Vector<string> avoidedPorts;          
    int maxVoyageMinutes;                 
    bool filterActive;                    

    ShipPreferences()
    {
        maxVoyageMinutes = 0;
        filterActive = false;
    }

    
    
    void addPreferredCompany(string company)
    {
        
        for (int i = 0; i < preferredCompanies.getSize(); i++)
        {
            if (preferredCompanies[i] == company)
                return;
        }
        preferredCompanies.push_back(company);
        filterActive = true;
    }

    void removePreferredCompany(string company)
    {
        Vector<string> newList;
        for (int i = 0; i < preferredCompanies.getSize(); i++)
        {
            if (preferredCompanies[i] != company)
            {
                newList.push_back(preferredCompanies[i]);
            }
        }
        
        while (preferredCompanies.getSize() > 0)
            preferredCompanies.pop();
        for (int i = 0; i < newList.getSize(); i++)
            preferredCompanies.push_back(newList[i]);
    }

    bool isCompanyPreferred(string company)
    {
        
        if (preferredCompanies.getSize() == 0)
            return true;
        
        for (int i = 0; i < preferredCompanies.getSize(); i++)
        {
            if (preferredCompanies[i] == company)
                return true;
        }
        return false;
    }

    
    
    void addAvoidedPort(string port)
    {
        
        for (int i = 0; i < avoidedPorts.getSize(); i++)
        {
            if (avoidedPorts[i] == port)
                return;
        }
        avoidedPorts.push_back(port);
        filterActive = true;
    }

    void removeAvoidedPort(string port)
    {
        Vector<string> newList;
        for (int i = 0; i < avoidedPorts.getSize(); i++)
        {
            if (avoidedPorts[i] != port)
            {
                newList.push_back(avoidedPorts[i]);
            }
        }
        
        while (avoidedPorts.getSize() > 0)
            avoidedPorts.pop();
        for (int i = 0; i < newList.getSize(); i++)
            avoidedPorts.push_back(newList[i]);
    }

    bool isPortAvoided(string port)
    {
        for (int i = 0; i < avoidedPorts.getSize(); i++)
        {
            if (avoidedPorts[i] == port)
                return true;
        }
        return false;
    }

    bool isPortAllowed(string port)
    {
        return !isPortAvoided(port);
    }

    
    
    void setMaxVoyageTime(int hours)
    {
        maxVoyageMinutes = hours * 60;
        if (hours > 0)
            filterActive = true;
    }

    void setMaxVoyageTimeMinutes(int minutes)
    {
        maxVoyageMinutes = minutes;
        if (minutes > 0)
            filterActive = true;
    }

    int getMaxVoyageMinutes()
    {
        return maxVoyageMinutes;
    }

    bool isWithinTimeLimit(int totalMinutes)
    {
        if (maxVoyageMinutes == 0)
            return true;  
        return totalMinutes <= maxVoyageMinutes;
    }

    
    
    void clearAllPreferences()
    {
        while (preferredCompanies.getSize() > 0)
            preferredCompanies.pop();
        
        while (avoidedPorts.getSize() > 0)
            avoidedPorts.pop();
        
        maxVoyageMinutes = 0;
        filterActive = false;
    }

    void clearCompanyPreferences()
    {
        while (preferredCompanies.getSize() > 0)
            preferredCompanies.pop();
    }

    void clearAvoidedPorts()
    {
        while (avoidedPorts.getSize() > 0)
            avoidedPorts.pop();
    }

    
    
    string getPreferencesSummary()
    {
        string summary = "";
        
        if (preferredCompanies.getSize() > 0)
        {
            summary += "Companies: ";
            for (int i = 0; i < preferredCompanies.getSize(); i++)
            {
                summary += preferredCompanies[i];
                if (i < preferredCompanies.getSize() - 1)
                    summary += ", ";
            }
            summary += "\n";
        }
        
        if (avoidedPorts.getSize() > 0)
        {
            summary += "Avoiding: ";
            for (int i = 0; i < avoidedPorts.getSize(); i++)
            {
                summary += avoidedPorts[i];
                if (i < avoidedPorts.getSize() - 1)
                    summary += ", ";
            }
            summary += "\n";
        }
        
        if (maxVoyageMinutes > 0)
        {
            int hours = maxVoyageMinutes / 60;
            int mins = maxVoyageMinutes % 60;
            char buf[50];
            sprintf(buf, "Max Time: %dh %dm\n", hours, mins);
            summary += buf;
        }
        
        if (summary.empty())
        {
            summary = "No preferences set";
        }
        
        return summary;
    }

    bool hasActiveFilters()
    {
        return preferredCompanies.getSize() > 0 || 
               avoidedPorts.getSize() > 0 || 
               maxVoyageMinutes > 0;
    }
};

#endif
