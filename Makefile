all: main.cpp
	g++ -o main main.cpp httpParser.cpp httpParser.h
default:main.cpp
	g++ -o main main.cpp httpParser.cpp httpParser.h
clean:
	rm *.gch