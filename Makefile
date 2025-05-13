all: client server

client: client.cpp bullets.cpp gamestructs.hpp
	g++ client.cpp bullets.cpp -o client -lSDL2 -lSDL2_ttf -lSDL2_image

server: server.cpp bullets.cpp gamestructs.hpp
	g++ server.cpp bullets.cpp -o server -lSDL2 -lSDL2_ttf

