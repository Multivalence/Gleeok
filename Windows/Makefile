all: listener client

listener: listener.c
	gcc -o listener listener.c -lws2_32

client: client.c
	gcc -o client client.c -lws2_32 -mwindows

clean:
	del listener.exe client.exe
