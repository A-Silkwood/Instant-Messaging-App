cs:
	make server
	make client

server: server.cpp
	g++ server.cpp -o server

client: client.cpp
	g++ client.cpp -o client

test:
	make test_server
	make test_client

test_server: UDPEchoServer.c
	gcc UDPEchoServer.c -o test_server

test_client: UDPEchoServer.c
	gcc UDPEchoClient.c -o test_client

clean:
	rm server
	rm client

clean_test:
	rm test_server
	rm test_client