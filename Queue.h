#pragma once
#include <iostream>
#include <string>
using namespace std;

struct QueueNode
{
    string portName;
    QueueNode *next;

    QueueNode(string name)
    {
        portName = name;
        next = nullptr;
    }
};

class Queue
{
private:
    QueueNode *rear;
    QueueNode *front;
    int count;

public:
    Queue()
    {
        rear = nullptr;
        front = nullptr;
    }

    bool isEmpty()
    {
        return front == nullptr;
    }

    //* Add at Front
    void enqueue(string name)
    {
        QueueNode *newNode = new QueueNode(name);

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

    QueueNode dequeue()
    {
        if (isEmpty())
        {
            cout << "Queue Underflow! We Cannot dequeue. " << endl;
            return QueueNode("");
        }

        QueueNode *temp = front;
        QueueNode returnValue = *temp;

        front = front->next;

        if (front == nullptr)
            rear = nullptr;

        delete temp;
        return returnValue;
    }

    QueueNode frontfind()
    {
        if (isEmpty())
        {
            cout << "Queue is empty! " << endl;
            return QueueNode("");
        }
        return *front;
    }

    ~Queue()
    {
        while (!isEmpty())
        {
            dequeue();
        }
    }
};