part1: http_server.o console.o session.o string_utils.o
	g++ http_server.o string_utils.o -o http_server -std=c++11 -pthread -lboost_system
	g++ console.o session.o string_utils.o -o console.cgi -std=c++11 -pthread -lboost_system

part2: window_http_server.o window_console.o window_session.o string_utils.o
	g++ window_http_server.o window_console.o window_session.o string_utils.o -o cgi_server -lws2_32 -lwsock32 -std=c++11

window_http_server.o: window_http_server.cpp
	g++ -c window_http_server.cpp -o window_http_server.o -lws2_32 -lwsock32 -std=c++11

window_session.o: window_session.cpp
	g++ -c window_session.cpp -o window_session.o -lws2_32 -lwsock32 -std=c++11

window_console.o: window_console.cpp
	g++ -c window_console.cpp -o window_console.o -lws2_32 -lwsock32 -std=c++11

console.o: console.cpp
	g++ -c console.cpp -o console.o -std=c++11 -pthread -lboost_system

session.o: session.cpp
	g++ -c session.cpp -o session.o -std=c++11 -pthread -lboost_system

http_server.o: http_server.cpp
	g++ -c http_server.cpp -o http_server.o -std=c++11 -pthread -lboost_system

string_utils.o: string_utils.cpp
	g++ -c string_utils.cpp -o string_utils.o -std=c++11 -pthread -lboost_system

clean:
	rm *.o http_server console.cgi *.gch