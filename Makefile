all:
	g++ main.cpp -o main -g -lglfw -lX11 -ldl -lXrandr -lGLEW -lGL -I.
