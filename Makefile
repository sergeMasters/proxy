all: lb
	
lb: lb.c
	gcc  -pthread -D_GNU_SOURCE -D_BSD_SOURCE -std=c99 lb.c queue.c -o $@
clean:
	rm lb
