#include <iostream>
#include <string>
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <fstream>
#include <cmath>
#include "Queue.h"
#include "Vector.h"
using namespace sf;
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
    
    ~PortNode() {
        RouteNode* current = routeHead;
        while (current != nullptr) {
            RouteNode* next = current->next;
            delete current;
            current = next;
        }
        routeHead = nullptr;
    }
};

struct PortLocation {
    string name;
    float x;
    float y;
};

PortLocation portLocations[] = {
    {"AbuDhabi", 1280, 340},
    {"Alexandria", 1092, 286},
    {"Antwerp", 952, 122},
    {"Athens", 1027, 251},
    {"Busan", 1829, 224},       
    {"CapeTown", 1027, 849},
    {"Chittagong", 1553, 370},
    {"Colombo", 1491, 503},
    {"Copenhagen", 987, 82},
    {"Doha", 1310, 358},
    {"Dubai", 1301, 392},
    {"Dublin", 850, 105},
    {"Durban", 1109, 814},
    {"Genoa", 954, 174},
    {"Hamburg", 969, 105},
    {"Helsinki", 1136, 40},
    {"HongKong", 1770, 322},
    {"Istanbul", 1121, 244},
    {"Jakarta", 1697, 605},
    {"Jeddah", 1175, 365},
    {"Karachi", 1380, 338},
    {"Lisbon", 816, 230},
    {"London", 888, 117},
    {"LosAngeles", 150, 350},    
    {"Manila", 1824, 472},
    {"Marseille", 990, 178},
    {"Melbourne", 1830, 840},
    {"Montreal", 301, 44},
    {"Mumbai", 1436, 401},
    {"NewYork", 278, 174},
    {"Osaka", 1870, 269},
    {"Oslo", 958, 40},
    {"PortLouis", 1317, 736},
    {"Rotterdam", 923, 109},
    {"Shanghai", 1774, 280},
    {"Singapore", 1674, 555},
    {"Stockholm", 1034, 28},
    {"Sydney", 1870, 870},
    {"Tokyo", 1875, 226},
    {"Vancouver", 53, 105}
};
const int numLocations = sizeof(portLocations) / sizeof(portLocations[0]);

class InputBox {
public:
    RectangleShape box;
    Text text;
    string inputString;
    bool isActive;
    string label;
    Text labelText;
    
    InputBox(Font& font, string lbl, Vector2f position) {
        label = lbl;
        box.setSize(Vector2f(300, 40));
        box.setFillColor(Color(0, 20, 60, 240));
        box.setOutlineThickness(2);
        box.setOutlineColor(Color(100, 200, 255));
        box.setPosition(position);
        
        labelText.setFont(font);
        labelText.setString(label + ":");
        labelText.setCharacterSize(16);
        labelText.setFillColor(Color(200, 230, 255));
        labelText.setPosition(position.x, position.y - 25);
        
        text.setFont(font);
        text.setCharacterSize(18);
        text.setFillColor(Color::White);
        text.setPosition(position.x + 10, position.y + 8);
        
        inputString = "";
        isActive = false;
    }
    
    void handleEvent(Event& event, RenderWindow& window) {
        if (event.type == Event::MouseButtonPressed) {
            Vector2f mousePos = window.mapPixelToCoords(Mouse::getPosition(window));
            if (box.getGlobalBounds().contains(mousePos)) {
                isActive = true;
                box.setOutlineColor(Color::Yellow);
            } else {
                isActive = false;
                box.setOutlineColor(Color(100, 200, 255));
            }
        }
        
        if (event.type == Event::TextEntered && isActive) {
            if (event.text.unicode == '\b') {
                if (!inputString.empty()) {
                    inputString.pop_back();
                }
            } else if (event.text.unicode < 128 && event.text.unicode != '\r' && event.text.unicode != '\t') {
                inputString += static_cast<char>(event.text.unicode);
            }
            text.setString(inputString);
        }
    }
    
    void draw(RenderWindow& window) {
        window.draw(labelText);
        window.draw(box);
        window.draw(text);
    }
    
    string getText() { return inputString; }
    void setText(string newText) { 
        inputString = newText; 
        text.setString(inputString);
    }
};

class Maps
{
public:
    Vector<PortNode> ports;

    void addPort(string name, int ch)
    {
        PortNode p;
        p.portName = name;
        p.charge = ch;
        ports.push_back(p);
    }

    int findPortIndex(string s)
    {
        for (int i = 0; i < ports.getSize(); i++)
        {
            if (ports[i].portName == s)
            {
                return i;
            }
        }
        return -1;
    }

