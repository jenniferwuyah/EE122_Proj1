objects = server1 client1

all: $(objects)

$(objects): %: %.c
	gcc -o $@ $<

clean: 
	rm $(objects)
