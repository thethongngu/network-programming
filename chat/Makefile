all: npshell.cpp npshell.h string_utils.h np_simple.cpp user.cpp user.h np_single_proc.cpp
	g++ npshell.cpp npshell.h string_utils.h np_simple.cpp -o np_simple -std=c++11
	g++ npshell.cpp npshell.h string_utils.h np_single_proc.cpp user.h user.cpp -o np_single_proc -std=c++11

clean:
	rm -rf npshell *.o *.gch