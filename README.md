# 🚢 OceanRoute Navigator

A comprehensive maritime route planning and cargo booking simulation system built with C++ and SFML. This application provides an interactive visualization of global shipping routes, pathfinding algorithms, port docking management, and booking systems.

![C++](https://img.shields.io/badge/C++-17-blue.svg)
![SFML](https://img.shields.io/badge/SFML-2.5+-green.svg)

## 📋 Table of Contents

- [Features](#features)
- [Project Structure](#project-structure)
- [Data Structures Used](#data-structures-used)
- [Algorithms Implemented](#algorithms-implemented)
- [Installation](#installation)
- [Usage](#usage)
- [File Descriptions](#file-descriptions)
- [Dependencies](#dependencies)
- [Contributors](#contributors)

## ✨ Features

### 🗺️ Interactive World Map
- Visual representation of 40 global ports
- Real-time route visualization with animations
- Zoom and pan functionality
- Port information on hover

### 🔍 Pathfinding Algorithms
- **Dijkstra's Algorithm**: Find optimal routes based on cost or time
- **A* Algorithm**: Intelligent heuristic-based pathfinding
- Support for ship preferences (preferred companies, avoided ports, max voyage time)

### 📦 Cargo Booking System
- Complete booking workflow with customer details
- Route selection (cheapest vs fastest)
- Cost calculation including:
  - Base route costs
  - Port charges
  - Docking wait charges
- Booking history with persistent storage

### ⚓ Port Docking Management
- Real-time docking simulation
- Queue management for ships waiting to dock
- Visual representation of dock occupancy
- Estimated wait time calculations

### 🔗 Multi-Leg Route Builder
- Custom route creation using linked list
- Route validation
- Interactive port selection on map
- Visual linked list representation

### ⚙️ Ship Preferences
- Filter routes by shipping companies
- Avoid specific ports
- Set maximum voyage duration
- Preferences applied to pathfinding algorithms

## 📁 Project Structure

```
OceanRoute-Navigator/
├── main.cpp                 # Main application entry point
├── AStar.h                  # A* pathfinding algorithm
├── Dijkstra.h               # Dijkstra's algorithm implementation
├── BookingInterface.h       # Cargo booking UI and logic
├── ViewBookings.h           # Booking history viewer
├── MainMenu.h               # Main menu interface
├── LinkedListRoute.h        # Multi-leg route linked list
├── ShipPreferences.h        # Ship preference management
├── PortDockingManager.h     # Port docking simulation
├── DockingStructures.h      # Docking data structures
├── RouteStructures.h        # Route and port structures
├── InputBoxUI.h             # Input box UI component
├── GlobalVariables.h        # Global state management
├── Vector.h                 # Custom vector implementation
├── Queue.h                  # Queue data structure
├── priorityqueue.h          # Priority queue (min-heap)
├── ship.h                   # Ship-related structures
├── booking.h                # Booking data structure
├── dockingqueue.h           # Docking queue management
├── Routes.txt               # Route database
├── PortCharges.txt          # Port charges database
├── bookings.txt             # Persistent booking storage
└── pics/                    # Image assets
    └── map2.png             # World map image
```

## 🏗️ Data Structures Used

| Data Structure | File | Purpose |
|---------------|------|---------|
| **Custom Vector** | `Vector.h` | Dynamic array for storing ports, routes, paths |
| **Linked List** | `LinkedListRoute.h` | Multi-leg custom route building |
| **Priority Queue (Min-Heap)** | `priorityqueue.h` | Dijkstra's and A* algorithm optimization |
| **Queue** | `Queue.h` | BFS traversal, docking queue management |
| **Adjacency List** | `RouteStructures.h` | Graph representation of port connections |

### Custom Vector Implementation
```cpp
template <typename V>
class Vector {
    V *arr;
    int size, cap;
public:
    void push_back(const V &val);
    void pop();
    V& operator[](int index);
    int getSize() const;
    // ...
};
```

### Priority Queue (Min-Heap)
```cpp
class PriorityQueue {
    struct Node {
        int index;
        float cost;
    };
    Vector<Node> heap;
    void heapifyUp(int childIndex);
    void heapifyDown(int parentIndex);
    void enqueue(int index, float cost);
    Node front();
    void dequeue();
};
```

## 🧮 Algorithms Implemented

### Dijkstra's Algorithm
- **Purpose**: Find shortest/cheapest path between ports
- **Complexity**: O((V + E) log V) with priority queue
- **Features**:
  - Date-aware scheduling
  - Wait time calculation at ports
  - Ship preference filtering

### A* Algorithm
- **Purpose**: Heuristic-guided pathfinding
- **Heuristic**: Euclidean distance between port coordinates
- **Features**:
  - Faster convergence than Dijkstra for distant destinations
  - Same preference filtering support

### BFS (Breadth-First Search)
- **Purpose**: Finding connecting routes
- **Used in**: Route validation, multi-leg route checking

## 🛠️ Installation

### Prerequisites
- C++ compiler with C++17 support
- SFML 2.5 or later
- Make (optional)

### Linux (Ubuntu/Debian)
```bash
# Install SFML
sudo apt-get update
sudo apt-get install libsfml-dev

# Clone the repository
git clone https://github.com/Tahasohail-arch/Data-StructuresProject.git
cd Data-StructuresProject

# Compile
g++ -std=c++17 main.cpp -o oceanroute -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio

# Run
./oceanroute
```

### Windows (with MinGW)
```bash
g++ -std=c++17 main.cpp -o oceanroute.exe -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio
```

### macOS
```bash
# Install SFML via Homebrew
brew install sfml

# Compile
g++ -std=c++17 main.cpp -o oceanroute -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio

# Run
./oceanroute
```

## 🎮 Usage

### Main Menu Options
1. **View Map** - Interactive world map with port visualization
2. **Book Cargo** - Create new cargo bookings
3. **View Bookings** - See booking history
4. **Multi-Leg Route** - Build custom multi-stop routes
5. **Exit** - Close application

### Keyboard Shortcuts
| Key | Action |
|-----|--------|
| `ESC` | Return to previous menu / Exit |
| `C` | Clear route (in Multi-Leg Builder) |
| `Mouse Wheel` | Scroll lists / Zoom map |
| `Left Click` | Select / Confirm |
| `Right Click` | Remove (in Multi-Leg Builder) |

### Booking a Cargo Route
1. Enter customer name
2. Enter departure port
3. Enter destination port
4. Enter departure date (DD/MM/YYYY)
5. Choose route type (Cheapest / Fastest)
6. Optionally set ship preferences
7. Confirm booking

## 📄 File Descriptions

### Data Files

| File | Format | Description |
|------|--------|-------------|
| `Routes.txt` | `StartPort DestPort Date DepTime ArrTime Cost Company` | All available shipping routes |
| `PortCharges.txt` | `PortName Charge` | Docking charges per port |
| `bookings.txt` | `ID\|Name\|Origin\|Dest\|Date\|Cost` | Saved bookings |

### Sample Route Entry
```
HongKong Jeddah 22/12/2024 09:00 03:00 5112 Evergreen
```

### Sample Port Charge Entry
```
Singapore 946
Dubai 1000
```

## 📦 Dependencies

- **SFML 2.5+** - Graphics, Window, Audio, System modules
- **C++17** - Standard library features
- **Roboto.ttf** - Font file (included)
- **Audio files** - click.wav, transition.wav, ship.wav

## 🌐 Ports Included

The system includes 40 major global ports:
```
AbuDhabi, Alexandria, Antwerp, Athens, Busan, CapeTown, 
Chittagong, Colombo, Copenhagen, Doha, Dubai, Dublin, 
Durban, Genoa, Hamburg, Helsinki, HongKong, Istanbul, 
Jakarta, Jeddah, Karachi, Lisbon, London, LosAngeles, 
Manila, Marseille, Melbourne, Montreal, Mumbai, NewYork, 
Osaka, Oslo, PortLouis, Rotterdam, Shanghai, Singapore, 
Stockholm, Sydney, Tokyo, Vancouver
```

## 🎨 Theme

The application features a **"Stranger Things" inspired** dark theme with:
- Deep red and maroon accents
- Flickering light effects
- Atmospheric particle systems
- Retro-futuristic UI elements

## 🤝 Contributors

- **Taha Sohail** - Developer
- **Muhammad Ali** - Developer

## 🙏 Acknowledgments

- SFML development team for the multimedia library
- Shipping route data inspired by real-world maritime routes
- UI design inspired by Netflix's "Stranger Things"

---

**Note**: This project was developed as a Data Structures course project demonstrating practical applications of graphs, linked lists, priority queues, and pathfinding algorithms.
