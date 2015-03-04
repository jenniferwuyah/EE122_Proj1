objects = proj1_server proj1_client proj1_server4 proj1_client4

all: $(objects)

$(objects): %: %.c
	gcc -std=c99 -o $@ -Wno-implicit-function-declaration $<

clean: 
	rm $(objects)
