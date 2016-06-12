SHELL := /bin/bash
CC=mpicc #gcc
CFLAGS=-lmpi -lcrypto -lm
RUN=mpirun -np 4

.PHONY: all

all: build test

obj/main.o: src/main.c src/master.h src/worker.h src/range.h src/result_buffer.h src/tags.h
	mkdir -p obj
	${CC} -c src/main.c -o obj/main.o ${CFLAGS}
	
obj/hashmap.o: c_hashmap-master/hashmap.c c_hashmap-master/hashmap.h
	mkdir -p obj
	${CC} -c c_hashmap-master/hashmap.c -o obj/hashmap.o ${CFLAGS}
	
obj/test.o: test/test.c test/test_range.h test/test_result_buffer.h
	mkdir -p obj
	${CC} -c test/test.c -o obj/test.o

build: obj/main.o obj/hashmap.o
	mkdir -p bin
	${CC} obj/main.o obj/hashmap.o -o bin/mpi-rainbow-tables ${CFLAGS}
	
test: obj/test.o
	mkdir -p bin
	${CC} obj/test.o -o bin/mpi-rainbow-tables-test ${CFLAGS}

clean:
	rm obj/*
	rm bin/*
