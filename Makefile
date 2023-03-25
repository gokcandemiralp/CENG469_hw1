all:
	g++ main.cpp utils.cpp -o main -g -lglfw -lX11 -ldl -lXrandr -lGLEW -lGL -I.
