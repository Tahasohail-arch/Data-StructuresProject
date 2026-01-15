#ifndef SHIPDOCKING_H
#define SHIPDOCKING_H

#include <iostream>
#include <string>
#include "dockingqueue.h"
#include "ship.h"
using namespace std;

class DockingShip {
public:
    string portName;
    int totalDocks;
    int availableDocks;
    DockingQueue waitingQueue;
    Ship *dockedShips;
    string recentActivity;      
    int totalProcessed;         
    
    DockingShip() : portName(""), totalDocks(0), availableDocks(0), dockedShips(nullptr), 
                   recentActivity(""), totalProcessed(0) {}
    
    DockingShip(string name, int docks) {
        portName = name;
        totalDocks = docks;
        availableDocks = docks;
        dockedShips = nullptr;
        recentActivity = "";
        totalProcessed = 0;
    }
    
    
    int calculateWaitTime(int queuePosition) {
        int waitMinutes = 0;
        int shipsAhead = queuePosition - 1;
        
        
        
        if (totalDocks > 0) {
            int avgServiceTime = 180; 
            waitMinutes = (shipsAhead / totalDocks) * avgServiceTime;
            
            
            Ship* current = dockedShips;
            int minRemainingTime = 9999;
            while (current != nullptr) {
                int remaining = current->requiredhours * 60; 
                if (remaining < minRemainingTime) {
                    minRemainingTime = remaining;
                }
                current = current->next;
            }
            if (minRemainingTime < 9999 && availableDocks == 0) {
                waitMinutes += minRemainingTime;
            }
        }
        
        return waitMinutes;
    }
    
    
    void updateQueueWaitTimes() {
        Ship* current = waitingQueue.front;
        int position = 1;
        while (current != nullptr) {
            current->waitTimeMinutes = calculateWaitTime(position);
            position++;
            current = current->next;
        }
    }
    
    void arriveShip(Ship *ship) {
        ship->currentPort = portName;
        
        if (availableDocks > 0) {
            availableDocks--;
            ship->status = 1;  
            ship->waitTimeMinutes = 0;
            
            ship->next = dockedShips;
            dockedShips = ship;
            
            recentActivity = ship->sID + " DOCKED";
            cout << "Ship " << ship->sID << " DOCKED at " << portName 
                 << " (Docks left: " << availableDocks << "/" << totalDocks << ")" << endl;
        } else {
            waitingQueue.enqueue(ship);
            ship->status = 2;  
            
            int queuePos = waitingQueue.getSize();
            ship->waitTimeMinutes = calculateWaitTime(queuePos);
            
            recentActivity = ship->sID + " QUEUED #" + to_string(queuePos);
            cout << "Ship " << ship->sID << " in QUEUE at " << portName 
                 << " (Position: " << queuePos << ", Est. Wait: " 
                 << ship->waitTimeMinutes << " min)" << endl;
        }
        
        updateQueueWaitTimes();
    }
    
    Ship* departShip() {
        if (dockedShips != nullptr) {
            Ship* departing = dockedShips;
            dockedShips = dockedShips->next;
            departing->next = nullptr;
            
            recentActivity = departing->sID + " DEPARTED";
            cout << "Ship " << departing->sID << " DEPARTED from " << portName << endl;
            
            totalProcessed++;
            availableDocks++;
            
            
            if (!waitingQueue.isEmpty()) {
                Ship* nextShip = waitingQueue.dequeue();
                nextShip->status = 1;  
                nextShip->waitTimeMinutes = 0;
                
                nextShip->next = dockedShips;
                dockedShips = nextShip;
                availableDocks--;
                
                recentActivity = nextShip->sID + " MOVED to DOCK";
                cout << "Ship " << nextShip->sID << " MOVED from queue to DOCK" << endl;
                
                updateQueueWaitTimes();
            }
            
            return departing;
        } else {
            cout << "No ships docked at " << portName << endl;
            return nullptr;
        }
    }
    
    int getDockedCount() {
        int count = 0;
        Ship* current = dockedShips;
        while (current != nullptr) {
            count++;
            current = current->next;
        }
        return count;
    }
    
    int getQueueLength() {
        return waitingQueue.getSize();
    }
    
    int getEstimatedWaitMinutes() {
        if (waitingQueue.isEmpty()) return 0;
        Ship* last = waitingQueue.rear;
        return last ? last->waitTimeMinutes : 0;
    }
    
    
    Ship* getShipAt(int position, bool inQueue) {
        if (inQueue) {
            Ship* current = waitingQueue.front;
            int pos = 0;
            while (current != nullptr && pos < position) {
                current = current->next;
                pos++;
            }
            return current;
        } else {
            Ship* current = dockedShips;
            int pos = 0;
            while (current != nullptr && pos < position) {
                current = current->next;
                pos++;
            }
            return current;
        }
    }
    
    ~DockingShip() {
        while (dockedShips != nullptr) {
            Ship* temp = dockedShips;
            dockedShips = dockedShips->next;
            delete temp;
        }
    }
};

#endif