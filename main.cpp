#include "main.h" 
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

vector<Vertex> gVertices;
vector<Normal> gNormals;
vector<Face> gFaces;

GLuint gVertexAttribBuffer, gIndexBuffer;
GLint gInVertexLoc, gInNormalLoc;
int gVertexDataSizeInBytes, gNormalDataSizeInBytes;

GLint lightCount;
GLint cPointX, cPointY;
PointLight *pointLights;

float rotationAngle = -30;
float coordMultiplier = 0.8;
int sampleRate = 10;

Vertex **controlPoints;

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

		controlPoints = new Vertex*[cPointY];
		for(int iy = 0; iy < cPointY ; ++iy){
			controlPoints[iy] = new Vertex[cPointY];
		}

		GLfloat tempRead_z;
		GLfloat dGrid = min(1.0/(cPointX-1) , 1.0/(cPointY-1)); // Find the smallest grid step to fit
		cout << "dGrid: " << dGrid << "\n"; 
		GLfloat comp_y = -0.5;
		for(int iy = 0; iy < cPointY && getline(myfile, curLine) ; ++iy){
			stringstream str(curLine);
			GLfloat comp_x = -0.5;
			for(int ix = 0; ix < cPointX ; ++ix){
				str >> tempRead_z;
				controlPoints[iy][ix] = Vertex(comp_x,comp_y,tempRead_z);
				comp_x += dGrid;
			}
			cout << "comp_y: " << comp_y << "\n"; 
			comp_y += dGrid;
		}

		for(int iy = 0; iy < cPointY ; ++iy){ // Preview and delete
			for(int ix = 0; ix < cPointX ; ++ix){
				// cout << heightMap[iy][ix] << " ";
				Vertex tempVertex = controlPoints[iy][ix];
				cout << fixed << setprecision(1) << "[x:" <<tempVertex.x << " y:" <<tempVertex.y << " z:" <<tempVertex.z << "] ";
			}
			cout << "\n";
		}

		myfile.close();
	}
	else{
		return false;
	}
	return true;
}

bool ParseObj(const string& fileName){
	fstream myfile;
	myfile.open(fileName.c_str(), std::ios::in);

	if (myfile.is_open()){
		string curLine;
		while (getline(myfile, curLine)){
			stringstream str(curLine);
			GLfloat c1, c2, c3;
			GLuint index[9];
			string tmp;

			if (curLine.length() >= 2){
				if (curLine[0] == 'v'){
					if (curLine[1] == 't'){ // texture
						str >> tmp; // consume "vt"
						str >> c1 >> c2;
					}
					else if (curLine[1] == 'n'){ // normal
						str >> tmp; // consume "vn"
						str >> c1 >> c2 >> c3;
						gNormals.push_back(Normal(c1, c2, c3));
					}
					else{ // vertex
						str >> tmp; // consume "v"
						str >> c1 >> c2 >> c3;
						gVertices.push_back(Vertex(c1, c2, c3));
					}
				}
				else if (curLine[0] == 'f'){ // face
					str >> tmp; // consume "f"
					char c;
					int vIndex[3], nIndex[3], tIndex[3];
					str >> vIndex[0]; str >> c >> c; // consume "//"
					str >> nIndex[0];
					str >> vIndex[1]; str >> c >> c; // consume "//"
					str >> nIndex[1];
					str >> vIndex[2]; str >> c >> c; // consume "//"
					str >> nIndex[2];

					assert(vIndex[0] == nIndex[0] &&
						vIndex[1] == nIndex[1] &&
						vIndex[2] == nIndex[2]); // a limitation for now

					for (int c = 0; c < 3; ++c){ // make indices start from 0
						vIndex[c] -= 1;
						nIndex[c] -= 1;
						tIndex[c] -= 1;
					}
					gFaces.push_back(Face(vIndex, tIndex, nIndex));
				}
				else{
					cout << "Ignoring unidentified line in obj file: " << curLine << endl;
				}
			}
		}
		myfile.close();
	}
	else{
		return false;
	}	
	assert(gVertices.size() == gNormals.size());
	return true;
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

	gVertexDataSizeInBytes = gVertices.size() * 3 * sizeof(GLfloat);
	gNormalDataSizeInBytes = gNormals.size() * 3 * sizeof(GLfloat);
	int indexDataSizeInBytes = gFaces.size() * 3 * sizeof(GLuint);
	GLfloat* vertexData = new GLfloat[gVertices.size() * 3];
	GLfloat* normalData = new GLfloat[gNormals.size() * 3];
	GLuint* indexData = new GLuint[gFaces.size() * 3];

	float minX = 1e6, maxX = -1e6;
	float minY = 1e6, maxY = -1e6;
	float minZ = 1e6, maxZ = -1e6;

	for (int i = 0; i < gVertices.size(); ++i){
		vertexData[3 * i] = gVertices[i].x;
		vertexData[3 * i + 1] = gVertices[i].y;
		vertexData[3 * i + 2] = gVertices[i].z;

		minX = std::min(minX, gVertices[i].x);
		maxX = std::max(maxX, gVertices[i].x);
		minY = std::min(minY, gVertices[i].y);
		maxY = std::max(maxY, gVertices[i].y);
		minZ = std::min(minZ, gVertices[i].z);
		maxZ = std::max(maxZ, gVertices[i].z);
	}

	for (int i = 0; i < gNormals.size(); ++i){
		normalData[3 * i] = gNormals[i].x;
		normalData[3 * i + 1] = gNormals[i].y;
		normalData[3 * i + 2] = gNormals[i].z;
	}

	for (int i = 0; i < gFaces.size(); ++i){
		indexData[3 * i] = gFaces[i].vIndex[0];
		indexData[3 * i + 1] = gFaces[i].vIndex[1];
		indexData[3 * i + 2] = gFaces[i].vIndex[2];
	}


	glBufferData(GL_ARRAY_BUFFER, gVertexDataSizeInBytes + gNormalDataSizeInBytes, 0, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, gVertexDataSizeInBytes, vertexData);
	glBufferSubData(GL_ARRAY_BUFFER, gVertexDataSizeInBytes, gNormalDataSizeInBytes, normalData);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexDataSizeInBytes, indexData, GL_STATIC_DRAW);

	// done copying; can free now
	delete[] vertexData;
	delete[] normalData;
	delete[] indexData;

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(gVertexDataSizeInBytes));
}

