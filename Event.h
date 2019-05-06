#ifndef EVENT_H
#define EVENT_H

enum Type{
	OBSERVER,
	ARRIVAL,
	DEPARTURE
};

class Event{
	Type type;
	double time;
public:
	Event(Type, double);
	double getTime() const;
	Type getType() const;
};

#endif
