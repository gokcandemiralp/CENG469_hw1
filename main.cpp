#include "utils.h" 
#define BUFFER_OFFSET(i) ((char*)NULL + (i))


GLuint gProgram;
int gWidth, gHeight;
int programCount = 1;

GLint modelingMatrixLoc;
GLint viewingMatrixLoc;
GLint projectionMatrixLoc;
GLint eyePosLoc;

glm::mat4 projectionMatrix;
glm::mat4 viewingMatrix;
glm::mat4 modelingMatrix;
glm::vec3 eyePos(0, 0, 2);

GLuint gVertexAttribBuffer, gIndexBuffer;
GLint gInVertexLoc, gInNormalLoc;
int indexDataSizeInBytes, UVdataSizeInBytes, anchorDataSizeInBytes;

GLint lightCount;
GLint cPointX, cPointY;
int anchorCountY, anchorCountX;
PointLight *pointLights;

float rotationAngle = -30;
float coordMultiplier = 0.8;
int sampleRate = 10;
int surfaceCount = 1;

glm::vec3 controlSurfaces[6][6][4][4];
GLfloat* UVdata;
bool updateSurface = true;

void calcSurfaceVertices(){
	if(!updateSurface){return;}

	int anchorDownScale = max(anchorCountX,anchorCountY);
	int surfaceIndex = 0;
	float surfaceSize = 1.0/anchorDownScale;
	surfaceCount = anchorCountX * anchorCountY;
	int vertexIndex = 0;

	int verticesPerSurface = sampleRate * sampleRate;
	int squaresPerSurface = (sampleRate-1) * (sampleRate-1);

	int faceEntries = squaresPerSurface * 6 * surfaceCount;
	int UVentries = verticesPerSurface * 2 * surfaceCount;
	int anchorEntries = verticesPerSurface * 2 * surfaceCount;

	indexDataSizeInBytes = faceEntries * sizeof(GLuint);
	UVdataSizeInBytes = UVentries * sizeof(GLfloat);
	anchorDataSizeInBytes = anchorEntries * sizeof(GLfloat);

	GLuint* indexData = new GLuint[faceEntries];
	GLfloat* UVdata = new GLfloat[UVentries];
	GLfloat* anchorData = new GLfloat[anchorEntries];

	for(int anchorY = 0; anchorY < anchorCountY ; ++anchorY){
		for(int anchorX = 0; anchorX < anchorCountX ; ++anchorX){
			float step = 1.0/(sampleRate-1);
			float fraction = step/anchorDownScale;
			float tempZ = 0;
			int vIterator = 0;
			for(int iy = 0 ; iy < sampleRate; ++iy){
				for(int ix = 0 ; ix < sampleRate ; ++ix){
					UVdata[2*vIterator]     = step*ix	; UVdata[2*vIterator + 1]     = step*iy; 
					anchorData[2*vIterator] = anchorX   ; anchorData[2*vIterator + 1] = anchorY;
					cout << "anchorY == " << anchorY << " | anchorX == " << anchorX << "\n";
					++vIterator;
					//cout << tempZ << " ";
				}
				// cout << "\n";
			}

			int fIterator = 0;
			for(int iy = 0 ; iy < sampleRate-1; ++iy){
				for(int ix = 0 ; ix < sampleRate-1 ; ++ix){
					vIterator = iy*sampleRate+ix;
					fIterator = iy*(sampleRate-1)+ix;

					indexData[6 * ((squaresPerSurface * surfaceIndex) + fIterator)    ] = (verticesPerSurface * surfaceIndex) + vIterator;
					indexData[6 * ((squaresPerSurface * surfaceIndex) + fIterator) + 1] = (verticesPerSurface * surfaceIndex) + vIterator + sampleRate;
					indexData[6 * ((squaresPerSurface * surfaceIndex) + fIterator) + 2] = (verticesPerSurface * surfaceIndex) + vIterator + 1;
					indexData[6 * ((squaresPerSurface * surfaceIndex) + fIterator) + 3] = (verticesPerSurface * surfaceIndex) + vIterator + 1;
					indexData[6 * ((squaresPerSurface * surfaceIndex) + fIterator) + 4] = (verticesPerSurface * surfaceIndex) + vIterator + sampleRate;
					indexData[6 * ((squaresPerSurface * surfaceIndex) + fIterator) + 5] = (verticesPerSurface * surfaceIndex) + vIterator + sampleRate + 1;
				}
			}
			++surfaceIndex;
		}
	}

	glBufferData(GL_ARRAY_BUFFER, UVdataSizeInBytes + anchorDataSizeInBytes, 0, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, UVdataSizeInBytes, UVdata);
	glBufferSubData(GL_ARRAY_BUFFER, UVdataSizeInBytes, anchorDataSizeInBytes, anchorData);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexDataSizeInBytes, indexData, GL_STATIC_DRAW);

	// done copying; can free now
	delete[] indexData;
	delete[] UVdata;
	delete[] anchorData;

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(UVdataSizeInBytes));

	updateSurface = false;
}

