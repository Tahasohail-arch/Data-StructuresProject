#ifndef VECTOR_H
#define VECTOR_H

#include <iostream>
using namespace std;
template <class V>
class Vector
{
private:
    V *arr;
    int size;
    int cap;

public:
    Vector()
    {
        size = 0;
        cap = 40;
        arr = new V[cap];
    }

    Vector(int initialSize)
    {
        size = initialSize;
        cap = initialSize * 2;
        if (cap < 40) cap = 40;
        arr = new V[cap];
        for (int i = 0; i < size; i++)
        {
            arr[i] = V();
        }
    }

    Vector(const Vector<V> &other)
    {
        size = other.size;
        cap = other.cap;
        arr = new V[cap];
        for (int i = 0; i < size; i++)
        {
            arr[i] = other.arr[i];
        }
    }

    Vector<V>& operator=(const Vector<V> &other)
    {
        if (this == &other) return *this;
        delete[] arr;
        size = other.size;
        cap = other.cap;
        arr = new V[cap];
        for (int i = 0; i < size; i++)
        {
            arr[i] = other.arr[i];
        }
        return *this;
    }

    void resize()
    {
        cap *= 2;
        V *newArr = new V[cap];
        for (int i = 0; i < size; i++)
        {
            newArr[i] = arr[i];
        }
        delete[] arr;
        arr = newArr;
    }

    void push_back(const V &val)
    {
        if (size >= cap)
        {
            resize();
        }
        arr[size] = val;
        size++;
    }
    void pop(){
        if (size >0)
        size--;
    }

    int getSize() const
    {
        return size;
    }

    V& operator[](int index)
    {
        if (index < 0 || index >= size)
        {
            throw std::out_of_range("Index out of bounds");
        }
        return arr[index];
    }

    const V& operator[](int index) const
    {
        if (index < 0 || index >= size)
        {
            throw std::out_of_range("Index out of bounds");
        }
        return arr[index];
    }

    void clear()
    {
        size = 0;
    }

    ~Vector()
    {
        delete[] arr;
    }
};

#endif
