#ifndef BOOKING_H
#define BOOKING_H

#include <iostream>
#include <string>
#include <fstream>
#include <ctime>
#include <cstdlib>
#include <cmath>
#include <SFML/Graphics.hpp>
#include "Vector.h"
#include "Queue.h"

using namespace std;
using namespace sf;

class Maps;
struct PortLocation;
extern PortLocation portLocations[];

struct Booking
{
    string bookingID;
    string customerName;
    string origin;
    string destination;
    string departureDate;
    int totalCost;
    Vector<string> routePath;
    Vector<string> waitPorts;
    Vector<string> waitDurations;
    Booking *next;

    Booking()
    {
        next = nullptr;
    }
};

class BookingManager
{
public:
    Booking *head;

    BookingManager()
    {
        head = nullptr;
    }

    ~BookingManager()
    {
        Booking *current = head;
        while (current != nullptr)
        {
            Booking *next = current->next;
            delete current;
            current = next;
        }
        head = nullptr;
    }

    string GenerateBookingID()
    {
        srand(time(0));
        string id = "ON";
        for (int i = 0; i < 6; i++)
        {
            id += to_string(rand() % 10);
        }

        return id;
    }

    void AddBooking(Booking *newBooking)
    {
        if (head == nullptr)
        {
            head = newBooking;
        }
        else
        {
            Booking *temp = head;
            while (temp->next != nullptr)
            {
                temp = temp->next;
            }
            temp->next = newBooking;
        }
        SaveToFile();
    }

    void SaveToFile()
    {
        ofstream file("bookings.txt", ios::out);

        if (!file.is_open())
        {
            cout << "Error opening bookings.txt for writing." << endl;
            return;
        }

        Booking *current = head;

        while (current != nullptr)
        {
            file << current->bookingID << "|"
                 << current->customerName << "|"
                 << current->origin << "|"
                 << current->destination << "|"
                 << current->departureDate << "|"
                 << current->totalCost << endl;
            current = current->next;
        }
        file.close();
    }

    void loadFromFile()
    {
        ifstream file("bookings.txt");
        if (!file.is_open())
            return;

        string line;
        while (getline(file, line))
        {
            Booking *b = new Booking();
            size_t pos = 0;

            pos = line.find('|');
            b->bookingID = line.substr(0, pos);
            line = line.substr(pos + 1);

            pos = line.find('|');
            b->customerName = line.substr(0, pos);
            line = line.substr(pos + 1);

            pos = line.find('|');
            b->origin = line.substr(0, pos);
            line = line.substr(pos + 1);

            pos = line.find('|');
            b->destination = line.substr(0, pos);
            line = line.substr(pos + 1);

            pos = line.find('|');
            b->departureDate = line.substr(0, pos);
            line = line.substr(pos + 1);

            b->totalCost = stoi(line);

            b->next = head;
            head = b;
        }
        file.close();
    }
};

extern BookingManager bookingManager;

void bookingInterface(Maps& graph, Font& font);

#endif