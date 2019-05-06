#include <stdlib.h>
#include <math.h>
#include <iostream>
#include <queue>
#include <limits.h>

#include "Event.h"

using namespace std;

// Global variables
int defaultSize = 1000;
// - Queues
queue<Event*> observerQueue;
queue<Event*> arrivalQueue;
queue<Event*> departureQueue;
// - Counters
double observationCounter;
double arrivalCounter;
double departureCounter;
double idleCounter;
double numberOfPacketsInQueue;
double droppedPacketsCounter;

double getExponentialRandomVariable(double param) {
	// Get random value between 0 (exclusive) and 1 (inclusive)
	double uniformRandomValue = ((double) rand() + 1) / ((double) (RAND_MAX) + 1);
	double exponentialRandomValue = (double)(-(log(1 - uniformRandomValue)) / param);
	
	return exponentialRandomValue;
}

void generateObserverAndArrivalEvents(double simulationTime, double *observerTimes, double *arrivalTimes, double alpha, double lambda) {
	double observerTimeCounter = 0;
	double arrivalTimeCounter = 0;
	int j = 0;
	while (observerTimeCounter < simulationTime || arrivalTimeCounter < simulationTime) {		
		if (j == defaultSize) {
			j = 0;
			// Reset observerTimes and arrivalTimes random value arrays
			for (int i = 0; i < defaultSize; i++) {
				observerTimes[i] = getExponentialRandomVariable(alpha);
				arrivalTimes[i] = getExponentialRandomVariable(lambda);
			}
		}
		
		// Update observer and arrival queues
		if (observerTimeCounter < simulationTime) {
			observerTimeCounter += observerTimes[j];
			observerQueue.push(new Event(OBSERVER, observerTimeCounter));
		}
		
		if (arrivalTimeCounter < simulationTime) {
			arrivalTimeCounter += arrivalTimes[j];
			arrivalQueue.push(new Event(ARRIVAL, arrivalTimeCounter));
		}
		
		j++;
	}
}

Event * getNextEvent() {
	// Check the top of each queue and return the smallest time (next event) of the three
	bool observerEventsExist = observerQueue.size() > 0;
	bool arrivalEventsExist = arrivalQueue.size() > 0;
	bool departureEventsExist = departureQueue.size() > 0;
	
	if (observerEventsExist && arrivalEventsExist && departureEventsExist) {
		Event *observerEvent = observerQueue.front();
		Event *arrivalEvent = arrivalQueue.front();
		Event *departureEvent = departureQueue.front();
		
		Event *nextEvent = arrivalEvent;
		if (observerEvent->getTime() < arrivalEvent->getTime()) {
			nextEvent = observerEvent;
		}
		
		return nextEvent->getTime() < departureEvent->getTime() ? nextEvent : departureEvent;
	} else if (observerEventsExist && arrivalEventsExist) {
		Event *observerEvent = observerQueue.front();
		Event *arrivalEvent = arrivalQueue.front();
		return observerEvent->getTime() < arrivalEvent->getTime() ? observerEvent : arrivalEvent;		
	} else if (observerEventsExist && departureEventsExist) {
		Event *observerEvent = observerQueue.front();
		Event *departureEvent = departureQueue.front();
		return observerEvent->getTime() < departureEvent->getTime() ? observerEvent : departureEvent;		
	} else if (arrivalEventsExist && departureEventsExist) {
		Event *arrivalEvent = arrivalQueue.front();
		Event *departureEvent = departureQueue.front();
		return arrivalEvent->getTime() < departureEvent->getTime() ? arrivalEvent : departureEvent;
	} else if (observerEventsExist) {
		return observerQueue.front();
	} else if (arrivalEventsExist) {
		return arrivalQueue.front();
	} else if (departureEventsExist) {
		return departureQueue.front();
	}
}

void handleObserverEvent() {
	observationCounter++;
	observerQueue.pop();
	
	bool isQueueEmpty = departureQueue.size() == 0;
	if (isQueueEmpty){
		// Empty buffer, increment idle counter
		idleCounter++;
	} else {
		// Non-empty buffer, update queue packet counter
		numberOfPacketsInQueue += departureQueue.size();
	}
}

