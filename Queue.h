#ifndef QUEUE_H
#define QUEUE_H

#include <iostream>
#include "Vector.h" 
using namespace std;

struct QueueNode
{
    Vector<int> path; // har node me current path store hoga
    QueueNode *next;

    QueueNode(Vector<int> p)
    {
        path = p;
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

    void enqueue(Vector<int> p)
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

    Vector<int> dequeue()
    {
        if (isEmpty())
        {
            cout << "Queue Underflow! Cannot dequeue." << endl;
            return Vector<int>(); // empty vector
        }

        QueueNode *temp = front;
        Vector<int> returnValue = temp->path;

        front = front->next;
        if (front == nullptr)
            rear = nullptr;

        delete temp;
        count--;
        return returnValue;
    }

    Vector<int> frontfind()
    {
        if (isEmpty())
        {
            cout << "Queue is empty!" << endl;
            return Vector<int>(); // empty vector
        }
        return front->path;
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
