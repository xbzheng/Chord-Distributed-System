#auther Xiaobin Zheng, xzheng19, 668374748
EXECS=chord_sim
CFLAGS=-g -Wall -Wextra -Wno-unused-parameter -lpthread
#CFLAGS=-lpthread
CC=clang++
#CC=gcc

all: $(EXECS)

chord_sim: chord_sim.o
	$(CC) $(CFLAGS) chord_sim.o -o chord_sim

chord_sim.o: chord_sim.cpp chord_sim.h
	$(CC) $(CFLAGS) -c chord_sim.cpp -o chord_sim.o

%.o: %.c
	$(CC) -c $<

clean:
	rm -f *.o $(EXECS) *~