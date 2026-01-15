#ifndef PORT_DOCKING_MANAGER_H
#define PORT_DOCKING_MANAGER_H

#include <iostream>
#include <string>
#include <cstdlib>
#include <ctime>
#include "shipdocking.h"
#include "ship.h"
#include "Vector.h"

using namespace std;


struct ShipAnimation {
    string shipId;
    string portName;
    float startX, startY;
    float endX, endY;
    float progress;      
    bool isArriving;     
    bool isComplete;
    string company;
    
    ShipAnimation() : shipId(""), portName(""), startX(0), startY(0), 
                     endX(0), endY(0), progress(0), isArriving(true), 
                     isComplete(false), company("") {}
};

class PortDockingManager {
private:
    static const int MAX_PORTS = 50;
    DockingShip ports[MAX_PORTS];
    int portCount;
    int shipIdCounter;
    
    
    Vector<ShipAnimation> activeAnimations;
    
    
    string companies[10] = {
        "MaerskLine", "MSC", "COSCO", "CMA_CGM", "Evergreen",
        "HapagLloyd", "ONE", "YangMing", "ZIM", "PIL"
    };
    
public:
    PortDockingManager() : portCount(0), shipIdCounter(1000) {
        srand(time(nullptr));
    }
    
    
    void initializePort(const string& portName, int dockCapacity) {
        if (portCount < MAX_PORTS) {
            ports[portCount] = DockingShip(portName, dockCapacity);
            portCount++;
        }
    }
    
    
    void initializeAllPorts(const string portNames[], int numPorts) {
        for (int i = 0; i < numPorts && portCount < MAX_PORTS; i++) {
            
            int dockCapacity = 3; 
            
            
            if (portNames[i] == "Singapore" || portNames[i] == "Shanghai" || 
                portNames[i] == "Rotterdam" || portNames[i] == "HongKong" ||
                portNames[i] == "Dubai" || portNames[i] == "LosAngeles") {
                dockCapacity = 6;
            } else if (portNames[i] == "Mumbai" || portNames[i] == "Karachi" ||
                       portNames[i] == "Tokyo" || portNames[i] == "Sydney" ||
                       portNames[i] == "NewYork" || portNames[i] == "Hamburg") {
                dockCapacity = 5;
            } else if (portNames[i] == "PortKlang" || portNames[i] == "Busan" ||
                       portNames[i] == "Colombo" || portNames[i] == "Jeddah") {
                dockCapacity = 4;
            }
            
            initializePort(portNames[i], dockCapacity);
        }
    }
    
    
    int findPortIndex(const string& portName) {
        for (int i = 0; i < portCount; i++) {
            if (ports[i].portName == portName) {
                return i;
            }
        }
        return -1;
    }
    
    
    DockingShip* getPort(const string& portName) {
        int idx = findPortIndex(portName);
        if (idx >= 0) {
            return &ports[idx];
        }
        return nullptr;
    }
    
    
    Ship* generateRandomShip(const string& currentPort, const string& destPort) {
        shipIdCounter++;
        string shipId = "SH-" + to_string(shipIdCounter);
        string shipName = "Cargo Vessel " + to_string(shipIdCounter % 100);
        string company = companies[rand() % 10];
        int cargoSize = 500 + rand() % 2000;
        int requiredHours = 2 + rand() % 6; 
        int arrivalTime = rand() % 24;
        
        Ship* ship = new Ship(shipId, shipName, company, currentPort, destPort, 
                              cargoSize, 0, arrivalTime, requiredHours);
        return ship;
    }
    
    
    bool shipArrival(const string& portName, Ship* ship, float portX, float portY) {
        int idx = findPortIndex(portName);
        if (idx >= 0) {
            ports[idx].arriveShip(ship);
            
            
            ShipAnimation anim;
            anim.shipId = ship->sID;
            anim.portName = portName;
            anim.company = ship->company;
            anim.isArriving = true;
            anim.progress = 0.0f;
            anim.isComplete = false;
            
            
            float angle = (rand() % 360) * 3.14159f / 180.0f;
            float distance = 150.0f;
            anim.startX = portX + cos(angle) * distance;
            anim.startY = portY + sin(angle) * distance;
            anim.endX = portX;
            anim.endY = portY;
            
            activeAnimations.push_back(anim);
            
            return true;
        }
        return false;
    }
    
    
    Ship* shipDeparture(const string& portName, float portX, float portY) {
        int idx = findPortIndex(portName);
        if (idx >= 0) {
            Ship* departing = ports[idx].departShip();
            if (departing) {
                
                ShipAnimation anim;
                anim.shipId = departing->sID;
                anim.portName = portName;
                anim.company = departing->company;
                anim.isArriving = false;
                anim.progress = 0.0f;
                anim.isComplete = false;
                
                
                float angle = (rand() % 360) * 3.14159f / 180.0f;
                float distance = 150.0f;
                anim.startX = portX;
                anim.startY = portY;
                anim.endX = portX + cos(angle) * distance;
                anim.endY = portY + sin(angle) * distance;
                
                activeAnimations.push_back(anim);
            }
            return departing;
        }
        return nullptr;
    }
    
    
    void simulateRandomArrival(const string portNames[], int numPorts, 
                               float portX[], float portY[]) {
        if (numPorts == 0) return;
        
        int randomPortIdx = rand() % numPorts;
        string portName = portNames[randomPortIdx];
        
        
        int destIdx = rand() % numPorts;
        while (destIdx == randomPortIdx) destIdx = rand() % numPorts;
        string destPort = portNames[destIdx];
        
        Ship* newShip = generateRandomShip(portName, destPort);
        shipArrival(portName, newShip, portX[randomPortIdx], portY[randomPortIdx]);
    }
    
    
    void simulateRandomDeparture(const string portNames[], int numPorts,
                                 float portX[], float portY[]) {
        if (numPorts == 0) return;
        
        
        for (int attempts = 0; attempts < 10; attempts++) {
            int randomPortIdx = rand() % numPorts;
            string portName = portNames[randomPortIdx];
            
            DockingShip* port = getPort(portName);
            if (port && port->getDockedCount() > 0) {
                Ship* departed = shipDeparture(portName, portX[randomPortIdx], portY[randomPortIdx]);
                if (departed) {
                    delete departed; 
                    return;
                }
            }
        }
    }
    
    
    void updateAnimations(float deltaTime) {
        float animSpeed = 0.8f; 
        
        for (int i = 0; i < activeAnimations.getSize(); i++) {
            if (!activeAnimations[i].isComplete) {
                activeAnimations[i].progress += deltaTime * animSpeed;
                if (activeAnimations[i].progress >= 1.0f) {
                    activeAnimations[i].progress = 1.0f;
                    activeAnimations[i].isComplete = true;
                }
            }
        }
        
        
        while (activeAnimations.getSize() > 20) {
            if (activeAnimations[0].isComplete) {
                
                Vector<ShipAnimation> temp;
                for (int i = 1; i < activeAnimations.getSize(); i++) {
                    temp.push_back(activeAnimations[i]);
                }
                activeAnimations = temp;
            } else {
                break;
            }
        }
    }
    
    
    Vector<ShipAnimation>& getActiveAnimations() {
        return activeAnimations;
    }
    
    
    void getPortStats(const string& portName, int& docked, int& total, 
                      int& queueLen, int& waitMinutes, string& recent) {
        DockingShip* port = getPort(portName);
        if (port) {
            docked = port->getDockedCount();
            total = port->totalDocks;
            queueLen = port->getQueueLength();
            waitMinutes = port->getEstimatedWaitMinutes();
            recent = port->recentActivity;
        } else {
            docked = 0;
            total = 3;
            queueLen = 0;
            waitMinutes = 0;
            recent = "";
        }
    }
    
    
    bool portExists(const string& portName) {
        return findPortIndex(portName) >= 0;
    }
    
    
    int getPortCount() { return portCount; }
    
    
    void clearAnimations() {
        while (activeAnimations.getSize() > 0) {
            activeAnimations.pop();
        }
    }
    
    
    void processTimeStep(float hours) {
        
        for (int i = 0; i < portCount; i++) {
            Ship* current = ports[i].dockedShips;
            Ship* prev = nullptr;
            
            while (current != nullptr) {
                current->requiredhours -= hours;
                if (current->requiredhours <= 0) {
                    
                    current->requiredhours = 0;
                }
                prev = current;
                current = current->next;
            }
            
            ports[i].updateQueueWaitTimes();
        }
    }
};

#endif
