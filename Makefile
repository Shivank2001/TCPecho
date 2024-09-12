
init_server:server
init_client:client

client:client_v1.o
	gcc client_v1.o -o client

server:MP1_server.o
	gcc MP1_server.o -o server

output: MP1_server.o client_v1.o
	gcc MP1_server.o -o server
	gcc client_v1.o -o client

MP1_server.o: MP1_server.c
	gcc -c MP1_server.c

client_v1.o: client_v1.c
	gcc -c client_v1.c

clean:
	rm -f output MP1_server.o client_v1.o client server
