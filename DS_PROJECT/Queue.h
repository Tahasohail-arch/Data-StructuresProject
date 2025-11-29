#ifndef QUEUE_H
#define QUEUE_H

#include <iostream>
#include "Vector.h" 
using namespace std;

struct PathWithDate {
    Vector<int> path;
    string currentDate; 
    string currentTime; 
    Vector<string> waitPorts;   
    Vector<string> waitDurations; 
};

struct QueueNode
{
    PathWithDate data; 
    QueueNode *next;

    QueueNode(PathWithDate p)
    {
        data = p;
        next = nullptr;
    }
};

class Queue
{
public:
    QueueNode *rear;
    QueueNode *front;
    int count;

    Queue()
    {
        rear = nullptr;
        front = nullptr;
        count = 0;
    }

    bool isEmpty()
    {
        return front == nullptr;
    }

    void enqueue(PathWithDate p)
    {
        QueueNode *newNode = new QueueNode(p);

        if (isEmpty())
        {
            front = rear = newNode;
        }
        else
        {
            rear->next = newNode;
            rear = newNode;
        }
        count++;
    }

    PathWithDate dequeue()
    {
        if (isEmpty())
        {
            cout << "Queue Underflow! Cannot dequeue." << endl;
            PathWithDate empty;
            empty.path = Vector<int>();
            empty.currentDate = "";
            empty.currentTime = "";
            empty.waitPorts = Vector<string>();
            empty.waitDurations = Vector<string>();
            return empty;
        }

        QueueNode *temp = front;
        PathWithDate returnValue = temp->data;

        front = front->next;
        if (front == nullptr)
            rear = nullptr;

        delete temp;
        count--;
        return returnValue;
    }

    PathWithDate frontfind()
    {
        if (isEmpty())
        {
            cout << "Queue is empty!" << endl;
            PathWithDate empty;
            empty.path = Vector<int>();
            empty.currentDate = "";
            empty.currentTime = "";
            empty.waitPorts = Vector<string>();
            empty.waitDurations = Vector<string>();
            return empty;
        }
        return front->data;
    }

    ~Queue()
    {
        while (!isEmpty())
        {
            dequeue();
        }
    }
};

#endif