    void addRoute(string start, string dest, string depDate, string depTime, string arrTime, int cost, string comp)
    {
        int s = findPortIndex(start);
        int d = findPortIndex(dest);
        if (s == -1 || d == -1)
        {
            cout << "Cannot add Route: Port not found - " << start << " or " << dest << endl;
            return;
        }
        RouteNode *newroute = new RouteNode(start, dest, depDate, depTime, arrTime, cost, comp);
        newroute->next = ports[s].routeHead;
        ports[s].routeHead = newroute;
    }

    bool checkDirectRoutes(string origin, string des, string date, Vector<int>& path)
    {
        int originindex = findPortIndex(origin);
        int desindex = findPortIndex(des);
        
        if (originindex == -1 || desindex == -1){
            return false;
        }
        
        RouteNode *temp = ports[originindex].routeHead;
        while (temp != nullptr){
            if (temp->destinationPort == des && temp->departureDate == date){
                path.push_back(originindex);
                path.push_back(desindex);
                return true;
            }
            temp = temp->next;
        }
        return false;
    }

bool checkConnectingRoutes(string origin, string dest, string date, Vector<int>& path) {
    const int MIN_DOCKING_DELAY = 120; 

    auto convertToMinutes = [](string t) {
        int h = stoi(t.substr(0, 2));
        int m = stoi(t.substr(3, 2));
        return h * 60 + m;
    };

    int org_index = findPortIndex(origin);
    int des_index = findPortIndex(dest);
    if (org_index == -1 || des_index == -1) return false;

    Queue q;
    Vector<int> startPath;
    startPath.push_back(org_index);
    q.enqueue(startPath);

    bool found = false;

    while (!q.isEmpty() && !found) {
        Vector<int> currentPath = q.frontfind();
        q.dequeue();
        int currentPort = currentPath[currentPath.getSize() - 1]; 

        RouteNode* route = ports[currentPort].routeHead;
        while (route != nullptr && !found) {
            int nextIndex = findPortIndex(route->destinationPort);
            if (nextIndex == -1) {
                route = route->next;
                continue;
            }

            if (currentPath.getSize() == 1 && route->departureDate != date) {
                route = route->next;
                continue;
            }

            if (currentPath.getSize() > 1) {
                int prevArrival = convertToMinutes(route->arrivalTime); 
                int nextDeparture = convertToMinutes(route->departureTime); 
                if (nextDeparture < prevArrival + MIN_DOCKING_DELAY) {
                    route = route->next;
                    continue;
                }
            }

            Vector<int> newPath = currentPath;
            newPath.push_back(nextIndex);

            if (nextIndex == des_index) {
                path = newPath;
                found = true;
                break;
            } else {
                q.enqueue(newPath);
            }

            route = route->next;
        }
    }

    return found;
}



