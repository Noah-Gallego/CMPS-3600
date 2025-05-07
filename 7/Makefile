# makefile
# compile to an object file
# then link the object file with libraries needed

all: lab7 mutex_test dotprod

#link the object files...
lab7: lab7.o
	gcc lab7.o -olab7 -lpthread -lm
mutex_test: mutex_test.o
	gcc mutex_test.o -omutex_test -lpthread -lm
dotprod: dotprod.o
	gcc dotprod.o -odotprod -lpthread -lm


#create the object files by compiling...
lab7.o: vrlab7.c
	gcc vrlab7.c -c -Wall
mutex_test.o: mutex_test.c
	gcc mutex_test.c -c -DPOSIX_4D9 -Wall
dotprod.o: dotprod.c
	gcc dotprod.c -c -DPOSIX_4D9 -Wall

clean:
	rm -f *.o
	rm -f lab7
	rm -f dotprod mutex_test
	rm -f out

