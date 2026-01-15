#ifndef BOOKINGINTERFACE_H
#define BOOKINGINTERFACE_H

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <cmath>
#include <iostream>
#include "Dijkstra.h"
#include "ShipPreferences.h"
//#include "Maps.h"
#include "booking.h"
#include "DockingStructures.h"

using namespace sf;
using namespace std;

extern ShipPreferences shipPrefs;
extern BookingManager bookingManager;
extern const int numLocations;
extern PortLocation portLocations[];
extern Sound hornSound;
extern PortDockingState globalPortDocking[40];
int getPortDockWaitMinutes(int portIndex);


void openShipPreferencesPopup(Font &font);


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
    bgSprite.setColor(Color(80, 40, 60, 255)); 

    Clock clock;
    float time = 0.0f;
    float stateTransition = 1.0f;
    float transitionSpeed = 3.0f;

    enum BookingState { ENTER_DETAILS, SELECT_ROUTE, DISPLAY_ROUTE, SUCCESS };
    BookingState currentState = ENTER_DETAILS;

    Vector<int> currentPath;
    Vector<int> directPath;
    Vector<string> waitPorts;
    Vector<string> waitDurations;
    string searchResultText = "";
    Color searchResultColor = Color::White;
    string errorDetails = "";
    bool routeFound = false;
    bool hasBothRoutes = false;
    int selectedRoute = 0;

   
    struct RouteOption {
        Vector<int> path;
        int totalCost;
        int totalTime; 
        string routeType; 
        string description;
    };
    Vector<RouteOption> availableRoutes;
    int hoveredRoute = -1;

    Booking currentBooking;

    int currentSegment = 0;
    float segmentProgress = 0.0f;
    float animationSpeed = 0.8f;
    bool isPaused = false;
    float pauseTimer = 0.0f;
    const float SEGMENT_PAUSE = 1.5f;
    bool animationStarted = false;
    bool useCheapest = true;

    InputBox nameInput(font, "Customer Name", Vector2f(560, 240));
    InputBox originInput(font, "Origin Port", Vector2f(560, 360));
    InputBox destInput(font, "Destination Port", Vector2f(560, 480));
    InputBox dateInput(font, "Departure Date (DD/MM/YYYY)", Vector2f(560, 600));

    nameInput.box.setSize(Vector2f(800, 70));
    originInput.box.setSize(Vector2f(800, 70));
    destInput.box.setSize(Vector2f(800, 70));
    dateInput.box.setSize(Vector2f(800, 70));
    
    nameInput.box.setFillColor(Color(30, 10, 20, 230));
    originInput.box.setFillColor(Color(30, 10, 20, 230));
    destInput.box.setFillColor(Color(30, 10, 20, 230));
    dateInput.box.setFillColor(Color(30, 10, 20, 230));
    
    nameInput.box.setOutlineColor(Color(200, 50, 50, 200));
    originInput.box.setOutlineColor(Color(200, 50, 50, 200));
    destInput.box.setOutlineColor(Color(200, 50, 50, 200));
    dateInput.box.setOutlineColor(Color(200, 50, 50, 200));
    
    nameInput.box.setOutlineThickness(3);
    originInput.box.setOutlineThickness(3);
    destInput.box.setOutlineThickness(3);
    dateInput.box.setOutlineThickness(3);
    
    nameInput.labelText.setFillColor(Color(255, 100, 100));
    originInput.labelText.setFillColor(Color(255, 100, 100));
    destInput.labelText.setFillColor(Color(255, 100, 100));
    dateInput.labelText.setFillColor(Color(255, 100, 100));
    nameInput.labelText.setCharacterSize(20);
    originInput.labelText.setCharacterSize(20);
    destInput.labelText.setCharacterSize(20);
    dateInput.labelText.setCharacterSize(20);
    
    nameInput.text.setFillColor(Color::White);
    originInput.text.setFillColor(Color::White);
    destInput.text.setFillColor(Color::White);
    dateInput.text.setFillColor(Color::White);
    nameInput.text.setCharacterSize(22);
    originInput.text.setCharacterSize(22);
    destInput.text.setCharacterSize(22);
    dateInput.text.setCharacterSize(22);

    nameInput.isActive = true;

    RectangleShape cheapestButton(Vector2f(280, 60));
    cheapestButton.setFillColor(Color(150, 30, 30, 255));
    cheapestButton.setOutlineThickness(3);
    cheapestButton.setOutlineColor(Color(255, 80, 80));
    cheapestButton.setPosition(610, 700);

    Text cheapestText("CHEAPEST ROUTE", font, 18);
    cheapestText.setStyle(Text::Bold);
    cheapestText.setFillColor(Color::White);
    cheapestText.setPosition(655, 717);

    RectangleShape fastestButton(Vector2f(280, 60));
    fastestButton.setFillColor(Color(80, 20, 60, 200));
    fastestButton.setOutlineThickness(3);
    fastestButton.setOutlineColor(Color(180, 50, 120));
    fastestButton.setPosition(920, 700);

    Text fastestText("FASTEST ROUTE", font, 18);
    fastestText.setStyle(Text::Bold);
    fastestText.setFillColor(Color::White);
    fastestText.setPosition(965, 717);

    RectangleShape searchButton(Vector2f(400, 70));
    searchButton.setFillColor(Color(180, 30, 30));
    searchButton.setOutlineThickness(4);
    searchButton.setOutlineColor(Color(255, 80, 80));
    searchButton.setPosition(760, 790);

    Text searchBtnText("SEARCH & BOOK ROUTE", font, 24);
    searchBtnText.setStyle(Text::Bold);
    searchBtnText.setFillColor(Color::White);
    searchBtnText.setPosition(820, 808);

    RectangleShape backButton(Vector2f(200, 65));
    backButton.setFillColor(Color(100, 20, 40));
    backButton.setOutlineThickness(4);
    backButton.setOutlineColor(Color(200, 60, 80));
    backButton.setPosition(100, 970);

    Text backBtnText("BACK", font, 24);
    backBtnText.setStyle(Text::Bold);
    backBtnText.setFillColor(Color::White);
    backBtnText.setPosition(155, 985);

    RectangleShape confirmButton(Vector2f(320, 65));
    confirmButton.setFillColor(Color(150, 30, 50));
    confirmButton.setOutlineThickness(4);
    confirmButton.setOutlineColor(Color(255, 80, 100));
    confirmButton.setPosition(1550, 970);

    Text confirmBtnText("CONFIRM BOOKING", font, 22);
    confirmBtnText.setStyle(Text::Bold);
    confirmBtnText.setFillColor(Color::White);
    confirmBtnText.setPosition(1580, 987);

 
    RectangleShape prefsButton(Vector2f(180, 50));
    prefsButton.setFillColor(Color(80, 40, 100, 220));
    prefsButton.setOutlineThickness(3);
    prefsButton.setOutlineColor(Color(150, 80, 200));
    prefsButton.setPosition(1700, 20);

    Text prefsBtnText("PREFERENCES", font, 16);
    prefsBtnText.setStyle(Text::Bold);
    prefsBtnText.setFillColor(Color::White);
    prefsBtnText.setPosition(1730, 33);

   
    RectangleShape prefsIndicator(Vector2f(150, 30));
    prefsIndicator.setFillColor(Color(0, 100, 50, 200));
    prefsIndicator.setOutlineThickness(2);
    prefsIndicator.setOutlineColor(Color(0, 255, 150));
    prefsIndicator.setPosition(1700, 75);

    Text prefsIndicatorText("FILTER ACTIVE", font, 12);
    prefsIndicatorText.setFillColor(Color(100, 255, 150));
    prefsIndicatorText.setStyle(Text::Bold);
    prefsIndicatorText.setPosition(1720, 81);

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

                if (currentState == ENTER_DETAILS)
                {
                    if (cheapestButton.getGlobalBounds().contains(mousePos))
                    {
                        useCheapest = true;
                    }
                    if (fastestButton.getGlobalBounds().contains(mousePos))
                    {
                        useCheapest = false;
                    }
                }

                
                if (currentState == ENTER_DETAILS && prefsButton.getGlobalBounds().contains(mousePos))
                {
                    openShipPreferencesPopup(font);
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
                       
                        DijkstraAlgorithm dateValidator;
                        string dateError = "";
                        
                        if (!dateValidator.isValidDate(date, dateError))
                        {
                            searchResultText = "Invalid Date!";
                            searchResultColor = Color::Red;
                            errorDetails = dateError;
                        }
                        else
                        {
                            currentPath.clear();
                            waitPorts.clear();
                            waitDurations.clear();
                            availableRoutes.clear();

                            string normalizedDate = graph.normalizeDate(date);
                            int originIdx = graph.findPortIndex(origin);
                            int destIdx = graph.findPortIndex(dest);

                            if (originIdx == -1 || destIdx == -1)
                            {
                                searchResultText = originIdx == -1 ? "Origin port not found!" : "Destination port not found!";
                                searchResultColor = Color::Red;
                                errorDetails = "Please check port names.";
                                routeFound = false;
                            }
                            else
                            {
                                DijkstraAlgorithm dijkstra;
                                
                                
                                Vector<int> cheapestPath = dijkstra.findCheapestPath(graph.ports, originIdx, destIdx, normalizedDate, &shipPrefs);
                                
                                Vector<int> fastestPath = dijkstra.findFastestPath(graph.ports, originIdx, destIdx, normalizedDate, &shipPrefs);
                            
                                auto calculateRouteCostAndTime = [&](Vector<int>& path, int& totalCost, int& totalTime) {
                                    totalCost = 0;
                                    totalTime = 0;
                                    string currDate = normalizedDate;
                                    string currTime = "00:00";
                                    const int MIN_DOCKING_DELAY = 120;
                                    
                                    for (int i = 0; i < path.getSize() - 1; i++) {
                                        string fromPort = graph.ports[path[i]].portName;
                                        string toPort = graph.ports[path[i + 1]].portName;
                                        int fromIdx = graph.findPortIndex(fromPort);
                                        if (fromIdx == -1) continue;
                                        
                                        RouteNode* route = graph.ports[fromIdx].routeHead;
                                        RouteNode* bestRoute = nullptr;
                                        int bestWait = INT_MAX;
                                        
                                        while (route != nullptr) {
                                            if (route->destinationPort == toPort && 
                                                dijkstra.isDateGreaterOrEqual(route->departureDate, currDate)) {
                                                int wait = 0;
                                                if (route->departureDate == currDate) {
                                                    int depMin = dijkstra.toMinutes(route->departureTime);
                                                    int curMin = dijkstra.toMinutes(currTime);
                                                    if (depMin >= curMin + MIN_DOCKING_DELAY) {
                                                        wait = depMin - curMin;
                                                        if (bestRoute == nullptr || wait < bestWait) {
                                                            bestRoute = route;
                                                            bestWait = wait;
                                                        }
                                                    }
                                                } else {
                                                    int days = dijkstra.calculateDaysBetween(currDate, route->departureDate);
                                                    wait = (24*60 - dijkstra.toMinutes(currTime)) + (days-1)*24*60 + dijkstra.toMinutes(route->departureTime);
                                                    if (bestRoute == nullptr || wait < bestWait) {
                                                        bestRoute = route;
                                                        bestWait = wait;
                                                    }
                                                }
                                            }
                                            route = route->next;
                                        }
                                        
                                        if (bestRoute != nullptr) {
                                            totalCost += bestRoute->cost;
                                            bool cross = false;
                                            int travel = dijkstra.calculateTravelTime(bestRoute->departureTime, bestRoute->arrivalTime, cross);
                                            totalTime += bestWait + travel;
                                            currDate = cross ? dijkstra.addDaysToDate(bestRoute->departureDate, 1) : bestRoute->departureDate;
                                            currTime = bestRoute->arrivalTime;
                                        }
                                    }
                                    
                                    for (int i = 0; i < path.getSize(); i++) {
                                        totalCost += graph.ports[path[i]].charge;
                                    }
                                };
                                
                                if (cheapestPath.getSize() > 1) {
                                    RouteOption cheapOpt;
                                    cheapOpt.path = cheapestPath;
                                    cheapOpt.routeType = "CHEAPEST";
                                    calculateRouteCostAndTime(cheapestPath, cheapOpt.totalCost, cheapOpt.totalTime);
                                    
                                    
                                    cheapOpt.description = "";
                                    for (int i = 0; i < cheapestPath.getSize(); i++) {
                                        cheapOpt.description += graph.ports[cheapestPath[i]].portName;
                                        if (i < cheapestPath.getSize() - 1) cheapOpt.description += " -> ";
                                    }
                                    availableRoutes.push_back(cheapOpt);
                                }
                                
                               
                                if (fastestPath.getSize() > 1) {
                                    bool isDifferent = false;
                                    if (fastestPath.getSize() != cheapestPath.getSize()) {
                                        isDifferent = true;
                                    } else {
                                        for (int i = 0; i < fastestPath.getSize(); i++) {
                                            if (fastestPath[i] != cheapestPath[i]) {
                                                isDifferent = true;
                                                break;
                                            }
                                        }
                                    }
                                    
                                    if (isDifferent) {
                                        RouteOption fastOpt;
                                        fastOpt.path = fastestPath;
                                        fastOpt.routeType = "FASTEST";
                                        calculateRouteCostAndTime(fastestPath, fastOpt.totalCost, fastOpt.totalTime);
                                        
                                        fastOpt.description = "";
                                        for (int i = 0; i < fastestPath.getSize(); i++) {
                                            fastOpt.description += graph.ports[fastestPath[i]].portName;
                                            if (i < fastestPath.getSize() - 1) fastOpt.description += " -> ";
                                        }
                                        availableRoutes.push_back(fastOpt);
                                    }
                                }
                                
                              
                                RouteNode* directRoute = graph.ports[originIdx].routeHead;
                                while (directRoute != nullptr) {
                                    if (directRoute->destinationPort == dest && 
                                        dijkstra.isDateGreaterOrEqual(directRoute->departureDate, normalizedDate)) {
                                        
                                        Vector<int> directPath;
                                        directPath.push_back(originIdx);
                                        directPath.push_back(destIdx);
                                        
                                       
                                        bool alreadyExists = false;
                                        for (int r = 0; r < availableRoutes.getSize(); r++) {
                                            if (availableRoutes[r].path.getSize() == 2 &&
                                                availableRoutes[r].path[0] == originIdx &&
                                                availableRoutes[r].path[1] == destIdx) {
                                                alreadyExists = true;
                                                break;
                                            }
                                        }
                                        
                                        if (!alreadyExists) {
                                            RouteOption directOpt;
                                            directOpt.path = directPath;
                                            directOpt.routeType = "DIRECT";
                                            calculateRouteCostAndTime(directPath, directOpt.totalCost, directOpt.totalTime);
                                            directOpt.description = graph.ports[originIdx].portName + " -> " + dest;
                                            availableRoutes.push_back(directOpt);
                                        }
                                        break;
                                    }
                                    directRoute = directRoute->next;
                                }
                                
                                if (availableRoutes.getSize() > 0) {
                                    routeFound = true;
                                    searchResultText = to_string(availableRoutes.getSize()) + " route(s) found!";
                                    searchResultColor = Color::Green;
                                    errorDetails = "Select a route to proceed with booking.";
                                    
                                   
                                    currentBooking.customerName = name;
                                    currentBooking.origin = origin;
                                    currentBooking.destination = dest;
                                    currentBooking.departureDate = normalizedDate;
                                    
                                  
                                    currentState = SELECT_ROUTE;
                                    stateTransition = 0.0f;
                                    selectedRoute = 0;
                                }
                                else
                                {
                                    searchResultText = "No route found!";
                                    searchResultColor = Color::Red;
                                    errorDetails = "No valid path exists for the given date and ports.";
                                    routeFound = false;
                                }
                            }
                        }
                    }
                }

                
                if (currentState == SELECT_ROUTE)
                {
                   
                    for (int r = 0; r < availableRoutes.getSize(); r++)
                    {
                        float cardY = 200 + r * 160;
                        FloatRect cardBounds(200, cardY, 1520, 140);
                        if (cardBounds.contains(mousePos))
                        {
                            selectedRoute = r;
                            
                            
                            currentPath = availableRoutes[r].path;
                            currentBooking.routePath.clear();
                            for (int i = 0; i < currentPath.getSize(); i++)
                            {
                                currentBooking.routePath.push_back(graph.ports[currentPath[i]].portName);
                            }
                            
                            DijkstraAlgorithm dijkstra;
                            int actualRouteCost = 0;
                            waitPorts.clear();
                            waitDurations.clear();
                            
                            string currentDate = currentBooking.departureDate;
                            string currentTime = "00:00";
                            const int MIN_DOCKING_DELAY = 120;

                            for (int i = 0; i < currentPath.getSize() - 1; i++)
                            {
                                string fromPort = graph.ports[currentPath[i]].portName;
                                string toPort = graph.ports[currentPath[i + 1]].portName;
                                
                                int fromGraphIdx = graph.findPortIndex(fromPort);
                                if (fromGraphIdx == -1) continue;

                                RouteNode* bestRoute = nullptr;
                                int bestWaitMinutes = INT_MAX;
                                
                                RouteNode* route = graph.ports[fromGraphIdx].routeHead;
                                while (route != nullptr)
                                {
                                    if (route->destinationPort == toPort &&
                                        dijkstra.isDateGreaterOrEqual(route->departureDate, currentDate))
                                    {
                                        int waitMinutes = 0;
                                        
                                        if (route->departureDate == currentDate)
                                        {
                                            int departureMin = dijkstra.toMinutes(route->departureTime);
                                            int currentMin = dijkstra.toMinutes(currentTime);
                                            
                                            if (departureMin >= currentMin + MIN_DOCKING_DELAY)
                                            {
                                                waitMinutes = departureMin - currentMin;
                                                if (bestRoute == nullptr || waitMinutes < bestWaitMinutes)
                                                {
                                                    bestRoute = route;
                                                    bestWaitMinutes = waitMinutes;
                                                }
                                            }
                                        }
                                        else
                                        {
                                            int daysBetween = dijkstra.calculateDaysBetween(currentDate, route->departureDate);
                                            waitMinutes = (24 * 60 - dijkstra.toMinutes(currentTime)) + (daysBetween - 1) * 24 * 60 + dijkstra.toMinutes(route->departureTime);
                                            if (bestRoute == nullptr || waitMinutes < bestWaitMinutes)
                                            {
                                                bestRoute = route;
                                                bestWaitMinutes = waitMinutes;
                                            }
                                        }
                                    }
                                    route = route->next;
                                }
                                
                                if (bestRoute != nullptr)
                                {
                                    actualRouteCost += bestRoute->cost;
                                    
                                    if (bestWaitMinutes > 0)
                                    {
                                        waitPorts.push_back(fromPort);
                                        waitDurations.push_back(graph.formatWaitTime(bestWaitMinutes));
                                    }
                                    
                                    bool crossesMidnight = false;
                                    dijkstra.calculateTravelTime(bestRoute->departureTime, bestRoute->arrivalTime, crossesMidnight);
                                    currentDate = crossesMidnight ? dijkstra.addDaysToDate(bestRoute->departureDate, 1) : bestRoute->departureDate;
                                    currentTime = bestRoute->arrivalTime;
                                }
                            }
                            
                            currentBooking.waitPorts = waitPorts;
                            currentBooking.waitDurations = waitDurations;

                            int portCharges = 0;
                            for (int i = 0; i < currentPath.getSize(); i++)
                            {
                                portCharges += graph.ports[currentPath[i]].charge;
                            }

                            int dockingCharges = 0;
                            int totalDockWaitMins = 0;
                            for (int i = 0; i < currentPath.getSize(); i++)
                            {
                                string portName = graph.ports[currentPath[i]].portName;
                                int portLocationIdx = -1;
                                for (int j = 0; j < numLocations; j++) {
                                    if (portLocations[j].name == portName) {
                                        portLocationIdx = j;
                                        break;
                                    }
                                }
                                
                                if (portLocationIdx != -1) {
                                    int dockWait = getPortDockWaitMinutes(portLocationIdx);
                                    totalDockWaitMins += dockWait;
                                    if (dockWait > 0) {
                                        dockingCharges += static_cast<int>(graph.ports[currentPath[i]].charge * (dockWait / 1440.0f));
                                    }
                                }
                            }

                            currentBooking.baseRouteCost = actualRouteCost;
                            currentBooking.dockingCharges = portCharges;
                            currentBooking.queueWaitCharges = dockingCharges;
                            currentBooking.dockingWaitTime = totalDockWaitMins;
                            currentBooking.totalCost = actualRouteCost + portCharges + dockingCharges;

                            currentState = DISPLAY_ROUTE;
                            stateTransition = 0.0f;
                            animationStarted = true;
                            currentSegment = 0;
                            segmentProgress = 0.0f;
                            isPaused = false;
                            break;
                        }
                    }
                }

                if (currentState == DISPLAY_ROUTE && confirmButton.getGlobalBounds().contains(mousePos))
                {
                    currentBooking.bookingID = bookingManager.GenerateBookingID();

                    Booking *newBooking = new Booking();
                    *newBooking = currentBooking;
                    bookingManager.AddBooking(newBooking);
                    
                    // Dock the ship at the destination port
                    int destPortIndex = -1;
                    for (int i = 0; i < numLocations; i++) {
                        if (portLocations[i].name == currentBooking.destination) {
                            destPortIndex = i;
                            break;
                        }
                    }
                    
                    if (destPortIndex >= 0) {
                        GlobalDockShipInfo newShip;
                        newShip.shipId = currentBooking.bookingID;
                        newShip.company = currentBooking.customerName;
                        newShip.dockTimer = 7200.0f; // 2 hours default docking time in seconds
                        newShip.maxTime = 7200.0f;
                        newShip.dockSlot = globalPortDocking[destPortIndex].findFreeSlot();
                        
                        if (newShip.dockSlot >= 0) {
                            // Free dock slot available, dock directly
                            globalPortDocking[destPortIndex].dockedShips.push_back(newShip);
                        } else {
                            // No free slot, add to queue
                            GlobalQueueShipInfo queueShip;
                            queueShip.shipId = currentBooking.bookingID;
                            queueShip.company = currentBooking.customerName;
                            queueShip.queuePosition = globalPortDocking[destPortIndex].queuedShips.getSize() + 1;
                            globalPortDocking[destPortIndex].queuedShips.push_back(queueShip);
                        }
                    }
                
                    hornSound.play();

                    currentState = SUCCESS;
                    stateTransition = 0.0f;
                }
            }
        }

        window.clear();
        window.draw(bgSprite);

        RectangleShape overlay(Vector2f(1920, 1080));
        overlay.setFillColor(Color(40, 0, 10, 180)); 
        window.draw(overlay);

        if (currentState == ENTER_DETAILS)
        {
            
            for (int p = 0; p < 60; p++)
            {
                float px = static_cast<float>((p * 67) % 1920);
                float py = static_cast<float>(fmod(1080.0 + fmod(time * 20.0 + p * 40.0, 1100.0), 1100.0)); 
                float wobble = static_cast<float>(sin(time * 2.0 + p)) * 15.0f;
                float flicker = 0.5f + 0.5f * static_cast<float>(sin(time * 8.0 + p * 0.5f));
                CircleShape particle(2.0f + flicker * 2.0f);
                particle.setFillColor(Color(255, static_cast<Uint8>(80 + 80 * flicker), 50, static_cast<Uint8>((60 + 60 * flicker) * stateTransition)));
                particle.setPosition(px + wobble, py);
                window.draw(particle);
            }

            float scale = 0.9f + (stateTransition * 0.1f);
            float alpha = stateTransition * 255;
            RectangleShape mainBox(Vector2f(850 * scale, 680 * scale));
            mainBox.setFillColor(Color(20, 5, 15, static_cast<Uint8>(230 * stateTransition)));
            mainBox.setOutlineThickness(4);
            mainBox.setOutlineColor(Color(200, 50, 50, static_cast<Uint8>(alpha)));
            mainBox.setPosition(535 + (850 - 850 * scale) / 2, 150 + (680 - 680 * scale) / 2);
            window.draw(mainBox);

            float accentWidth = 850.0f * scale;
            RectangleShape topAccent(Vector2f(accentWidth, 8));
            
            float flickerIntensity = 0.6f + 0.4f * static_cast<float>(sin(time * 4.0));
            topAccent.setFillColor(Color(static_cast<Uint8>(200 * flickerIntensity), 30, 30, static_cast<Uint8>(alpha)));
            topAccent.setPosition(535 + (850 - 850 * scale) / 2, 150 + (680 - 680 * scale) / 2);
            window.draw(topAccent);

            for (int corner = 0; corner < 4; corner++)
            {
                float cx = (corner % 2 == 0) ? 540 : 1380 - 25;
                float cy = (corner < 2) ? 155 : 825 - 25;
                RectangleShape cornerH(Vector2f(50, 4));
                RectangleShape cornerV(Vector2f(4, 50));
               
                float flicker = 0.4f + 0.6f * static_cast<float>(sin(time * 5.0 + corner * 1.5f));
                Uint8 glowAlpha = static_cast<Uint8>((180 + 75 * flicker) * stateTransition);
                cornerH.setFillColor(Color(255, static_cast<Uint8>(50 * flicker), 50, glowAlpha));
                cornerV.setFillColor(Color(255, static_cast<Uint8>(50 * flicker), 50, glowAlpha));
                cornerH.setPosition(cx, cy);
                cornerV.setPosition(cx, cy);
                window.draw(cornerH);
                window.draw(cornerV);
            }

            Text title;
            title.setFont(font);
            title.setString("Book Cargo Route");
            title.setCharacterSize(42);
            title.setStyle(Text::Bold);
           
            float titleGlow = 0.5f + 0.5f * static_cast<float>(sin(time * 3.0));
            title.setFillColor(Color(255, static_cast<Uint8>(50 + 50 * titleGlow), static_cast<Uint8>(50 + 30 * titleGlow), static_cast<Uint8>(alpha)));
            title.setOutlineThickness(3);
            title.setOutlineColor(Color(100, 0, 0, static_cast<Uint8>(200 * stateTransition)));
            FloatRect titleBounds = title.getLocalBounds();
            title.setPosition(960 - titleBounds.width / 2, 160);
            window.draw(title);

            Text subtitle;
            subtitle.setFont(font);
            subtitle.setString("Enter Shipment Details Below");
            subtitle.setCharacterSize(18);
            subtitle.setFillColor(Color(255, 150, 150, static_cast<Uint8>(alpha)));
            FloatRect subBounds = subtitle.getLocalBounds();
            subtitle.setPosition(960 - subBounds.width / 2, 210);
            window.draw(subtitle);

            RectangleShape divider(Vector2f(400, 3));
            divider.setFillColor(Color(200, 50, 50, static_cast<Uint8>(150 * stateTransition)));
            divider.setPosition(760, 235);
            window.draw(divider);

            nameInput.draw(window);
            originInput.draw(window);
            destInput.draw(window);
            dateInput.draw(window);

            Text modeLabel;
            modeLabel.setFont(font);
            modeLabel.setString("Choose Your Path:");
            modeLabel.setCharacterSize(18);
            modeLabel.setStyle(Text::Bold);
            modeLabel.setFillColor(Color(255, 150, 150, static_cast<Uint8>(alpha)));
            modeLabel.setPosition(560, 670);
            window.draw(modeLabel);

            if (useCheapest)
            {
                cheapestButton.setFillColor(Color(180, 40, 40, 255));
                cheapestButton.setOutlineColor(Color(255, 100, 100));
                cheapestButton.setOutlineThickness(4);
                cheapestText.setCharacterSize(19);
                fastestButton.setFillColor(Color(60, 20, 40, 180));
                fastestButton.setOutlineColor(Color(120, 50, 80));
                fastestButton.setOutlineThickness(2);
                fastestText.setCharacterSize(17);
            }
            else
            {
                cheapestButton.setFillColor(Color(60, 20, 30, 180));
                cheapestButton.setOutlineColor(Color(120, 50, 60));
                cheapestButton.setOutlineThickness(2);
                cheapestText.setCharacterSize(17);
                fastestButton.setFillColor(Color(150, 40, 80, 255));
                fastestButton.setOutlineColor(Color(255, 80, 150));
                fastestButton.setOutlineThickness(4);
                fastestText.setCharacterSize(19);
            }

            if (cheapestButton.getGlobalBounds().contains(mousePos))
            {
                cheapestButton.setOutlineColor(Color(255, 150, 150));
                RectangleShape hoverGlow(Vector2f(290, 70));
                hoverGlow.setFillColor(Color(255, 80, 80, 50));
                hoverGlow.setPosition(605, 695);
                window.draw(hoverGlow);
            }
            if (fastestButton.getGlobalBounds().contains(mousePos))
            {
                fastestButton.setOutlineColor(Color(255, 120, 180));
                RectangleShape hoverGlow(Vector2f(290, 70));
                hoverGlow.setFillColor(Color(255, 80, 150, 50));
                hoverGlow.setPosition(915, 695);
                window.draw(hoverGlow);
            }

            window.draw(cheapestButton);
            window.draw(cheapestText);
            window.draw(fastestButton);
            window.draw(fastestText);

            bool searchHover = searchButton.getGlobalBounds().contains(mousePos);
            if (searchHover)
            {
                float pulse = 1.0f + 0.06f * static_cast<float>(sin(time * 6.0));
                searchButton.setScale(pulse, pulse);
                searchButton.setFillColor(Color(220, 50, 50));
                searchButton.setOutlineColor(Color(255, 120, 120));
                RectangleShape btnGlow(Vector2f(420, 90));
                btnGlow.setFillColor(Color(255, 50, 50, 60));
                btnGlow.setPosition(750, 780);
                window.draw(btnGlow);
            }
            else
            {
                searchButton.setScale(1.0f, 1.0f);
                searchButton.setFillColor(Color(180, 30, 30));
                searchButton.setOutlineColor(Color(255, 80, 80));
            }
            window.draw(searchButton);
            searchBtnText.setFillColor(Color::White);
            searchBtnText.setPosition(searchHover ? 798 : 800, searchHover ? 806 : 808);
            window.draw(searchBtnText);

            if (!searchResultText.empty())
            {
                RectangleShape resultBox(Vector2f(400, 70));
                resultBox.setFillColor(searchResultColor == Color::Green ? Color(30, 50, 30, 200) : Color(60, 10, 20, 200));
                resultBox.setOutlineThickness(2);
                resultBox.setOutlineColor(searchResultColor == Color::Green ? Color(100, 255, 100) : Color(255, 80, 80));
                resultBox.setPosition(760, 760);
                window.draw(resultBox);

                Text resultText;
                resultText.setFont(font);
                resultText.setString(searchResultText);
                resultText.setCharacterSize(18);
                resultText.setFillColor(searchResultColor == Color::Green ? Color(100, 255, 100) : Color(255, 100, 100));
                resultText.setStyle(Text::Bold);
                resultText.setPosition(780, 770);
                window.draw(resultText);

                if (!errorDetails.empty())
                {
                    Text detailText;
                    detailText.setFont(font);
                    detailText.setString(errorDetails);
                    detailText.setCharacterSize(14);
                    detailText.setFillColor(Color(255, 180, 100));
                    detailText.setPosition(780, 800);
                    window.draw(detailText);
                }
            }

            
            bool prefsHover = prefsButton.getGlobalBounds().contains(mousePos);
            if (prefsHover)
            {
                prefsButton.setFillColor(Color(100, 60, 140, 255));
                prefsButton.setOutlineColor(Color(200, 120, 255));
                RectangleShape prefsGlow(Vector2f(190, 60));
                prefsGlow.setFillColor(Color(150, 80, 200, 50));
                prefsGlow.setPosition(1695, 15);
                window.draw(prefsGlow);
            }
            else
            {
                prefsButton.setFillColor(Color(80, 40, 100, 220));
                prefsButton.setOutlineColor(Color(150, 80, 200));
            }
            window.draw(prefsButton);
            window.draw(prefsBtnText);

            
            if (shipPrefs.filterActive)
            {
                float indicatorPulse = 0.7f + 0.3f * static_cast<float>(sin(time * 4.0));
                prefsIndicator.setFillColor(Color(0, static_cast<Uint8>(80 * indicatorPulse), static_cast<Uint8>(40 * indicatorPulse), 220));
                prefsIndicator.setOutlineColor(Color(0, static_cast<Uint8>(200 + 55 * indicatorPulse), static_cast<Uint8>(100 + 50 * indicatorPulse)));
                window.draw(prefsIndicator);
                window.draw(prefsIndicatorText);

             
                string prefSummary = "";
                if (shipPrefs.preferredCompanies.getSize() > 0)
                {
                    prefSummary = "Companies: " + to_string(shipPrefs.preferredCompanies.getSize());
                }
                if (shipPrefs.avoidedPorts.getSize() > 0)
                {
                    if (!prefSummary.empty()) prefSummary += " | ";
                    prefSummary += "Avoiding: " + to_string(shipPrefs.avoidedPorts.getSize()) + " ports";
                }
                if (shipPrefs.maxVoyageMinutes > 0)
                {
                    if (!prefSummary.empty()) prefSummary += " | ";
                    prefSummary += "Max: " + to_string(shipPrefs.maxVoyageMinutes / 60) + "h";
                }

                if (!prefSummary.empty())
                {
                    Text prefSummaryText;
                    prefSummaryText.setFont(font);
                    prefSummaryText.setString(prefSummary);
                    prefSummaryText.setCharacterSize(11);
                    prefSummaryText.setFillColor(Color(180, 255, 200));
                    prefSummaryText.setPosition(1700, 110);
                    window.draw(prefSummaryText);
                }
            }
        }
        else if (currentState == SELECT_ROUTE)
        {
          
            for (int p = 0; p < 60; p++)
            {
                float px = static_cast<float>((p * 67) % 1920);
                float py = static_cast<float>(fmod(1080.0 + fmod(time * 20.0 + p * 40.0, 1100.0), 1100.0));
                float wobble = static_cast<float>(sin(time * 2.0 + p)) * 15.0f;
                float flicker = 0.5f + 0.5f * static_cast<float>(sin(time * 8.0 + p * 0.5f));
                CircleShape particle(2.0f + flicker * 2.0f);
                particle.setFillColor(Color(255, static_cast<Uint8>(80 + 80 * flicker), 50, static_cast<Uint8>(60 + 60 * flicker)));
                particle.setPosition(px + wobble, py);
                window.draw(particle);
            }

          
            Text title;
            title.setFont(font);
            title.setString("Select Your Route");
            title.setCharacterSize(48);
            title.setStyle(Text::Bold);
            float titleGlow = 0.5f + 0.5f * static_cast<float>(sin(time * 3.0));
            title.setFillColor(Color(255, static_cast<Uint8>(80 + 40 * titleGlow), static_cast<Uint8>(80 + 30 * titleGlow)));
            title.setOutlineThickness(3);
            title.setOutlineColor(Color(100, 0, 0, 200));
            FloatRect titleBounds = title.getLocalBounds();
            title.setPosition(960 - titleBounds.width / 2, 80);
            window.draw(title);

            Text subtitle;
            subtitle.setFont(font);
            subtitle.setString(currentBooking.origin + " to " + currentBooking.destination + " | " + to_string(availableRoutes.getSize()) + " route(s) available");
            subtitle.setCharacterSize(22);
            subtitle.setFillColor(Color(255, 180, 180));
            FloatRect subBounds = subtitle.getLocalBounds();
            subtitle.setPosition(960 - subBounds.width / 2, 140);
            window.draw(subtitle);

           
            hoveredRoute = -1;
            for (int r = 0; r < availableRoutes.getSize(); r++)
            {
                float cardY = 200 + r * 160;
                FloatRect cardBounds(200, cardY, 1520, 140);
                bool isHovered = cardBounds.contains(mousePos);
                if (isHovered) hoveredRoute = r;

               
                RectangleShape card(Vector2f(1520, 140));
                card.setPosition(200, cardY);
                
                if (isHovered)
                {
                    card.setFillColor(Color(60, 20, 30, 240));
                    card.setOutlineThickness(4);
                    card.setOutlineColor(Color(255, 100, 100));
                }
                else
                {
                    card.setFillColor(Color(30, 10, 20, 220));
                    card.setOutlineThickness(2);
                    card.setOutlineColor(Color(150, 50, 50));
                }
                window.draw(card);

                
                RectangleShape badge(Vector2f(120, 35));
                badge.setPosition(220, cardY + 15);
                Color badgeColor;
                if (availableRoutes[r].routeType == "CHEAPEST")
                    badgeColor = Color(50, 180, 50);
                else if (availableRoutes[r].routeType == "FASTEST")
                    badgeColor = Color(50, 120, 200);
                else
                    badgeColor = Color(180, 120, 50);
                badge.setFillColor(badgeColor);
                badge.setOutlineThickness(2);
                badge.setOutlineColor(Color(255, 255, 255, 150));
                window.draw(badge);

                Text badgeText;
                badgeText.setFont(font);
                badgeText.setString(availableRoutes[r].routeType);
                badgeText.setCharacterSize(14);
                badgeText.setStyle(Text::Bold);
                badgeText.setFillColor(Color::White);
                FloatRect badgeBounds = badgeText.getLocalBounds();
                badgeText.setPosition(280 - badgeBounds.width / 2, cardY + 22);
                window.draw(badgeText);

                
                Text pathText;
                pathText.setFont(font);
                pathText.setString(availableRoutes[r].description);
                pathText.setCharacterSize(18);
                pathText.setFillColor(Color(255, 200, 200));
                pathText.setPosition(360, cardY + 20);
                window.draw(pathText);

                
                Text stopsText;
                stopsText.setFont(font);
                int stops = availableRoutes[r].path.getSize() - 2;
                stopsText.setString(stops == 0 ? "Direct Route" : to_string(stops) + " stop(s)");
                stopsText.setCharacterSize(16);
                stopsText.setFillColor(Color(200, 150, 150));
                stopsText.setPosition(360, cardY + 50);
                window.draw(stopsText);

                
                Text costLabel;
                costLabel.setFont(font);
                costLabel.setString("Total Cost:");
                costLabel.setCharacterSize(14);
                costLabel.setFillColor(Color(180, 150, 150));
                costLabel.setPosition(1200, cardY + 20);
                window.draw(costLabel);

                Text costText;
                costText.setFont(font);
                costText.setString("$" + to_string(availableRoutes[r].totalCost));
                costText.setCharacterSize(28);
                costText.setStyle(Text::Bold);
                costText.setFillColor(Color(100, 255, 100));
                costText.setPosition(1200, cardY + 40);
                window.draw(costText);

                
                Text timeLabel;
                timeLabel.setFont(font);
                timeLabel.setString("Duration:");
                timeLabel.setCharacterSize(14);
                timeLabel.setFillColor(Color(180, 150, 150));
                timeLabel.setPosition(1400, cardY + 20);
                window.draw(timeLabel);

                
                int totalMins = availableRoutes[r].totalTime;
                int days = totalMins / (24 * 60);
                int hours = (totalMins % (24 * 60)) / 60;
                int mins = totalMins % 60;
                string timeStr;
                if (days > 0)
                    timeStr = to_string(days) + "d " + to_string(hours) + "h";
                else
                    timeStr = to_string(hours) + "h " + to_string(mins) + "m";

                Text timeText;
                timeText.setFont(font);
                timeText.setString(timeStr);
                timeText.setCharacterSize(24);
                timeText.setStyle(Text::Bold);
                timeText.setFillColor(Color(100, 200, 255));
                timeText.setPosition(1400, cardY + 40);
                window.draw(timeText);

                
                if (isHovered)
                {
                    Text clickText;
                    clickText.setFont(font);
                    clickText.setString("Click to Select");
                    clickText.setCharacterSize(16);
                    clickText.setFillColor(Color(255, 200, 100));
                    clickText.setPosition(360, cardY + 100);
                    window.draw(clickText);
                }

                
                if (shipPrefs.filterActive)
                {
                    Vector<int>& routePath = availableRoutes[r].path;
                    string prefInfo = "";
                    bool hasPreferredPorts = false;
                    
                    for (int pi = 0; pi < routePath.getSize(); pi++)
                    {
                        string portName = graph.ports[routePath[pi]].portName;
                        if (shipPrefs.isPortAvoided(portName))
                        {
                            
                            prefInfo = "Warning: Route includes avoided port";
                            break;
                        }
                    }
                    
                    if (!prefInfo.empty())
                    {
                        Text prefWarning;
                        prefWarning.setFont(font);
                        prefWarning.setString(prefInfo);
                        prefWarning.setCharacterSize(12);
                        prefWarning.setFillColor(Color(255, 150, 50));
                        prefWarning.setPosition(360, cardY + 75);
                        window.draw(prefWarning);
                    }
                }
            }

            
            if (shipPrefs.filterActive)
            {
                RectangleShape filterBox(Vector2f(200, 80));
                filterBox.setFillColor(Color(20, 60, 40, 230));
                filterBox.setOutlineThickness(2);
                filterBox.setOutlineColor(Color(0, 200, 100));
                filterBox.setPosition(1680, 20);
                window.draw(filterBox);

                Text filterTitle;
                filterTitle.setFont(font);
                filterTitle.setString("PREFERENCES ACTIVE");
                filterTitle.setCharacterSize(12);
                filterTitle.setStyle(Text::Bold);
                filterTitle.setFillColor(Color(100, 255, 150));
                filterTitle.setPosition(1695, 28);
                window.draw(filterTitle);

                
                string summary = "";
                if (shipPrefs.preferredCompanies.getSize() > 0)
                    summary += to_string(shipPrefs.preferredCompanies.getSize()) + " companies\n";
                if (shipPrefs.avoidedPorts.getSize() > 0)
                    summary += to_string(shipPrefs.avoidedPorts.getSize()) + " avoided ports\n";
                if (shipPrefs.maxVoyageMinutes > 0)
                    summary += "Max " + to_string(shipPrefs.maxVoyageMinutes / 60) + "h voyage";
                
                Text filterSummary;
                filterSummary.setFont(font);
                filterSummary.setString(summary);
                filterSummary.setCharacterSize(11);
                filterSummary.setFillColor(Color(180, 255, 200));
                filterSummary.setPosition(1695, 48);
                window.draw(filterSummary);
            }

            
            Text instructionText;
            instructionText.setFont(font);
            instructionText.setString("Click on a route to proceed with booking");
            instructionText.setCharacterSize(18);
            instructionText.setFillColor(Color(200, 150, 150));
            FloatRect instrBounds = instructionText.getLocalBounds();
            instructionText.setPosition(960 - instrBounds.width / 2, 950);
            window.draw(instructionText);
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
                mapSprite.setColor(Color(255, 255, 255, 255)); 
                window.draw(mapSprite);
            }

            if (hasBothRoutes)
            {
                RectangleShape selectionBar(Vector2f(600, 80));
                selectionBar.setFillColor(Color(30, 10, 20, 230));
                selectionBar.setOutlineThickness(3);
                selectionBar.setOutlineColor(Color(200, 60, 60));
                selectionBar.setPosition(660, 20);
                window.draw(selectionBar);

                Text selectionTitle("Choose Route:", font, 20);
                selectionTitle.setFillColor(Color(255, 150, 150));
                selectionTitle.setStyle(Text::Bold);
                selectionTitle.setPosition(680, 28);
                window.draw(selectionTitle);

                Vector2f mousePos = window.mapPixelToCoords(Mouse::getPosition(window));

                RectangleShape directBtn(Vector2f(120, 40));
                directBtn.setPosition(680, 55);
                bool directHover = directBtn.getGlobalBounds().contains(mousePos);
                directBtn.setFillColor(selectedRoute == 0 ? Color(180, 50, 50) : (directHover ? Color(100, 30, 40) : Color(50, 20, 30)));
                directBtn.setOutlineThickness(2);
                directBtn.setOutlineColor(selectedRoute == 0 ? Color(255, 100, 100) : Color(150, 60, 70));
                window.draw(directBtn);

                Text directText("DIRECT", font, 16);
                directText.setFillColor(Color::White);
                directText.setStyle(Text::Bold);
                directText.setPosition(700, 62);
                window.draw(directText);

                RectangleShape connectingBtn(Vector2f(140, 40));
                connectingBtn.setPosition(820, 55);
                bool connectingHover = connectingBtn.getGlobalBounds().contains(mousePos);
                connectingBtn.setFillColor(selectedRoute == 1 ? Color(180, 50, 50) : (connectingHover ? Color(100, 30, 40) : Color(50, 20, 30)));
                connectingBtn.setOutlineThickness(2);
                connectingBtn.setOutlineColor(selectedRoute == 1 ? Color(255, 100, 100) : Color(150, 60, 70));
                window.draw(connectingBtn);

                Text connectingText("CONNECTING", font, 16);
                connectingText.setFillColor(Color::White);
                connectingText.setStyle(Text::Bold);
                connectingText.setPosition(830, 62);
                window.draw(connectingText);

                static bool btnClicked = false;
                if (Mouse::isButtonPressed(Mouse::Button::Left) && !btnClicked)
                {
                    if (directHover) selectedRoute = 0;
                    if (connectingHover) selectedRoute = 1;
                    btnClicked = true;
                }
                if (!Mouse::isButtonPressed(Mouse::Button::Left))
                {
                    btnClicked = false;
                }
            }

            Vector<int>& displayPath = (hasBothRoutes && selectedRoute == 0) ? directPath : currentPath;

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

            for (int i = 0; i < displayPath.getSize() - 1; i++)
            {
                int startIdx = displayPath[i];
                int endIdx = displayPath[i + 1];
                Vector2f start(portLocations[startIdx].x, portLocations[startIdx].y);
                Vector2f end(portLocations[endIdx].x, portLocations[endIdx].y);

                float length = sqrt(pow(end.x - start.x, 2) + pow(end.y - start.y, 2));
                float angle = atan2(end.y - start.y, end.x - start.x) * 180 / 3.14159f;

                
                bool isPreferenceRoute = shipPrefs.filterActive;

                if (i == currentSegment)
                {
                    float currentLength = length * segmentProgress;

                    RectangleShape currentLine(Vector2f(currentLength, 6.0f));
                    currentLine.setPosition(start);
                    currentLine.setRotation(angle);
                    
                    if (isPreferenceRoute)
                        currentLine.setFillColor(Color(80, 255, 180)); 
                    else
                        currentLine.setFillColor(Color(255, 80, 80)); 
                    window.draw(currentLine);

                    RectangleShape currentGlow(Vector2f(currentLength, 12.0f));
                    currentGlow.setPosition(start);
                    currentGlow.setRotation(angle);
                    if (isPreferenceRoute)
                        currentGlow.setFillColor(Color(50, 255, 150, 100)); 
                    else
                        currentGlow.setFillColor(Color(255, 50, 50, 100));
                    window.draw(currentGlow);

                    Vector2f currentPos = start + (end - start) * segmentProgress;
                    float pulse = 0.7f + 0.3f * sin(time * 8.0f);

                    CircleShape endGlow(15.f * pulse);
                    if (isPreferenceRoute)
                        endGlow.setFillColor(Color(80, 255, 150, 150)); 
                    else
                        endGlow.setFillColor(Color(255, 80, 80, 150));
                    endGlow.setOrigin(15.f * pulse, 15.f * pulse);
                    endGlow.setPosition(currentPos);
                    window.draw(endGlow);

                    CircleShape endDot(8.f);
                    if (isPreferenceRoute)
                        endDot.setFillColor(Color(100, 255, 200)); 
                    else
                        endDot.setFillColor(Color(255, 200, 50)); 
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
                    if (isPreferenceRoute)
                        completedLine.setFillColor(Color(60, 200, 140)); 
                    else
                        completedLine.setFillColor(Color(200, 60, 60)); 
                    window.draw(completedLine);
                }
                else
                {
                    RectangleShape upcomingLine(Vector2f(length, 3.0f));
                    upcomingLine.setPosition(start);
                    upcomingLine.setRotation(angle);
                    if (isPreferenceRoute)
                        upcomingLine.setFillColor(Color(30, 100, 70, 100)); 
                    else
                        upcomingLine.setFillColor(Color(100, 30, 30, 100)); 
                    window.draw(upcomingLine);
                }
            }

            for (int i = 0; i < displayPath.getSize(); i++)
            {
                int portIdx = displayPath[i];
                Vector2f portPos(portLocations[portIdx].x, portLocations[portIdx].y);
                string portName = portLocations[portIdx].name;

                
                bool isPreferenceRoute = shipPrefs.filterActive;

                
                CircleShape glow(16.f);
                glow.setOrigin(16, 16);
                if (isPreferenceRoute)
                {
                    
                    float pulse = 0.6f + 0.4f * static_cast<float>(sin(time * 3.0 + i * 0.5f));
                    glow.setFillColor(Color(static_cast<Uint8>(100 * pulse), 255, static_cast<Uint8>(150 * pulse), 120));
                }
                else
                {
                    glow.setFillColor(Color(255, 80, 80, 100)); 
                }
                glow.setPosition(portPos);
                window.draw(glow);

                
                CircleShape dot(8.f);
                dot.setOrigin(8.f, 8.f);
                if (isPreferenceRoute)
                {
                    
                    dot.setFillColor(Color(100, 255, 200));
                    dot.setOutlineColor(Color(0, 255, 150));
                    dot.setOutlineThickness(2.0f);
                }
                else
                {
                    dot.setFillColor(Color(255, 100, 100)); 
                    dot.setOutlineColor(Color::White);
                    dot.setOutlineThickness(1.5f);
                }
                dot.setPosition(portPos);
                window.draw(dot);

                Text label;
                label.setFont(font);
                label.setString(portName);
                label.setCharacterSize(16);
                label.setStyle(Text::Bold);
                label.setOutlineThickness(1.f);
                label.setOutlineColor(Color(0, 0, 0, 255));
                if (isPreferenceRoute)
                {
                    label.setFillColor(Color(150, 255, 200)); 
                }
                else
                {
                    label.setFillColor(Color(255, 150, 150)); 
                }
                label.setPosition(portPos.x, portPos.y - 25.f);
                window.draw(label);

                
                if (isPreferenceRoute && i == 0)
                {
                    RectangleShape prefMarker(Vector2f(140, 22));
                    prefMarker.setFillColor(Color(0, 100, 60, 220));
                    prefMarker.setOutlineThickness(1);
                    prefMarker.setOutlineColor(Color(0, 255, 150));
                    prefMarker.setPosition(portPos.x - 10, portPos.y + 10);
                    window.draw(prefMarker);

                    Text prefText;
                    prefText.setFont(font);
                    prefText.setString("FILTERED ROUTE");
                    prefText.setCharacterSize(11);
                    prefText.setStyle(Text::Bold);
                    prefText.setFillColor(Color(100, 255, 180));
                    prefText.setPosition(portPos.x - 5, portPos.y + 13);
                    window.draw(prefText);
                }
            }

            
            if (shipPrefs.filterActive)
            {
                RectangleShape prefDisplayBox(Vector2f(220, 90));
                prefDisplayBox.setFillColor(Color(0, 50, 35, 230));
                prefDisplayBox.setOutlineThickness(2);
                prefDisplayBox.setOutlineColor(Color(0, 200, 120));
                prefDisplayBox.setPosition(1680, 20);
                window.draw(prefDisplayBox);

                Text prefDisplayTitle;
                prefDisplayTitle.setFont(font);
                prefDisplayTitle.setString("PREFERENCES APPLIED");
                prefDisplayTitle.setCharacterSize(12);
                prefDisplayTitle.setStyle(Text::Bold);
                prefDisplayTitle.setFillColor(Color(100, 255, 150));
                prefDisplayTitle.setPosition(1695, 28);
                window.draw(prefDisplayTitle);

                string prefDetails = "";
                if (shipPrefs.preferredCompanies.getSize() > 0)
                    prefDetails += "Companies: " + to_string(shipPrefs.preferredCompanies.getSize()) + "\n";
                if (shipPrefs.avoidedPorts.getSize() > 0)
                    prefDetails += "Avoided: " + to_string(shipPrefs.avoidedPorts.getSize()) + " ports\n";
                if (shipPrefs.maxVoyageMinutes > 0)
                    prefDetails += "Max time: " + to_string(shipPrefs.maxVoyageMinutes / 60) + "h";

                Text prefDetailsText;
                prefDetailsText.setFont(font);
                prefDetailsText.setString(prefDetails);
                prefDetailsText.setCharacterSize(11);
                prefDetailsText.setFillColor(Color(180, 255, 200));
                prefDetailsText.setPosition(1695, 48);
                window.draw(prefDetailsText);
            }

            RectangleShape routeInfoBox(Vector2f(420, 320));
            routeInfoBox.setFillColor(Color(30, 10, 20, 245)); 
            routeInfoBox.setOutlineThickness(3);
            routeInfoBox.setOutlineColor(Color(200, 50, 50)); 
            routeInfoBox.setPosition(20, 20);
            window.draw(routeInfoBox);

            RectangleShape infoAccent(Vector2f(420, 5));
            infoAccent.setFillColor(Color(200, 50, 50));
            infoAccent.setPosition(20, 20);
            window.draw(infoAccent);

            for (int c = 0; c < 4; c++)
            {
                float cx = (c % 2 == 0) ? 20 : 440 - 25;
                float cy = (c < 2) ? 20 : 340 - 25;
                RectangleShape cornerH(Vector2f(30, 2));
                RectangleShape cornerV(Vector2f(2, 30));
                float flicker = 0.5f + 0.5f * sin(time * 5.0 + c * 1.2f);
                Uint8 cornerAlpha = static_cast<Uint8>((150 + 100 * flicker));
                cornerH.setFillColor(Color(255, static_cast<Uint8>(80 * flicker), 80, cornerAlpha));
                cornerV.setFillColor(Color(255, static_cast<Uint8>(80 * flicker), 80, cornerAlpha));
                cornerH.setPosition(cx, cy);
                cornerV.setPosition(cx, cy);
                window.draw(cornerH);
                window.draw(cornerV);
            }

            Text infoTitle;
            infoTitle.setFont(font);
            infoTitle.setString("Booking Details");
            infoTitle.setCharacterSize(22);
            infoTitle.setStyle(Text::Bold);
            infoTitle.setFillColor(Color(255, 100, 100)); 
            infoTitle.setPosition(35, 35);
            window.draw(infoTitle);

            RectangleShape infoDivider(Vector2f(390, 2));
            infoDivider.setFillColor(Color(200, 50, 50, 100));
            infoDivider.setPosition(35, 65);
            window.draw(infoDivider);

            Text routeInfo;
            routeInfo.setFont(font);
            routeInfo.setCharacterSize(15);
            routeInfo.setFillColor(Color(255, 200, 200)); 
            routeInfo.setPosition(35, 80);

            string infoText = "Customer: " + currentBooking.customerName + "\n\n";
            infoText += "Route: " + currentBooking.origin + " -> " + currentBooking.destination + "\n";
            infoText += "Departure Date: " + currentBooking.departureDate + "\n\n";
            infoText += "Path:\n";
            for (int i = 0; i < currentBooking.routePath.getSize(); i++)
            {
                infoText += "   " + currentBooking.routePath[i];
                if (i < currentBooking.routePath.getSize() - 1)
                    infoText += " ->\n";
            }

            if (currentBooking.waitPorts.getSize() > 0)
            {
                infoText += "\n\nDocking Times:\n";
                for (int i = 0; i < currentBooking.waitPorts.getSize(); i++)
                {
                    infoText += "   " + currentBooking.waitPorts[i] + ": " + currentBooking.waitDurations[i] + "\n";
                }
            }

            routeInfo.setString(infoText);
            window.draw(routeInfo);

            RectangleShape costBox(Vector2f(420, 90));
            costBox.setFillColor(Color(40, 15, 20, 245)); 
            costBox.setOutlineThickness(3);
            costBox.setOutlineColor(Color(255, 80, 80)); 
            costBox.setPosition(20, 850);
            window.draw(costBox);

            float costGlow = 0.5f + 0.5f * static_cast<float>(sin(time * 3.0));
            RectangleShape costGlowRect(Vector2f(420, 90));
            costGlowRect.setFillColor(Color(255, 50, 50, static_cast<Uint8>(30 * costGlow)));
            costGlowRect.setPosition(20, 850);
            window.draw(costGlowRect);

            CircleShape dollarCircle(20);
            dollarCircle.setFillColor(Color(180, 40, 40)); 
            dollarCircle.setOutlineThickness(2);
            dollarCircle.setOutlineColor(Color(255, 120, 120));
            dollarCircle.setPosition(35, 875);
            window.draw(dollarCircle);

            Text dollarSign;
            dollarSign.setFont(font);
            dollarSign.setString("$");
            dollarSign.setCharacterSize(24);
            dollarSign.setStyle(Text::Bold);
            dollarSign.setFillColor(Color::White);
            dollarSign.setPosition(47, 878);
            window.draw(dollarSign);

            Text costLabel;
            costLabel.setFont(font);
            costLabel.setString("Total Cost");
            costLabel.setCharacterSize(16);
            costLabel.setFillColor(Color(255, 180, 180)); 
            costLabel.setPosition(85, 860);
            window.draw(costLabel);

            Text costText;
            costText.setFont(font);
            costText.setString("$" + to_string(currentBooking.totalCost));
            costText.setCharacterSize(32);
            costText.setFillColor(Color(255, 100, 100)); 
            costText.setStyle(Text::Bold);
            costText.setPosition(85, 885);
            window.draw(costText);

            bool confirmHover = confirmButton.getGlobalBounds().contains(mousePos);

            if (confirmHover)
            {
                RectangleShape confirmGlow(Vector2f(320, 85));
                confirmGlow.setFillColor(Color(255, 50, 50, 50)); 
                confirmGlow.setPosition(1575, 925);
                window.draw(confirmGlow);
            }

            confirmButton.setSize(Vector2f(300, 65));
            confirmButton.setPosition(1585, 935);
            if (confirmHover)
            {
                float pulse = 1.0f + 0.03f * static_cast<float>(sin(time * 5.0));
                confirmButton.setScale(pulse, pulse);
                confirmButton.setFillColor(Color(220, 50, 60));
                confirmButton.setOutlineThickness(4);
                confirmButton.setOutlineColor(Color(255, 150, 150));
            }
            else
            {
                confirmButton.setScale(1.0f, 1.0f);
                confirmButton.setFillColor(Color(180, 40, 50));
                confirmButton.setOutlineThickness(3);
                confirmButton.setOutlineColor(Color(255, 100, 100));
            }
            window.draw(confirmButton);

            Text checkIcon;
            checkIcon.setFont(font);
            checkIcon.setString("*");
            checkIcon.setCharacterSize(28);
            checkIcon.setFillColor(Color::White);
            checkIcon.setPosition(confirmHover ? 1603 : 1605, confirmHover ? 948 : 950);
            window.draw(checkIcon);

            confirmBtnText.setString("CONFIRM BOOKING");
            confirmBtnText.setCharacterSize(20);
            confirmBtnText.setFillColor(Color::White);
            confirmBtnText.setStyle(Text::Bold);
            confirmBtnText.setPosition(confirmHover ? 1618 : 1620, confirmHover ? 953 : 955);
            window.draw(confirmBtnText);
        }
        else if (currentState == SUCCESS)
        {
            
            for (int p = 0; p < 60; p++)
            {
                float px = static_cast<float>((p * 41) % 1920);
                float py = static_cast<float>(fmod(1080.0 + fmod(time * 30.0 + p * 30.0, 1200.0), 1200.0));
                float flicker = 0.5f + 0.5f * static_cast<float>(sin(time * 6.0 + p * 0.3f));
                CircleShape particle(2.0f + flicker * 2.0f);
                particle.setFillColor(Color(255, static_cast<Uint8>(100 + 80 * flicker), 50, static_cast<Uint8>(50 + 50 * flicker)));
                particle.setPosition(px, py);
                window.draw(particle);
            }

            RectangleShape successGlow(Vector2f(680, 500));
            successGlow.setFillColor(Color(255, 50, 50, 20)); 
            successGlow.setPosition(620, 290);
            window.draw(successGlow);

            RectangleShape successBox(Vector2f(660, 480));
            successBox.setFillColor(Color(30, 10, 20, 250)); 
            successBox.setOutlineThickness(4);
            successBox.setOutlineColor(Color(200, 50, 50)); 
            successBox.setPosition(630, 300);
            window.draw(successBox);

            RectangleShape successAccent(Vector2f(660, 6));
            successAccent.setFillColor(Color(200, 50, 50)); 
            successAccent.setPosition(630, 300);
            window.draw(successAccent);

            for (int c = 0; c < 4; c++)
            {
                float cx = (c % 2 == 0) ? 630 : 1290 - 40;
                float cy = (c < 2) ? 300 : 780 - 40;
                RectangleShape cornerH(Vector2f(50, 3));
                RectangleShape cornerV(Vector2f(3, 50));
                float flicker = 0.5f + 0.5f * sin(time * 5.0 + c * 1.2f);
                Uint8 alpha = static_cast<Uint8>((180 + 75 * flicker));
                cornerH.setFillColor(Color(255, static_cast<Uint8>(100 * flicker), 100, alpha));
                cornerV.setFillColor(Color(255, static_cast<Uint8>(100 * flicker), 100, alpha));
                cornerH.setPosition(cx, cy);
                cornerV.setPosition(cx, cy);
                window.draw(cornerH);
                window.draw(cornerV);
            }

            float checkPulse = 1.0f + 0.1f * static_cast<float>(sin(time * 3.0));
            CircleShape checkCircle(45 * checkPulse);
            checkCircle.setFillColor(Color(180, 40, 40)); 
            checkCircle.setOutlineThickness(4);
            checkCircle.setOutlineColor(Color(255, 120, 120)); 
            checkCircle.setOrigin(45 * checkPulse, 45 * checkPulse);
            checkCircle.setPosition(960, 380);
            window.draw(checkCircle);

            Text checkMark;
            checkMark.setFont(font);
            checkMark.setString("OK");
            checkMark.setCharacterSize(28);
            checkMark.setStyle(Text::Bold);
            checkMark.setFillColor(Color::White);
            checkMark.setPosition(940, 365);
            window.draw(checkMark);

            Text title;
            title.setFont(font);
            title.setString("Booking Confirmed!");
            title.setCharacterSize(38);
            title.setStyle(Text::Bold);
            
            float titleGlow = 0.6f + 0.4f * static_cast<float>(sin(time * 4.0));
            title.setFillColor(Color(255, static_cast<Uint8>(80 + 50 * titleGlow), static_cast<Uint8>(80 + 30 * titleGlow)));
            title.setOutlineThickness(2);
            title.setOutlineColor(Color(100, 0, 0, 150));
            FloatRect titleBounds = title.getLocalBounds();
            title.setPosition(960 - titleBounds.width / 2, 440);
            window.draw(title);

            RectangleShape successDivider(Vector2f(500, 2));
            successDivider.setFillColor(Color(200, 50, 50, 100)); 
            successDivider.setPosition(730, 495);
            window.draw(successDivider);

            float detailY = 520;
            float labelX = 680;
            float valueX = 880;

            auto drawDetail = [&](string label, string value, float y, Color valueColor = Color(255, 200, 200))
            {
                Text labelText;
                labelText.setFont(font);
                labelText.setString(label);
                labelText.setCharacterSize(18);
                labelText.setFillColor(Color(200, 120, 120)); 
                labelText.setPosition(labelX, y);
                window.draw(labelText);

                Text valueText;
                valueText.setFont(font);
                valueText.setString(value);
                valueText.setCharacterSize(18);
                valueText.setStyle(Text::Bold);
                valueText.setFillColor(valueColor);
                valueText.setPosition(valueX, y);
                window.draw(valueText);
            };

            drawDetail("Booking ID:", currentBooking.bookingID, detailY, Color(255, 150, 100)); 
            drawDetail("Customer:", currentBooking.customerName, detailY + 35);
            drawDetail("Route:", currentBooking.origin + " -> " + currentBooking.destination, detailY + 70);
            drawDetail("Date:", currentBooking.departureDate, detailY + 105);
            
            RectangleShape costBreakdownBox(Vector2f(500, 200));
            costBreakdownBox.setFillColor(Color(40, 10, 20, 200)); 
            costBreakdownBox.setOutlineThickness(2);
            costBreakdownBox.setOutlineColor(Color(200, 60, 60)); 
            costBreakdownBox.setPosition(680, detailY + 145);
            window.draw(costBreakdownBox);
            
            Text costHeader;
            costHeader.setFont(font);
            costHeader.setString("COST BREAKDOWN");
            costHeader.setCharacterSize(14);
            costHeader.setFillColor(Color(255, 120, 120)); 
            costHeader.setStyle(Text::Bold);
            costHeader.setPosition(690, detailY + 150);
            window.draw(costHeader);
            
            auto formatWaitTime = [](int minutes) -> string {
                int days = minutes / (24 * 60);
                int hours = (minutes % (24 * 60)) / 60;
                int mins = minutes % 60;
                return to_string(days) + "d " + to_string(hours) + "h " + to_string(mins) + "m";
            };
            
            drawDetail("Base Route Cost:", "$" + to_string(currentBooking.baseRouteCost), detailY + 175, Color(255, 180, 180));
            
            
            int totalPortCharges = 0;
            float yOffset = detailY + 200;
            
            Text portChargesLabel;
            portChargesLabel.setFont(font);
            portChargesLabel.setString("Port Charges:");
            portChargesLabel.setCharacterSize(14);
            portChargesLabel.setFillColor(Color(255, 150, 150)); 
            portChargesLabel.setPosition(700, yOffset);
            window.draw(portChargesLabel);
            
            yOffset += 25;
            for (int i = 0; i < currentBooking.routePath.getSize(); i++)
            {
                string portName = currentBooking.routePath[i];
                int portIdx = graph.findPortIndex(portName);
                if (portIdx != -1)
                {
                    int portCharge = graph.ports[portIdx].charge;
                    totalPortCharges += portCharge;
                    
                    Text portDetail;
                    portDetail.setFont(font);
                    portDetail.setString("  - " + portName + ": $" + to_string(portCharge));
                    portDetail.setCharacterSize(13);
                    portDetail.setFillColor(Color(255, 180, 180)); 
                    portDetail.setPosition(710, yOffset);
                    window.draw(portDetail);
                    yOffset += 20;
                }
            }
            
            Text totalDockingLabel;
            totalDockingLabel.setFont(font);
            totalDockingLabel.setString("Total Docking Charges: $" + to_string(totalPortCharges));
            totalDockingLabel.setCharacterSize(14);
            totalDockingLabel.setFillColor(Color(255, 120, 120)); 
            totalDockingLabel.setStyle(Text::Bold);
            totalDockingLabel.setPosition(700, yOffset);
            window.draw(totalDockingLabel);
            
            yOffset += 30;
            
            if (currentBooking.queueWaitCharges > 0) {
                string queueWaitStr = "$" + to_string(currentBooking.queueWaitCharges);
                if (currentBooking.dockingWaitTime > 0) {
                    queueWaitStr += " (" + to_string(currentBooking.dockingWaitTime/60) + "h " + to_string(currentBooking.dockingWaitTime%60) + "m queue wait)";
                }
                drawDetail("Queue Wait Charges:", queueWaitStr, yOffset, Color(255, 150, 100)); 
                yOffset += 25;
            }
            
            RectangleShape costDivider(Vector2f(460, 2));
            costDivider.setFillColor(Color(200, 60, 60, 150)); 
            costDivider.setPosition(700, yOffset);
            window.draw(costDivider);
            
            yOffset += 10;
            drawDetail("TOTAL COST:", "$" + to_string(currentBooking.totalCost), yOffset, Color(255, 100, 100)); 

            Text successNote;
            successNote.setFont(font);
            successNote.setString("Your booking has been saved successfully!");
            successNote.setCharacterSize(16);
            successNote.setFillColor(Color(255, 180, 180)); 
            FloatRect noteBounds = successNote.getLocalBounds();
            successNote.setPosition(960 - noteBounds.width / 2, 710);
            window.draw(successNote);

            Text backNote;
            backNote.setFont(font);
            backNote.setString("Press BACK to return to main menu");
            backNote.setCharacterSize(14);
            backNote.setFillColor(Color(200, 150, 150)); 
            FloatRect backNoteBounds = backNote.getLocalBounds();
            backNote.setPosition(960 - backNoteBounds.width / 2, 740);
            window.draw(backNote);
        }

        mousePos = window.mapPixelToCoords(Mouse::getPosition(window));
        bool backHover = backButton.getGlobalBounds().contains(mousePos);
        if (backHover)
        {
            backButton.setFillColor(Color(150, 40, 60)); 
            backButton.setOutlineColor(Color(255, 120, 140));
            RectangleShape backGlow(Vector2f(200, 75));
            backGlow.setFillColor(Color(255, 50, 80, 50)); 
            backGlow.setPosition(95, 965);
            window.draw(backGlow);
        }
        else
        {
            backButton.setFillColor(Color(100, 20, 40)); 
            backButton.setOutlineColor(Color(200, 60, 80));
        }
        window.draw(backButton);
        backBtnText.setFillColor(Color::White);
        window.draw(backBtnText);

        window.display();
    }
}

#endif