bool ParseSurface(const string& fileName){
	fstream myfile;
	myfile.open(fileName.c_str(), std::ios::in);
	string str_Y, str_X;

	if (myfile.is_open()){
		string curLine;

		getline(myfile, curLine); 		// initial read for light sources
		lightCount = stoi(curLine);		// initial read for light sources
		pointLights = new PointLight[lightCount];

		for(int lightIndex = 0; lightIndex < lightCount && getline(myfile, curLine) ; ++lightIndex){
			stringstream str(curLine);
			str >> pointLights[lightIndex].position.x >> pointLights[lightIndex].position.y >> pointLights[lightIndex].position.z >>
					pointLights[lightIndex].color.x >> pointLights[lightIndex].color.y >> pointLights[lightIndex].color.z;
		}

		getline(myfile, curLine); 	// initial read for the control point matrix
		stringstream str(curLine);	// initial read for the control point matrix
		str >> cPointY >> cPointX;	// initial read for the control point matrix
		anchorCountY = cPointY /4;
		anchorCountX = cPointX /4;

		float **controlPoints;
		controlPoints = new float*[cPointY];	// Allocate space for Control Points
		for(int iy = 0; iy < cPointY ; ++iy){
			controlPoints[iy] = new float[cPointX];
		}

		GLfloat tempRead_z;
		for(int iy = 0; iy < cPointY && getline(myfile, curLine) ; ++iy){
			stringstream str(curLine);
			for(int ix = 0; ix < cPointX ; ++ix){
				str >> tempRead_z;
				controlPoints[iy][ix] = tempRead_z;
			}
		}

		// PrintControlPoints(cPointY, cPointX, controlPoints); // Preview of the controlPoints

		int anchorDownScale = max(anchorCountX,anchorCountY);
		float surfaceSize = 1.0/anchorDownScale;

		for(int anchorY = 0; anchorY < anchorCountY ; ++anchorY){
			for(int anchorX = 0; anchorX < anchorCountX ; ++anchorX){
				for(int offsetY = 0 ; offsetY < 4 ; ++offsetY){
					for(int offsetX = 0 ; offsetX < 4 ; ++offsetX){
						controlSurfaces[anchorY][anchorX][offsetY][offsetX].x = (offsetX*(1.0/(3*anchorDownScale))) + (surfaceSize*anchorX) - 0.5;
						controlSurfaces[anchorY][anchorX][offsetY][offsetX].y = (offsetY*(1.0/(3*anchorDownScale))) + (surfaceSize*anchorY) - 0.5;
						controlSurfaces[anchorY][anchorX][offsetY][offsetX].z = controlPoints[4*anchorY+offsetY][4*anchorX+offsetX];
					}
				}
			}
		}

		for(int iy = 0; iy < cPointY ; ++iy){
			delete[] controlPoints[iy];
		}
		delete[] controlPoints;

		myfile.close();
		return true;
	}
	return false;
}

bool ReadDataFromFile(const string& fileName,string& data){
	fstream myfile;
	myfile.open(fileName.c_str(), std::ios::in);

	if (myfile.is_open()){
		string curLine;

		while (getline(myfile, curLine)){
			data += curLine;
			if (!myfile.eof()){
				data += "\n";
			}
		}
		myfile.close();
	}
	else{
		return false;
	}
	return true;
}

GLuint createVS(const char* shaderName){
	string shaderSource;
	string filename(shaderName);
	if (!ReadDataFromFile(filename, shaderSource)){
		cout << "Cannot find file name: " + filename << endl;
		exit(-1);
	}

	GLint length = shaderSource.length();
	const GLchar* shader = (const GLchar*)shaderSource.c_str();

	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs, 1, &shader, &length);
	glCompileShader(vs);

	return vs;
}

