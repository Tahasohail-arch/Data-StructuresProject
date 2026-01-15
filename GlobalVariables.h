#ifndef GLOBALVARIABLES_H
#define GLOBALVARIABLES_H

#include <SFML/Audio.hpp>
#include "RouteStructures.h"
#include "DockingStructures.h"
#include "ShipPreferences.h"


extern PortLocation portLocations[];
extern const int numLocations;


extern sf::SoundBuffer clickSoundBuffer;
extern sf::Sound clickSound;


extern sf::SoundBuffer transitionSoundBuffer;
extern sf::Sound transitionSound;


inline void playClickSound() {
    clickSound.play();
}


inline void playTransitionSound() {
    transitionSound.play();
}


extern PortDockingState globalPortDocking[40];


extern ShipPreferences shipPrefs;


inline int getPortDockWaitMinutes(int portIndex) 
{
    if (portIndex < 0 || portIndex >= numLocations) 
        return 0;
    return globalPortDocking[portIndex].calculateWaitTimeMinutes();
}

inline int getPortDockWaitMinutesByName(const string& portName) 
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

#endif
