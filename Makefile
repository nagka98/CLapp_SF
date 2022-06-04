
CC = g++
CFLAGS = -Wall -g
 
# ****************************************************
# Targets needed to bring the executable up to date
 
main: main.o SerialPort.o
	$(CC) $(CFLAGS) -o main main.o SerialPort.o
 
# The main.o target can be written more simply
 
main.o: main.cpp SerialPort.h
	$(CC) $(CFLAGS) -c main.cpp
 
SerialPort.o: SerialPort.cpp SerialPort.h
	$(CC) $(CFLAGS) -c SerialPort.cpp

clean:
	rm *.o main

