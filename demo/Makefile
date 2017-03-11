all: echo_client echo_server

echo_client: EchoClient.cpp  
	g++ -std=c++0x -pthread -g EchoClient.cpp -I../include -L../lib -lnetwork -llibbase -o echo_client

echo_server: EchoServer.cpp
	g++ -std=c++0x -pthread -g EchoServer.cpp -I../include -L../lib -lnetwork -llibbase -o echo_server
	

