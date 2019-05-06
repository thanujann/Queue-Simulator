#include "Event.h"

using namespace std;

Event::Event(Type type, double time) {
	this->type = type;
	this->time = time;
}

double Event::getTime() const { 
	return this->time;
}

Type Event::getType() const {
	return this->type;
} 