    int findPortLocationIndex(const string& portName) {
        for (int i = 0; i < numLocations; i++) {
            if (portLocations[i].name == portName) {
                return i;
            }
        }
        return -1;
    }

bool findAndDisplayRoute(Maps &graph, string origin, string destination, string date, Vector<int>& currentPath, string& resultText, Color& resultColor) {
    currentPath.clear();
    
    bool directFound = graph.checkDirectRoutes(origin, destination, date, currentPath);
    if (directFound) {
        resultText = "Direct route found!";
        resultColor = Color::Green;
        return true;
    }
    
    bool connectingFound = graph.checkConnectingRoutes(origin, destination, date, currentPath);
    if (connectingFound) {
        resultText = "Connecting route found!";
        resultColor = Color::Green;
        return true;
    }
    
    resultText = "No route found!";
    resultColor = Color::Red;
    return false;
}

void showMap(Maps &graph) {
    RenderWindow window(VideoMode(1920, 1080), "OceanRoute Navigator");
    window.setFramerateLimit(60);
    
    Texture mapTexture;
    if (!mapTexture.loadFromFile("pics/map2.png")) {
        cout << "Map image not loaded!" << endl;
        return;
    }
    Sprite mapSprite(mapTexture);
    
    mapSprite.setColor(Color(180, 220, 255, 255));

    Font font;
    if (!font.loadFromFile("Roboto.ttf")) {
        cout << "Font load failed!" << endl;
        return;
    }

    float animationStartTime = 0.0f;
    bool animationStarted = false;

    RectangleShape searchButton(Vector2f(220, 60));
    searchButton.setFillColor(Color(0, 100, 200, 200));
    searchButton.setOutlineThickness(3);
    searchButton.setOutlineColor(Color(100, 200, 255));
    searchButton.setPosition(40, 880);
    
    CircleShape buttonGlow(40);
    buttonGlow.setFillColor(Color(100, 200, 255, 80));
    buttonGlow.setPosition(10, 970);
    
    Text searchButtonText;
    searchButtonText.setFont(font);
    searchButtonText.setString("Search Route");
    searchButtonText.setCharacterSize(22);
    searchButtonText.setFillColor(Color::White);
    searchButtonText.setStyle(Text::Bold);
    searchButtonText.setPosition(55, 895);

    bool showInputWindow = false;
    RectangleShape inputWindow(Vector2f(500, 400));
    inputWindow.setFillColor(Color(0, 30, 70, 250));
    inputWindow.setOutlineThickness(4);
    inputWindow.setOutlineColor(Color(100, 200, 255));
    inputWindow.setPosition(710, 340);

    Text windowTitle;
    windowTitle.setFont(font);
    windowTitle.setString("Route Search");
    windowTitle.setCharacterSize(28);
    windowTitle.setFillColor(Color(100, 255, 255));
    windowTitle.setStyle(Text::Bold);
    windowTitle.setOutlineThickness(2);
    windowTitle.setOutlineColor(Color(0, 0, 0, 150));
    windowTitle.setPosition(810, 360);

    class EnhancedInputBox {
    public:
        RectangleShape box;
        Text text;
        Text label;
        string inputString;
        bool isActive;
        float time;
        
        EnhancedInputBox(Font& font, string lbl, Vector2f position) {
            label.setFont(font);
            label.setString(lbl);
            label.setCharacterSize(18);
            label.setFillColor(Color(200, 230, 255));
            label.setStyle(Text::Bold);
            label.setPosition(position.x, position.y - 30);
            
            box.setSize(Vector2f(460, 50));
            box.setFillColor(Color(0, 20, 50, 240));
            box.setOutlineThickness(2);
            box.setOutlineColor(Color(150, 200, 255));
            box.setPosition(position.x, position.y);
            
            text.setFont(font);
            text.setCharacterSize(20);
            text.setFillColor(Color::White);
            text.setPosition(position.x + 15, position.y + 12);
            
            inputString = "";
            isActive = false;
            time = 0.0f;
        }
        
        void handleEvent(Event& event, RenderWindow& window) {
            Vector2f mousePos = window.mapPixelToCoords(Mouse::getPosition(window));
            
            if (event.type == Event::MouseButtonPressed) {
                if (box.getGlobalBounds().contains(mousePos)) {
                    isActive = true;
                    box.setOutlineColor(Color::Yellow);
                } else {
                    isActive = false;
                    box.setOutlineColor(Color(150, 200, 255));
                }
            }
            
            if (event.type == Event::TextEntered && isActive) {
                if (event.text.unicode == '\b') {
                    if (!inputString.empty()) {
                        inputString.pop_back();
                    }
                } 
                else if (event.text.unicode == 13) {
                }
                else if (event.text.unicode < 128 && event.text.unicode != '\r' && event.text.unicode != '\t') {
                    if (inputString.length() < 50) {
                        inputString += static_cast<char>(event.text.unicode);
                    }
                }
                text.setString(inputString + (isActive ? "|" : ""));
            }
        }
        
        void update(float deltaTime) {
            time += deltaTime;
            if (isActive) {
                float pulse = sin(time * 8.0f) * 0.3f + 0.7f;
                box.setOutlineColor(Color(255, 255, 150, 255 * pulse));
            }
        }
        
        void draw(RenderWindow& window) {
            window.draw(label);
            window.draw(box);
            
            string displayText = inputString;
            if (isActive && fmod(time, 1.0f) > 0.5f) {
                displayText += "|";
            }
            text.setString(displayText);
            window.draw(text);
        }
        
        string getText() { return inputString; }
        void setText(string newText) { 
            inputString = newText; 
            text.setString(inputString);
        }
    };

    EnhancedInputBox originInput(font, "Departure Port", Vector2f(730, 430));
    EnhancedInputBox destinationInput(font, "Destination Port", Vector2f(730, 520));
    EnhancedInputBox dateInput(font, "Date (YYYY-MM-DD)", Vector2f(730, 610));
    
    RectangleShape popupSearchButton(Vector2f(180, 50));
    popupSearchButton.setFillColor(Color(0, 180, 120));
    popupSearchButton.setOutlineThickness(2);
    popupSearchButton.setOutlineColor(Color(100, 255, 180));
    popupSearchButton.setPosition(750, 690);
    
    Text popupSearchText;
    popupSearchText.setFont(font);
    popupSearchText.setString("FIND ROUTE");
    popupSearchText.setCharacterSize(20);
    popupSearchText.setFillColor(Color::White);
    popupSearchText.setStyle(Text::Bold);
    popupSearchText.setPosition(780, 702);
    
    RectangleShape closeButton(Vector2f(180, 50));
    closeButton.setFillColor(Color(180, 80, 80));
    closeButton.setOutlineThickness(2);
    closeButton.setOutlineColor(Color(255, 150, 150));
    closeButton.setPosition(950, 690);
    
    Text closeText;
    closeText.setFont(font);
    closeText.setString("CANCEL");
    closeText.setCharacterSize(20);
    closeText.setFillColor(Color::White);
    closeText.setStyle(Text::Bold);
    closeText.setPosition(985, 702);

    CircleShape searchButtonGlow(30);
    searchButtonGlow.setFillColor(Color(100, 255, 180, 100));
    searchButtonGlow.setPosition(745, 685);
    
    CircleShape closeButtonGlow(30);
    closeButtonGlow.setFillColor(Color(255, 150, 150, 100));
    closeButtonGlow.setPosition(945, 685);

    int selectedPortIndex = -1;
    RouteNode* hoveredRoute = nullptr;
    bool showRoutes = false;
    float time = 0.0f;
    Vector<float> portPulse;
    
    Vector<int> currentPath;
    bool showSearchResult = false;
    string searchResultText = "";
    Color searchResultColor = Color::White;
    
    Vector<RectangleShape> waterRipples;
    for (int i = 0; i < 20; i++) {
        RectangleShape ripple(Vector2f(rand() % 200 + 50, rand() % 200 + 50));
        ripple.setFillColor(Color(30, 60, 120, 30));
        ripple.setPosition(rand() % 1920, rand() % 1080);
        waterRipples.push_back(ripple);
    }

    for (int i = 0; i < graph.ports.getSize(); i++) {
        portPulse.push_back(0.0f);
    }

    // Animation variables
    float animationTime = 0.0f;
    const float ANIMATION_DURATION = 8.0f;

    while (window.isOpen()) {
        Event event;
        Vector2f mousePos = window.mapPixelToCoords(Mouse::getPosition(window));
        float deltaTime = 0.016f;
        time += deltaTime;
        animationTime += deltaTime;

        while (window.pollEvent(event)) {
            if (event.type == Event::Closed) window.close();

            if (event.type == Event::MouseButtonPressed && event.mouseButton.button == Mouse::Left) {
                Vector2i clickPos = Mouse::getPosition(window);
                
                if (searchButton.getGlobalBounds().contains(mousePos) && !showInputWindow) {
                    showInputWindow = true;
                    originInput.setText("");
                    destinationInput.setText("");
                    dateInput.setText("");
                    showSearchResult = false;
                    originInput.isActive = true;
                    destinationInput.isActive = false;
                    dateInput.isActive = false;
                    animationTime = 0.0f;
                }
                
                if (showInputWindow) {
                    originInput.handleEvent(event, window);
                    destinationInput.handleEvent(event, window);
                    dateInput.handleEvent(event, window);
                    
                    if (popupSearchButton.getGlobalBounds().contains(mousePos)) {
                        string origin = originInput.getText();
                        string destination = destinationInput.getText();
                        string date = dateInput.getText();
                        
                        if (origin.empty() || destination.empty() || date.empty()) {
                            searchResultText = "Please fill all fields!";
                            searchResultColor = Color::Red;
                            showSearchResult = true;
                        } else {
                            bool routeFound = findAndDisplayRoute(graph, origin, destination, date, currentPath, searchResultText, searchResultColor);
                            showSearchResult = true;
                            showRoutes = true;
                            showInputWindow = false;
                            // Reset animation
                            animationStarted = true;
                            animationStartTime = time; 
                        }
                    }
                    if (closeButton.getGlobalBounds().contains(mousePos)) {
                        showInputWindow = false;
                        showSearchResult = false;
                    }
                }
                
                if (!showInputWindow) {
                    for (int i = 0; i < numLocations; i++) {
                        float px = portLocations[i].x;
                        float py = portLocations[i].y;
                        float dist = sqrt(pow(mousePos.x - px, 2) + pow(mousePos.y - py, 2));
                        if (dist < 12) {
                            selectedPortIndex = i;
                            showRoutes = true;
                            portPulse[i] = 1.0f;
                            showSearchResult = false;
                            currentPath.clear();
                            animationTime = 0.0f;
                        }
                    }
                }
            }

            if (showInputWindow) {
                originInput.handleEvent(event, window);
                destinationInput.handleEvent(event, window);
                dateInput.handleEvent(event, window);
            }

            if (event.type == Event::KeyPressed && event.key.code == Keyboard::Escape) {
                if (showInputWindow) {
                    showInputWindow = false;
                    showSearchResult = false;
                } else {
                    selectedPortIndex = -1;
                    showRoutes = false;
                    hoveredRoute = nullptr;
                    showSearchResult = false;
                    currentPath.clear();
                    animationTime = 0.0f;
                }
            }
            
            if (event.type == Event::KeyPressed && event.key.code == Keyboard::Tab && showInputWindow) {
                if (originInput.isActive) {
                    originInput.isActive = false;
                    destinationInput.isActive = true;
                } else if (destinationInput.isActive) {
                    destinationInput.isActive = false;
                    dateInput.isActive = true;
                } else {
                    dateInput.isActive = false;
                    originInput.isActive = true;
                }
            }
        }

        for (int i = 0; i < portPulse.getSize(); i++) {
            if (portPulse[i] > 0) {
                portPulse[i] -= 0.05f;
                if (portPulse[i] < 0) portPulse[i] = 0;
            }
        }

        if (showInputWindow) {
            originInput.update(deltaTime);
            destinationInput.update(deltaTime);
            dateInput.update(deltaTime);
        }

        for (int i = 0; i < waterRipples.getSize(); i++) {
            float scale = 1.0f + 0.1f * sin(time * 0.5f + waterRipples[i].getPosition().x * 0.01f);
            waterRipples[i].setScale(scale, scale);
            Color c = waterRipples[i].getFillColor();
            c.a = 20 + 10 * sin(time * 0.3f + waterRipples[i].getPosition().y * 0.01f);
            waterRipples[i].setFillColor(c);
        }

        window.clear(Color(10, 20, 40)); 

        for (int i = 0; i < waterRipples.getSize(); i++) {
            window.draw(waterRipples[i]);
        }
        window.draw(mapSprite);

   if (showSearchResult && currentPath.getSize() > 1) {
    // Compute total path length
    float totalLength = 0.0f;
    Vector<float> segmentLengths;
    for (int i = 0; i < currentPath.getSize() - 1; i++) {
        int startIdx = currentPath[i];
        int endIdx = currentPath[i + 1];
        Vector2f start(portLocations[startIdx].x, portLocations[startIdx].y);
        Vector2f end(portLocations[endIdx].x, portLocations[endIdx].y);
        float len = sqrt(pow(end.x - start.x, 2) + pow(end.y - start.y, 2));
        segmentLengths.push_back(len);
        totalLength += len;
    }

    float speed = 200.0f; 
    float traveled = 0.0f;
    
    if (animationStarted) {
        traveled = (time - animationStartTime) * speed;
    }

    // Draw lines and glow
    for (int i = 0; i < currentPath.getSize() - 1; i++) {
        int startIdx = currentPath[i];
        int endIdx = currentPath[i + 1];
        Vector2f start(portLocations[startIdx].x, portLocations[startIdx].y);
        Vector2f end(portLocations[endIdx].x, portLocations[endIdx].y);

        float angle = atan2(end.y - start.y, end.x - start.x) * 180 / 3.14159f;
        float length = segmentLengths[i];

        RectangleShape routeLine(Vector2f(length, 5.0f));
        routeLine.setPosition(start);
        routeLine.setRotation(angle);
        routeLine.setFillColor(Color::Cyan);
        window.draw(routeLine);

        RectangleShape glowLine(Vector2f(length, 10.0f));
        glowLine.setPosition(start);
        glowLine.setRotation(angle);
        glowLine.setFillColor(Color(0, 255, 255, 80));
        window.draw(glowLine);
    }

    // Only draw dot if it hasn't reached destination yet
    if (traveled < totalLength) {
        Vector2f dotPos;
        float remaining = traveled;
        
        for (int i = 0; i < currentPath.getSize() - 1; i++) {
            if (remaining <= segmentLengths[i]) {
                int startIdx = currentPath[i];
                int endIdx = currentPath[i + 1];
                Vector2f start(portLocations[startIdx].x, portLocations[startIdx].y);
                Vector2f end(portLocations[endIdx].x, portLocations[endIdx].y);

                float t = remaining / segmentLengths[i];
                dotPos = start + (end - start) * t;
                break;
            }
            remaining -= segmentLengths[i];
        }

        // Draw the moving dot
        CircleShape routeDot(8.f);
        routeDot.setFillColor(Color::Yellow);
        routeDot.setOutlineColor(Color::White);
        routeDot.setOutlineThickness(2);
        routeDot.setOrigin(8, 8);
        routeDot.setPosition(dotPos);
        window.draw(routeDot);
    }
}

        if (showRoutes && selectedPortIndex != -1 && !showSearchResult) {
            hoveredRoute = nullptr;
            RouteNode* route = graph.ports[selectedPortIndex].routeHead;
            int routeIndex = 0;
            
            while (route) {
                int destIdx = -1;
                for (int k = 0; k < numLocations; k++) {
                    if (portLocations[k].name == route->destinationPort) destIdx = k;
                }
                if (destIdx != -1) {
                    Vector2f start(portLocations[selectedPortIndex].x, portLocations[selectedPortIndex].y);
                    Vector2f end(portLocations[destIdx].x, portLocations[destIdx].y);

                    float length = sqrt(pow(end.x - start.x, 2) + pow(end.y - start.y, 2));
                    float angle = atan2(end.y - start.y, end.x - start.x) * 180 / 3.14159f;

                    float baseThickness = (route->cost / 5000.0f) + 1.5f;
                    if (baseThickness > 6.0f) baseThickness = 6.0f;
                    
                    float pulse = 0.2f * sin(time * 3.0f + routeIndex);
                    float thickness = baseThickness * (1.0f + pulse);

                    RectangleShape line(Vector2f(length, thickness));
                    line.setPosition(start);
                    line.setRotation(angle);

                    Color lineColor;
                    if (route->cost < 7000) lineColor = Color(100, 255, 150);
                    else if (route->cost < 10000) lineColor = Color(255, 255, 100);
                    else lineColor = Color(255, 100, 100);
                    
                    RectangleShape glowLine(Vector2f(length, thickness + 4.0f));
                    glowLine.setPosition(start);
                    glowLine.setRotation(angle);
                    Color glowColor = lineColor;
                    glowColor.a = 80;
                    glowLine.setFillColor(glowColor);
                    window.draw(glowLine);
                    
                    line.setFillColor(lineColor);
                    window.draw(line);
                    
                    float distance = abs((end.y - start.y) * mousePos.x - (end.x - start.x) * mousePos.y + end.x * start.y - end.y * start.x) 
                                     / sqrt(pow(end.y - start.y, 2) + pow(end.x - start.x, 2));
                    if (distance < 20.0f) hoveredRoute = route;
                }
                route = route->next;
                routeIndex++;
            }
        }

        for (int i = 0; i < numLocations; i++) {
            CircleShape glow(16.f);
            glow.setOrigin(16, 16);
            glow.setFillColor(Color(100, 180, 255, 100));
            glow.setPosition(portLocations[i].x, portLocations[i].y);
            window.draw(glow);
            
            float pulseScale = 1.0f + 0.2f * portPulse[i];
            CircleShape dot(8.f * pulseScale);
            dot.setOrigin(8.f * pulseScale, 8.f * pulseScale);
            
            if (showSearchResult && currentPath.getSize() > 0) {
                bool inPath = false;
                for (int j = 0; j < currentPath.getSize(); j++) {
                    if (currentPath[j] == i) {
                        inPath = true;
                        break;
                    }
                }
                if (inPath) {
                    dot.setFillColor(Color::Cyan);
                } else {
                    dot.setFillColor(Color(100, 100, 200));
                }
            } else if (i == selectedPortIndex) {
                dot.setFillColor(Color::Red);
                CircleShape selectionGlow(20.f);
                selectionGlow.setOrigin(20, 20);
                selectionGlow.setFillColor(Color(255, 50, 50, 100));
                selectionGlow.setPosition(portLocations[i].x, portLocations[i].y);
                window.draw(selectionGlow);
            } else if (showRoutes && selectedPortIndex != -1) {
                dot.setFillColor(Color(100, 200, 255));
            } else {
                float gentlePulse = 0.5f + 0.5f * sin(time * 2.0f + i);
                dot.setFillColor(Color(0, 150 * gentlePulse, 255));
            }
            
            dot.setOutlineColor(Color::White);
            dot.setOutlineThickness(1.5f);
            dot.setPosition(portLocations[i].x, portLocations[i].y);
            window.draw(dot);
        }

        if (selectedPortIndex != -1 || showSearchResult) {
            auto drawLabel = [&](int idx, Color fillColor, unsigned int size, float alpha = 255.0f) {
                Text label;
                label.setFont(font);
                label.setString(portLocations[idx].name);
                label.setCharacterSize(size);
                label.setStyle(Text::Bold);
                label.setOutlineThickness(1.f);
                label.setOutlineColor(Color(0, 0, 0, alpha));
                
                Color textColor = fillColor;
                textColor.a = alpha;
                label.setFillColor(textColor);
                
                label.setPosition(portLocations[idx].x, portLocations[idx].y - 25.f);
                window.draw(label);
            };

            if (selectedPortIndex != -1) {
                drawLabel(selectedPortIndex, Color::Red, 18);
                
                RouteNode* route = graph.ports[selectedPortIndex].routeHead;
                while (route) {
                    int destIdx = -1;
                    for (int k = 0; k < numLocations; k++)
                        if (portLocations[k].name == route->destinationPort) destIdx = k;
                    if (destIdx != -1) drawLabel(destIdx, Color::Yellow, 16);
                    route = route->next;
                }
            }
            
            if (showSearchResult && currentPath.getSize() > 0) {
                for (int i = 0; i < currentPath.getSize(); i++) {
                    drawLabel(currentPath[i], Color::Cyan, 16);
                }
            }
        }

        bool searchHover = searchButton.getGlobalBounds().contains(mousePos) && !showInputWindow;
        
        float buttonPulse = sin(time * 3.0f) * 0.2f + 0.8f;
        buttonGlow.setRadius(40 * buttonPulse);
        buttonGlow.setOrigin(40 * buttonPulse, 40 * buttonPulse);
        buttonGlow.setPosition(30, 1010);
        
        if (searchHover) {
            searchButton.setFillColor(Color(0, 150, 255, 220));
            searchButton.setOutlineColor(Color(150, 230, 255));
            window.draw(buttonGlow);
        } else {
            searchButton.setFillColor(Color(0, 100, 200, 200));
            searchButton.setOutlineColor(Color(100, 200, 255));
        }
        
        window.draw(searchButton);
        window.draw(searchButtonText);

        if (showInputWindow) {
            RectangleShape overlay(Vector2f(1920, 1080));
            overlay.setFillColor(Color(0, 0, 0, 120));
            window.draw(overlay);
            
            window.draw(inputWindow);
            
            window.draw(windowTitle);
            
            originInput.draw(window);
            destinationInput.draw(window);
            dateInput.draw(window);
            
            bool popupSearchHover = popupSearchButton.getGlobalBounds().contains(mousePos);
            bool closeHover = closeButton.getGlobalBounds().contains(mousePos);
            
            if (popupSearchHover) {
                popupSearchButton.setFillColor(Color(0, 220, 150));
                popupSearchButton.setOutlineColor(Color(150, 255, 200));
                window.draw(searchButtonGlow);
            } else {
                popupSearchButton.setFillColor(Color(0, 180, 120));
                popupSearchButton.setOutlineColor(Color(100, 255, 180));
            }
            
            if (closeHover) {
                closeButton.setFillColor(Color(220, 100, 100));
                closeButton.setOutlineColor(Color(255, 180, 180));
                window.draw(closeButtonGlow);
            } else {
                closeButton.setFillColor(Color(180, 80, 80));
                closeButton.setOutlineColor(Color(255, 150, 150));
            }
            
            window.draw(popupSearchButton);
            window.draw(popupSearchText);
            window.draw(closeButton);
            window.draw(closeText);
            
            if (showSearchResult) {
                Text resultText;
                resultText.setFont(font);
                resultText.setString(searchResultText);
                resultText.setCharacterSize(18);
                resultText.setFillColor(searchResultColor);
                resultText.setStyle(Text::Bold);
                resultText.setOutlineThickness(1);
                resultText.setOutlineColor(Color(0, 0, 0, 150));
                resultText.setPosition(750, 750);
                window.draw(resultText);
            }
        }

        if (showSearchResult && !showInputWindow) {
            Text resultText;
            resultText.setFont(font);
            resultText.setString(searchResultText);
            resultText.setCharacterSize(20);
            resultText.setFillColor(searchResultColor);
            resultText.setStyle(Text::Bold);
            resultText.setOutlineThickness(1);
            resultText.setOutlineColor(Color(0, 0, 0, 150));
            resultText.setPosition(250, 990);
            window.draw(resultText);
            
            if (currentPath.getSize() > 0 && searchResultColor == Color::Green) {
                string pathStr = "Route: ";
                for (int i = 0; i < currentPath.getSize(); i++) {
                    pathStr += portLocations[currentPath[i]].name;
                    if (i < currentPath.getSize() - 1) pathStr += " → ";
                }
                
                Text pathText;
                pathText.setFont(font);
                pathText.setString(pathStr);
                pathText.setCharacterSize(16);
                pathText.setFillColor(Color::Cyan);
                pathText.setStyle(Text::Bold);
                pathText.setPosition(250, 1020);
                window.draw(pathText);
            }
        }

        if (hoveredRoute != nullptr && !showInputWindow) {
            float boxWidth = 400.f, boxHeight = 180.f;
            float boxX = mousePos.x + 20.f;
            float boxY = mousePos.y - 160.f;
            if (boxX + boxWidth > 1920) boxX = 1920 - boxWidth - 20;
            if (boxY < 0) boxY = 20;

            RectangleShape infoBox(Vector2f(boxWidth, boxHeight));
            infoBox.setFillColor(Color(0, 20, 60, 240));
            infoBox.setOutlineThickness(2.f);
            infoBox.setOutlineColor(Color(100, 200, 255));
            infoBox.setPosition(boxX, boxY);
            
            VertexArray gradient(Quads, 4);
            gradient[0].position = Vector2f(boxX, boxY);
            gradient[1].position = Vector2f(boxX + boxWidth, boxY);
            gradient[2].position = Vector2f(boxX + boxWidth, boxY + boxHeight);
            gradient[3].position = Vector2f(boxX, boxY + boxHeight);
            
            gradient[0].color = Color(0, 30, 80, 240);
            gradient[1].color = Color(0, 20, 60, 240);
            gradient[2].color = Color(0, 10, 40, 240);
            gradient[3].color = Color(0, 30, 80, 240);
            
            window.draw(gradient);
            window.draw(infoBox);

            Text details;
            details.setFont(font);
            details.setCharacterSize(16);
            details.setFillColor(Color(200, 230, 255));
            details.setStyle(Text::Bold);
            details.setPosition(boxX + 15.f, boxY + 15.f);

            string infoText = "From: " + hoveredRoute->startingPort + "\n" +
                              "To: " + hoveredRoute->destinationPort + "\n" +
                              "Cost: $" + to_string(hoveredRoute->cost) + "\n" +
                              "Date: " + hoveredRoute->departureDate + "\n" +
                              "Time: " + hoveredRoute->departureTime + " - " + hoveredRoute->arrivalTime + "\n" +
                              "Company: " + hoveredRoute->shippingCompany;

            details.setString(infoText);
            window.draw(details);
            
            static float boxPulse = 0.0f;
            boxPulse += 0.1f;
            float pulseValue = 0.5f + 0.5f * sin(boxPulse);
            infoBox.setOutlineColor(Color(100, 200, 255, 150 + 105 * pulseValue));
        }

        Text title;
        title.setFont(font);
        title.setString("OceanRoute Navigator");
        title.setCharacterSize(28);
        title.setFillColor(Color(200, 230, 255));
        title.setStyle(Text::Bold);
        title.setPosition(20, 20);
        window.draw(title);
        
        Text legend;
        legend.setFont(font);
        legend.setString("Route Cost:\nGreen: < $7000\nYellow: $7000-$10000\nRed: > $10000");
        legend.setCharacterSize(14);
        legend.setFillColor(Color(180, 220, 255));
        legend.setPosition(20, 900);
        window.draw(legend);

        window.display();
    }
}
};

Maps readPorts(string fileName)
{
    Maps map;

    ifstream file(fileName);
    if (!file.is_open())
    {
        cout << "Error opening file: " << fileName << endl;
        return map;
    }

    string port;
    int charge;

    while (file >> port >> charge)
    {
        map.addPort(port, charge);
    }

    file.close();
    return map;
}

void addRoutesFromFile(Maps &graph, string fileName)
{
    ifstream file(fileName);
    if (!file.is_open())
    {
        cout << "Error opening " << fileName << "!" << endl;
        return;
    }

    string start, dest, depDate, depTime, arrTime, company;
    int cost;

    while (file >> start >> dest >> depDate >> depTime >> arrTime >> cost >> company)
    {
        graph.addRoute(start, dest, depDate, depTime, arrTime, cost, company);
    }

    file.close();
}

int main()
{
    Maps graph = readPorts("PortCharges.txt");
    addRoutesFromFile(graph, "Routes.txt");
    graph.showMap(graph);
   
    return 0;
}