#ifndef DOCKINGSTRUCTURES_H
#define DOCKINGSTRUCTURES_H

#include <string>
#include "Vector.h"
using namespace std;


struct GlobalDockShipInfo 
{
    string shipId;
    string company;
    float dockTimer;
    float maxTime;
    int dockSlot;
};


struct GlobalQueueShipInfo 
{
    string shipId;
    string company;
    int queuePosition;
};


struct PortDockingState 
{
    Vector<GlobalDockShipInfo> dockedShips;
    Vector<GlobalQueueShipInfo> queuedShips;
    int maxDocks;
    int maxQueue;
    
    PortDockingState() : maxDocks(4), maxQueue(6) {}
    
    int getOccupiedDocks() const 
    { 
        return dockedShips.getSize(); 
    }
    
    
    int getQueueLength() const 
    { 
        return queuedShips.getSize(); 
    }
    
    
    bool hasFreeSlot() const 
    { 
        return dockedShips.getSize() < maxDocks; 
    }
    
    
    int calculateWaitTimeMinutes() const 
    {
        if (dockedShips.getSize() < maxDocks) 
            return 0;
        float minTimeToFree = 999999.0f;
        for (int i = 0; i < dockedShips.getSize(); i++) 
            if (dockedShips[i].dockTimer < minTimeToFree) 
                minTimeToFree = dockedShips[i].dockTimer;
        int queueWaitMinutes = queuedShips.getSize() * 2 * 8 * 60;
        int dockFreeMinutes = static_cast<int>(minTimeToFree / 60.0f);
        return dockFreeMinutes + queueWaitMinutes;
    }
    
    
    int findFreeSlot() const 
    {
        for (int slot = 0; slot < maxDocks; slot++) 
        {
            bool used = false;
            for (int d = 0; d < dockedShips.getSize(); d++) 
                if (dockedShips[d].dockSlot == slot) 
                { 
                    used = true; 
                    break; 
                }
            if (!used) 
                return slot;
        }
        return -1;
    }
};

#endif
