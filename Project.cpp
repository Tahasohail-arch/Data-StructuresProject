#include <iostream>
#include <string>
#include "Queue.h"
using namespace std;

int main()
{
    Queue q;

    q.enqueue("Karachi");
    q.enqueue("Dubai");

    QueueNode n = q.dequeue();
    cout << n.portName << endl; // Karachi
}
