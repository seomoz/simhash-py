CPP     = g++
CPPOPTS = -g -O3 -Wall

all: bench hash test

deps: hashes/* tokenizers/* hash.hpp simhash.h simhash.cpp

bench: simhash.o bench.cpp hashes/* tokenizers/*
	$(CPP) $(CPPOPTS) -o bench bench.cpp simhash.o -lJudy

hash: hash.cpp deps
	$(CPP) $(CPPOPTS) -o hash hash.cpp

simhash.o: simhash.h simhash.cpp
	$(CPP) $(CPPOPTS) -o simhash.o -c simhash.cpp

test: test.cpp deps
	$(CPP) $(CPPOPTS) -o test test.cpp -Ideps/Catch/single_include

clean:
	rm -rf driver *.o