void init(){
	glEnable(GL_DEPTH_TEST);
	initShaders();
	initVBO();
}

void drawModel(){
	glBindBuffer(GL_ARRAY_BUFFER, gVertexAttribBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gIndexBuffer);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(gVertexDataSizeInBytes));

	glDrawElements(GL_TRIANGLES, gFaces.size() * 3, GL_UNSIGNED_INT, 0);
}

void display(){
	string tmp_str;
	glClearColor(0, 0, 0, 1);
	glClearDepth(1.0f);
	glClearStencil(0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	// Compute the modeling matrix
	glm::mat4 matT = glm::translate(glm::mat4(1.0), glm::vec3(-0.1f, -0.2f, -7.0f));
	glm::mat4 matR = glm::rotate(glm::mat4(1.f), glm::radians(rotationAngle), glm::vec3(0, 1, 0));
	glm::mat4 matS = glm::scale(glm::mat4(1.f), glm::vec3(coordMultiplier,coordMultiplier,coordMultiplier));

	modelingMatrix = matT * matS * matR;

	// Set the active program and the values of its uniform variables
	glUseProgram(gProgram);
	glUniformMatrix4fv(projectionMatrixLoc, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
	glUniformMatrix4fv(viewingMatrixLoc, 1, GL_FALSE, glm::value_ptr(viewingMatrix));
	glUniformMatrix4fv(modelingMatrixLoc, 1, GL_FALSE, glm::value_ptr(modelingMatrix));
	glUniform3fv(eyePosLoc, 1, glm::value_ptr(eyePos));

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

	viewingMatrix = glm::mat4(1);
}

void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS){
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	}
	else if (key == GLFW_KEY_W && action == GLFW_PRESS){
		if(sampleRate < 80){sampleRate += 2;}
		cout << "Key W Pressed | sampleRate ==  " << sampleRate << "\n";
	}
	else if (key == GLFW_KEY_S && action == GLFW_PRESS){
		if(sampleRate > 2){sampleRate -= 2;}
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
}

void mainLoop(GLFWwindow* window)
{
	while (!glfwWindowShouldClose(window)){
		display();
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}

int main(int argc, char** argv)   // Create Main Function For Bringing It All Together
{
	if(argc != 2){return 1;}
	ParseSurface(argv[1]);
	ParseObj("bunny.obj");

	GLFWwindow* window;
	if (!glfwInit()){
		exit(-1);
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	int width = 800, height = 600;
	window = glfwCreateWindow(width, height, "Simple Example", NULL, NULL);

	if (!window){
		glfwTerminate();
		exit(-1);
	}

	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	// Initialize GLEW to setup the OpenGL Function pointers
	if (GLEW_OK != glewInit()){
		std::cout << "Failed to initialize GLEW" << std::endl;
		return EXIT_FAILURE;
	}

	char rendererInfo[512] = { 0 };
	strcpy(rendererInfo, (const char*)glGetString(GL_RENDERER));
	strcat(rendererInfo, " - ");
	strcat(rendererInfo, (const char*)glGetString(GL_VERSION));
	glfwSetWindowTitle(window, rendererInfo);

	init();

	glfwSetKeyCallback(window, keyboard);
	glfwSetWindowSizeCallback(window, reshape);

	reshape(window, width, height); // need to call this once ourselves
	mainLoop(window); // this does not return unless the window is closed

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}
