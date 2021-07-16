all: server.o sock4_request.o sock4_response.o string_utils.o console.o sock4_session.o
	g++ server.o sock4_request.o sock4_response.o -o socks_server -std=c++11 -pthread -lboost_system
	g++ console.o string_utils.o sock4_session.o sock4_request.o sock4_response.o -o hw4.cgi -std=c++11 -pthread -lboost_system

server.o: server.cpp
	g++ -c server.cpp -o server.o -std=c++11 -pthread -lboost_system

sock4_request.o: sock4_request.cpp
	g++ -c sock4_request.cpp -o sock4_request.o -std=c++11 -pthread -lboost_system

sock4_response.o: sock4_response.cpp
	g++ -c sock4_response.cpp -o sock4_response.o -std=c++11 -pthread -lboost_system

console.o: console.cpp
	g++ -c console.cpp -o console.o -std=c++11 -pthread -lboost_system

sock4_session.o: sock4_session.cpp
	g++ -c sock4_session.cpp -o sock4_session.o -std=c++11 -pthread -lboost_system

string_utils.o: string_utils.cpp
	g++ -c string_utils.cpp -o string_utils.o -std=c++11 -pthread -lboost_system

clean:
	rm -rf *.o socks_server hw4.cgi