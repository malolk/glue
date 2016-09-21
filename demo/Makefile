all: echo_client echo_server

echo_client: EchoClient.cpp  
	g++ -std=c++0x -pthread -g EchoClient.cpp -I../include -L../lib -lglue_network -lglue_libbase -o echo_client

echo_server: EchoServer.cpp
	g++ -std=c++0x -pthread -g EchoServer.cpp -I../include -L../lib -lglue_network -lglue_libbase -o echo_server
	

