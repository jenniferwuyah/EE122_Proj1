objects = server1.o client1.o

all: $(objects)

$(objects): %.o: %.c
	gcc -c -W -o $@ $<

clean: 
	rm $(objects)
