#ifndef DOCKINGQUEUE_H
#define DOCKINGQUEUE_H

#include <iostream>
#include <string>
#include "ship.h"  
using namespace std;

class DockingQueue {
public:
    Ship *front;
    Ship *rear;
    
    DockingQueue() {
        front = nullptr;
        rear = nullptr;
    }
    
    bool isEmpty() {
        return front == nullptr;
    }
    
    void enqueue(Ship* newship) {
        newship->next = nullptr;
        if (isEmpty()) {
            front = newship;
            rear = newship;
        } else {
            rear->next = newship;
            rear = newship;
        }
    }
    
    Ship* dequeue() {
        if (isEmpty()) {
            return nullptr;
        }
        Ship *temp = front;
        front = front->next;
        if (front == nullptr)
            rear = nullptr;
        return temp;   
    }
    
    int getSize() {
        int count = 0;
        Ship* current = front;
        while (current) {
            count++;
            current = current->next;
        }
        return count;
    }
    
    Ship* peekFront() {
        return front;
    }
    
    ~DockingQueue() {
        while (!isEmpty()) {
            Ship *temp = dequeue();
            delete temp;
        }
    }
};

#endif