GLuint createFS(const char* shaderName){
	string shaderSource;
	string filename(shaderName);
	if (!ReadDataFromFile(filename, shaderSource)){
		cout << "Cannot find file name: " + filename << endl;
		exit(-1);
	}

	GLint length = shaderSource.length();
	const GLchar* shader = (const GLchar*)shaderSource.c_str();

	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fs, 1, &shader, &length);
	glCompileShader(fs);

	return fs;
}

void initShaders(){
	GLint status;

	gProgram = glCreateProgram();
	GLuint vs1 = createVS("vert.glsl");
	GLuint fs1 = createFS("frag.glsl");
	glAttachShader(gProgram, vs1);
	glAttachShader(gProgram, fs1);
	glLinkProgram(gProgram);
	glGetProgramiv(gProgram, GL_LINK_STATUS, &status);

	if (status != GL_TRUE){
		cout << "Program link failed" << endl;
		exit(-1);
	}

	modelingMatrixLoc = glGetUniformLocation(gProgram, "modelingMatrix");
	viewingMatrixLoc = glGetUniformLocation(gProgram, "viewingMatrix");
	projectionMatrixLoc = glGetUniformLocation(gProgram, "projectionMatrix");
	eyePosLoc = glGetUniformLocation(gProgram, "eyePos");
}

void initUnfiorms(){
	glUseProgram(gProgram);

	glUniform1i(glGetUniformLocation(gProgram, "anchorCountY"), anchorCountY);
	glUniform1i(glGetUniformLocation(gProgram, "anchorCountY"), anchorCountX);

	string tmp_str;
	for(int anchorY = 0; anchorY < anchorCountY ; ++anchorY){
		for(int anchorX = 0; anchorX < anchorCountX ; ++anchorX){
			cout << "anchorY == " << anchorY << " | anchorX == " << anchorX << "\n";
			for(int offsetY = 0 ; offsetY < 4 ; ++offsetY){
				for(int offsetX = 0 ; offsetX < 4 ; ++offsetX){
					tmp_str = "controlSurfaces[" + to_string(anchorY) + "]" + 
					"[" + to_string(anchorX) + "]"+
					"[" + to_string(offsetY) + "]"+
					"[" + to_string(offsetX) + "]";

					glUniform3fv(glGetUniformLocation(gProgram, tmp_str.c_str()), 1, 
					glm::value_ptr(controlSurfaces[anchorY][anchorX][offsetY][offsetX]));
					cout << tmp_str.c_str() << " ";
				}
				cout << "\n";
			}
			cout << "\n";
		}
	}
}

void initVBO(){
	GLuint vao;
	glGenVertexArrays(1, &vao);
	assert(vao > 0);
	glBindVertexArray(vao);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	assert(glGetError() == GL_NONE);

	glGenBuffers(1, &gVertexAttribBuffer);
	glGenBuffers(1, &gIndexBuffer);

	assert(gVertexAttribBuffer > 0 && gIndexBuffer > 0);

	glBindBuffer(GL_ARRAY_BUFFER, gVertexAttribBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gIndexBuffer);
}

void drawModel(){
	glBindBuffer(GL_ARRAY_BUFFER, gVertexAttribBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gIndexBuffer);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(UVdataSizeInBytes));

	glDrawElements(GL_TRIANGLES, (sampleRate-1) * (sampleRate-1) * surfaceCount * 6 , GL_UNSIGNED_INT, 0);
}

