#ifndef PRIORITY_QUEUE_H
#define PRIORITY_QUEUE_H

#include "Vector.h"   
#include <stdexcept>

class PriorityQueue {
public:
    struct Node {
        int index;      
        float cost;    
    };

    Vector<Node> heap;  
    
    void heapifyUp(int childIndex) {
        while (childIndex > 0) {
            int parentIndex = (childIndex - 1) / 2;
            if (heap[childIndex].cost < heap[parentIndex].cost) {
                swap(heap[childIndex], heap[parentIndex]);
                childIndex = parentIndex;
            } else {
                break;
            }
        }
    }
    
    void heapifyDown(int parentIndex) {
        int heapSize = heap.getSize();
        
        while (true) {
            int leftChild = 2 * parentIndex + 1;
            int rightChild = 2 * parentIndex + 2;
            int smallest = parentIndex;

            // Check left child exists and has smaller cost
            if (leftChild < heapSize && heap[leftChild].cost < heap[smallest].cost) {
                smallest = leftChild;
            }
            
            // Check right child exists and has smaller cost
            if (rightChild < heapSize && heap[rightChild].cost < heap[smallest].cost) {
                smallest = rightChild;
            }
            
            if (smallest != parentIndex) {
                swap(heap[parentIndex], heap[smallest]);
                parentIndex = smallest;
            } else {
                break;
            }
        }
    }

    void enqueue(int index, float cost) {
        Node newnode;
        newnode.index = index;
        newnode.cost = cost;
        heap.push_back(newnode);
        int childindex = heap.getSize() - 1;
        heapifyUp(childindex);
    }
    
    Node front() {
        if (heap.getSize() == 0) {
            throw std::out_of_range("PriorityQueue is empty");
        }
        return heap[0];
    }
    
    void dequeue() {
        if (heap.getSize() == 0) {
            throw std::out_of_range("PriorityQueue is empty");
        }
        
        if (heap.getSize() == 1) {
            heap.pop();
            return;
        }
        
        swap(heap[0], heap[heap.getSize() - 1]);
        heap.pop();
        heapifyDown(0);
    }
    
    bool isEmpty() {
        return heap.getSize() == 0;
    }
    
    int getSize() {
        return heap.getSize();
    }

    void swap(Node& a, Node& b) {
        Node temp = a;
        a = b;
        b = temp;
    }
};

#endif