void handleArrivalEvent(Event *event, double L, double C, int queueSize) {
	arrivalCounter++;
	arrivalQueue.pop();
	
	if (departureQueue.size() < queueSize) {
		// Create departure event
		double departureTime = getExponentialRandomVariable(1/L)/C; // Service time
		// Get base departure time to add service time to
		if (departureQueue.size() == 0) {
			departureTime += event->getTime();
		} else {
			departureTime += departureQueue.back()->getTime();
		}
		departureQueue.push(new Event(DEPARTURE, departureTime));
	} else {
		// Full buffer, drop packet
		droppedPacketsCounter++;
	}
}

void handleDepartureEvent() {
	departureCounter++;
	departureQueue.pop();
}

void simulate (double simulationTime, double alpha, double lambda, double L, double C, int queueSize) {
	// Reset global counters
	observationCounter = 0;
	arrivalCounter = 0;
	departureCounter = 0;
	idleCounter = 0;
	numberOfPacketsInQueue = 0;
	droppedPacketsCounter = 0;
	
	// Set random time arrays
	double observerTimes[defaultSize];
	double arrivalTimes[defaultSize];
	
	for (int i = 0; i < defaultSize; i++) {
		observerTimes[i] = getExponentialRandomVariable(alpha);
		arrivalTimes[i] = getExponentialRandomVariable(lambda);
	}
	
	// Generate observer and arrival events
	generateObserverAndArrivalEvents(simulationTime, observerTimes, arrivalTimes, alpha, lambda);
	
	// Handle events in the queues
	while (observerQueue.size() > 0 || arrivalQueue.size() > 0 || departureQueue.size() > 0) {
		Event *event = getNextEvent();
		if (event->getType() == OBSERVER) {
			handleObserverEvent();
		} else if (event->getType() == ARRIVAL) {
			handleArrivalEvent(event, L, C, queueSize);
		} else {
			handleDepartureEvent();
		}
	}
}

void simulateMM1Queue() {
	cout << "\nMM1 Queue:" << "\n";
	double simulationTime = 2000;
	double alpha = 0;
	double lambda = 0;
	double L = 2000;
	double C = 1000000;
	int queueSize = INT_MAX; // Represents a M/M/1 queue (infinite queue size)
	
	// Display the effects of traffic intensity on the average number of 
	// packets in the system and the proportion of system idle time
	for (double rho = 0.25; rho <= 0.95; rho += 0.1) {
		lambda = (rho * C)/L;
		alpha = 5 * lambda;
		simulate(simulationTime, alpha, lambda, L, C, queueSize);
		
		cout << "Traffic Intensity = " << rho << "\n";
		cout << "Average Number of Packets in System = " << numberOfPacketsInQueue/observationCounter << "\n";
		cout << "Proportion of system idle time = " << idleCounter/observationCounter << "\n\n";
	}
}

void simulateMM1KQueue() {
	cout << "\nMM1K Queue:" << "\n";
	double alpha = 0;
	double lambda = 0;
	double simulationTime = 2000;
	double L = 2000;
	double C = 1000000;
	
	// Display the effects of traffic intensity on the average number of 
	// packets in the system and the probability of dropping a packet,
	// with queue sizes of 10, 25, and 50
	for (double rho = 0.5; rho <= 1.6; rho += 0.1) {
		lambda = (rho * C)/L;
		alpha = 5 * lambda;
		
		cout << "Traffic Intensity = " << rho << "\n";
		simulate(simulationTime, alpha, lambda, L, C, 10);
		
		cout << "Queue Size = " << 10 << "\n";
		cout << "Average Number of Packets in System = " << numberOfPacketsInQueue/observationCounter << "\n";
		cout << "Probability of Dropping a Packet = " << droppedPacketsCounter/arrivalCounter << "\n\n";
		
		simulate(simulationTime, alpha, lambda, L, C, 25);
		
		cout << "Queue Size = " << 25 << "\n";
		cout << "Average Number of Packets in System = " << numberOfPacketsInQueue/observationCounter << "\n";
		cout << "Probability of Dropping a Packet  = " << droppedPacketsCounter/arrivalCounter << "\n\n";
		
		simulate(simulationTime, alpha, lambda, L, C, 50);
		
		cout << "Queue Size = " << 50 << "\n";
		cout << "Average Number of Packets in System = " << numberOfPacketsInQueue/observationCounter << "\n";
		cout << "Probability of Dropping a Packet  = " << droppedPacketsCounter/arrivalCounter << "\n\n";
	}
}

int main() {
	simulateMM1Queue();
	simulateMM1KQueue();
	return 0;
}