void display(){
	string tmp_str;
	glClearColor(0, 0, 0, 1);
	glClearDepth(1.0f);
	glClearStencil(0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	calcSurfaceVertices(); // Create vetices according to detail amount and control points

	// Compute the modeling matrix
	glm::mat4 matR = glm::rotate(glm::mat4(1.f), glm::radians(rotationAngle), glm::vec3(1, 0, 0));
	glm::mat4 matS = glm::scale(glm::mat4(1.f), glm::vec3(coordMultiplier,coordMultiplier,coordMultiplier));
	modelingMatrix = matS * matR;

	// Set the active program and the values of its uniform variables
	glUseProgram(gProgram);
	glUniformMatrix4fv(projectionMatrixLoc, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
	glUniformMatrix4fv(viewingMatrixLoc, 1, GL_FALSE, glm::value_ptr(viewingMatrix));
	glUniformMatrix4fv(modelingMatrixLoc, 1, GL_FALSE, glm::value_ptr(modelingMatrix));

	glUniform3fv(eyePosLoc, 1, glm::value_ptr(eyePos));
	glUniform1i(glGetUniformLocation(gProgram, "sampleRate"), sampleRate);

	glUniform1i(glGetUniformLocation(gProgram, "lightCount"), lightCount);
	for(int lightIndex = 0; lightIndex < lightCount ; ++lightIndex){
		tmp_str = "pointLights[" + std::to_string(lightIndex) + "].position";
		glUniform3fv(glGetUniformLocation(gProgram, tmp_str.c_str()), 1, glm::value_ptr(pointLights[lightIndex].position));
		tmp_str = "pointLights[" + std::to_string(lightIndex) + "].color";
		glUniform3fv(glGetUniformLocation(gProgram, tmp_str.c_str()), 1, glm::value_ptr(pointLights[lightIndex].color));
	}
	
	drawModel();
}

void reshape(GLFWwindow* window, int w, int h){
	w = w < 1 ? 1 : w;
	h = h < 1 ? 1 : h;

	gWidth = w;
	gHeight = h;

	glViewport(0, 0, w, h);

	float fovyRad = (float)(45.0 / 180.0) * M_PI;
	projectionMatrix = glm::perspective(fovyRad, w/(float) h, 1.0f, 100.0f);

	viewingMatrix = glm::lookAt(eyePos,glm::vec3(0,0,0),glm::vec3(0,1,0));
}

void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods){
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS){
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	}
	else if (key == GLFW_KEY_W && action == GLFW_PRESS){
		if(sampleRate < 80){
			sampleRate += 2;
			updateSurface = true;
		}
		cout << "Key W Pressed | sampleRate ==  " << sampleRate << "\n";
	}
	else if (key == GLFW_KEY_S && action == GLFW_PRESS){
		if(sampleRate > 2){
			sampleRate -= 2;
			updateSurface = true;
		}
		cout << "Key S Pressed | sampleRate ==  " << sampleRate << "\n";
	}
	else if (key == GLFW_KEY_E && action == GLFW_PRESS){
		coordMultiplier += 0.1;
		cout << "Key E Pressed | coordMultiplier ==  " << coordMultiplier << "\n";
	}
	else if (key == GLFW_KEY_D && action == GLFW_PRESS){
		if(coordMultiplier>0.11){coordMultiplier -= 0.1;}
		cout << "Key D Pressed | coordMultiplier ==  " << coordMultiplier << "\n";
	}
	else if (key == GLFW_KEY_R && action == GLFW_PRESS){
		rotationAngle += 10;
		cout << "Key R Pressed | rotationAngle ==  " << rotationAngle << "\n";
	}
	else if (key == GLFW_KEY_F && action == GLFW_PRESS){
		rotationAngle -= 10;
		cout << "Key F Pressed | rotationAngle ==  " << rotationAngle << "\n";
	}
	else if (key == GLFW_KEY_L && action == GLFW_PRESS){
		glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
		cout << "Key L Pressed | Wireframe" << "\n";
	}
	else if (key == GLFW_KEY_O && action == GLFW_PRESS){
		glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
		cout << "Key O Pressed | Solid Render" << "\n";
	}
}

void mainLoop(GLFWwindow* window){
	while (!glfwWindowShouldClose(window)){
		display();
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}

int main(int argc, char** argv){
	if(argc != 2){return 1;}
	ParseSurface(argv[1]);

	GLFWwindow* window;
	if (!glfwInit()){
		exit(-1);
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	int width = 800, height = 600;
	window = glfwCreateWindow(width, height, "Simple Example", NULL, NULL);

	if (!window){
		glfwTerminate();
		exit(-1);
	}

	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	if (GLEW_OK != glewInit()){
		std::cout << "Failed to initialize GLEW" << std::endl;
		return EXIT_FAILURE;
	}

	char rendererInfo[512] = { 0 };
	strcpy(rendererInfo, (const char*)glGetString(GL_RENDERER));
	strcat(rendererInfo, " - ");
	strcat(rendererInfo, (const char*)glGetString(GL_VERSION));
	glfwSetWindowTitle(window, rendererInfo);

	glEnable(GL_DEPTH_TEST);
	initShaders();
	initVBO();
	initUnfiorms();
	calcSurfaceVertices();

	glfwSetKeyCallback(window, keyboard);
	glfwSetWindowSizeCallback(window, reshape);

	reshape(window, width, height); // need to call this once ourselves
	mainLoop(window); // this does not return unless the window is closed

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}
