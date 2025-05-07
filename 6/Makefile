# Makefile for week-6 consumer/producer

all: echo create_thread threads1 sample1 sample2 sample3 vrlab6

echo: echo.c
	gcc -g echo.c -Wall -oecho -lpthread

sample1: sample1.c
	gcc sample1.c -Wall -osample1 -lpthread

sample2: sample2.c
	gcc sample2.c -Wall -osample2 -lpthread

sample3: sample3.c
	gcc sample3.c -Wall -osample3 -lpthread

create_thread: create_thread.c
	gcc create_thread.c -Wall -ocreate_thread -lpthread

threads1: threads1.c
	gcc threads1.c -Wall -othreads1 -lpthread

vrlab6: vrlab6.c
	gcc -g vrlab6.c -Wall -ovrlab6 -lpthread

clean:
	rm -f echo create_thread vrlab6
	rm -f threads1 sample1 sample2 sample3 log
	
