all: run;

run: DES
	./DES
	
DES: DES.o Event.o
	g++ -o DES DES.o Event.o
	
DES.o: DES.cpp Event.h
	g++ -c DES.cpp
	
Event.o: Event.h

clean:
	rm -f DES.exe DES.o Event.o