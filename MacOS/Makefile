all: listener client

listener: listener.c
	gcc -o listener listener.c

client: client.c
	gcc -o client client.c

clean:
	rm -f listener client tmp.txt tmp2.txt
