#ifndef SHIP_H
#define SHIP_H
#include <iostream>
using namespace std;

struct Ship{
    string sID;
    string sName;
    string company;
    string currentPort;
    string destinationPort;
    int cargosize;
    int status;          
    int arrivaltime;     
    int requiredhours;   
    int waitTimeMinutes; 
    Ship *next;

    
    Ship() {
        sID = "";
        sName = "";
        company = "";
        currentPort = "";
        destinationPort = "";
        cargosize = 0;
        status = 0;
        arrivaltime = 0;
        requiredhours = 2;
        waitTimeMinutes = 0;
        next = nullptr;
    }

    
    Ship(string id, string name, string comp, string cPort, string dPort, int csize, int st, int atime, int rtime){
        sID = id;
        sName = name;
        company = comp;
        currentPort = cPort;
        destinationPort = dPort;
        cargosize = csize;
        status = st;
        arrivaltime = atime;
        requiredhours = rtime;
        waitTimeMinutes = 0;
        next = nullptr;
    }
};

#endif