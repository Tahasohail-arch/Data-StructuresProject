#ifndef LINKED_LIST_ROUTE_H
#define LINKED_LIST_ROUTE_H
#include <iostream>
#include <string>
#include "Vector.h"
using namespace std;

struct LegPair {
	string first;
	string second;
	LegPair() { 
		first = ""; second = ""; 
	}
	LegPair(const string &a, const string &b) { 
		first = a; 
		second = b; 
	}
};

struct CustomRouteNode {
	string portName;
	string startPort;
	string endPort;
	string departureDate;
	string departureTime;
	string company;
	int distance;
	double cost;

	CustomRouteNode* next;

	CustomRouteNode(string name){
		portName = name;
		startPort = name;
		endPort = "";
		departureDate = "";
		departureTime = "";
		company = "";
		distance = 0;
		cost = 0.0;
		next = nullptr;
	}
};

class LinkedListRoute {
public:
	CustomRouteNode* head;
	CustomRouteNode* tail;

	LinkedListRoute() {
		head = nullptr;
		tail = nullptr;
	}

	~LinkedListRoute() {
		clear();
	}

	void append(string portName) {
		CustomRouteNode* n = new CustomRouteNode(portName);
		if (!head) {
			head = tail = n;
		} else {
			tail->next = n;
			tail->endPort = n->portName;
			n->startPort = n->portName;
			tail = n;
		}
		print();
	}

	bool insertAfter(string afterPort, string newPort) {
		CustomRouteNode* cur = head;
		while (cur) {
			if (cur->portName == afterPort) {
				CustomRouteNode* n = new CustomRouteNode(newPort);
				n->startPort = n->portName;
				if (cur->next) {
					n->endPort = cur->next->portName;
				} else {
					n->endPort = "";
				}
				n->next = cur->next;
				cur->next = n;
				cur->endPort = newPort;
				if (cur == tail) tail = n;
				print();
				return true;
			}
			cur = cur->next;
		}
		return false;
	}

	bool remove( string portName) {
		CustomRouteNode* cur = head;
		CustomRouteNode* prev = nullptr;
		while (cur) {
			if (cur->portName == portName) {
				if (prev) {
					prev->next = cur->next;
					if (cur->next) {
						prev->endPort = cur->next->portName;
					} else {
						prev->endPort = "";
					}
				} else {
					head = cur->next;
					if (head) {
						head->startPort = head->portName;
					}
				}
				if (cur == tail) tail = prev;
				delete cur;
				print();
				return true;
			}
			prev = cur;
			cur = cur->next;
		}
		return false;
	}

	void clear() {
		CustomRouteNode* cur = head;
		while (cur) {
			CustomRouteNode* nx = cur->next;
			delete cur;
			cur = nx;
		}
		head = tail = nullptr;
		cout << "Journey cleared" << endl;
	}

	
	class Maps;
	bool hasInvalidLeg(Maps &graph, string &badA, string &badB);

	Vector<LegPair> getLegPairs() const {
		Vector<LegPair> legs;
		CustomRouteNode* cur = head;
		while (cur) {
			if (!cur->endPort.empty()) {
				legs.push_back(LegPair(cur->startPort, cur->endPort));
			}
			cur = cur->next;
		}
		return legs;
	}

	Vector<string> getPortSequence() const {
		Vector<string> ports;
		CustomRouteNode* cur = head;
		while (cur) {
			ports.push_back(cur->portName);
			cur = cur->next;
		}
		return ports;
	}

	void print() {
		cout << "Journey: ";
		CustomRouteNode* cur = head;
		while (cur) {
			cout << cur->portName;
			if (cur->next) cout << " -> ";
			cur = cur->next;
		}
		cout << endl;
		cur = head;
		bool printedAnyLeg = false;
		while (cur) {
			if (!cur->endPort.empty()) {
				if (!printedAnyLeg) { cout << "Legs: "; printedAnyLeg = true; }
				cout << "[" << cur->startPort << " -> " << cur->endPort << "]";
				if (cur->next && cur->next->endPort.size()) cout << " -> ";
			}
			cur = cur->next;
		}
		if (printedAnyLeg) cout << endl;
	}
};



#endif
