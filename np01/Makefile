all: npshell.o
	g++ npshell.o -o npshell -std=c++11

npshell.o: npshell.cpp npshell.hpp
	g++ -c npshell.cpp npshell.hpp -std=c++11

clean:
	rm -rf npshell *.o *.gch