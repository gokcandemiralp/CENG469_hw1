#ifndef UTILS_H
#define UTILS_H

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#define _USE_MATH_DEFINES
#include <math.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp> 
#include <bits/stdc++.h>

using namespace std;

struct Vertex{
	Vertex(GLfloat inX, GLfloat inY, GLfloat inZ) : x(inX), y(inY), z(inZ) { }
	Vertex() : x(0), y(0), z(0) { }
	GLfloat x, y, z;

    void printVertex(){
        cout << fixed << setprecision(3) << "[" << x << "," << y << "," << z << "]";
    }
};

struct Texture{
	Texture(GLfloat inU, GLfloat inV) : u(inU), v(inV) { }
	GLfloat u, v;
};

struct Normal{
	Normal(GLfloat inX, GLfloat inY, GLfloat inZ) : x(inX), y(inY), z(inZ) { }
	GLfloat x, y, z;
};

struct Face{
	Face(int v[], int t[], int n[]) {
		vIndex[0] = v[0];
		vIndex[1] = v[1];
		vIndex[2] = v[2];
		tIndex[0] = t[0];
		tIndex[1] = t[1];
		tIndex[2] = t[2];
		nIndex[0] = n[0];
		nIndex[1] = n[1];
		nIndex[2] = n[2];
	}
	GLuint vIndex[3], tIndex[3], nIndex[3];
};

struct PointLight{
	glm::vec3 position;
	glm::vec3 color;
};

int Combination(int n, int k);
float Bernstein(int n, float t);
float calcBezierSurface(float s, float t, glm::vec3 (&controlPoints)[4][4]);
glm::vec3 calcBezierNormal(float u, float v, glm::vec3 (&controlPoints)[4][4]);
// void PrintControlPoints(int cPointY,int cPointX, float **controlPoints);

#endif