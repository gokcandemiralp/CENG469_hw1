#include "main.h" 
#define BUFFER_OFFSET(i) ((char*)NULL + (i))


GLuint gProgram[1];
int gWidth, gHeight;
int programCount = 1;

GLint modelingMatrixLoc[1];
GLint viewingMatrixLoc[1];
GLint projectionMatrixLoc[1];
GLint eyePosLoc[1];

glm::mat4 projectionMatrix;
glm::mat4 viewingMatrix;
glm::mat4 modelingMatrix;
glm::vec3 eyePos(0, 0, 0);

int activeProgramIndex = 0;

vector<Vertex> gVertices;
vector<Texture> gTextures;
vector<Normal> gNormals;
vector<Face> gFaces;

GLuint gVertexAttribBuffer, gIndexBuffer;
GLint gInVertexLoc, gInNormalLoc;
int gVertexDataSizeInBytes, gNormalDataSizeInBytes;

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
						gTextures.push_back(Texture(c1, c2));
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

	gProgram[0] = glCreateProgram();
	GLuint vs1 = createVS("vert.glsl");
	GLuint fs1 = createFS("frag.glsl");
	glAttachShader(gProgram[0], vs1);
	glAttachShader(gProgram[0], fs1);
	glLinkProgram(gProgram[0]);
	glGetProgramiv(gProgram[0], GL_LINK_STATUS, &status);

	if (status != GL_TRUE){
		cout << "Program link failed" << endl;
		exit(-1);
	}

	for (int i = 0; i < 1; ++i){
		modelingMatrixLoc[i] = glGetUniformLocation(gProgram[i], "modelingMatrix");
		viewingMatrixLoc[i] = glGetUniformLocation(gProgram[i], "viewingMatrix");
		projectionMatrixLoc[i] = glGetUniformLocation(gProgram[i], "projectionMatrix");
		eyePosLoc[i] = glGetUniformLocation(gProgram[i], "eyePos");
	}
}

void initVBO(){
	GLuint vao;
	glGenVertexArrays(1, &vao);
	assert(vao > 0);
	glBindVertexArray(vao);
	cout << "vao = " << vao << endl;

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
	glClearColor(0, 0, 0, 1);
	glClearDepth(1.0f);
	glClearStencil(0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	static float angle = 0;

	float angleRad = (float)(angle / 180.0) * M_PI;

	// Compute the modeling matrix
	glm::mat4 matT = glm::translate(glm::mat4(1.0), glm::vec3(-0.1f, -0.2f, -7.0f));
	glm::mat4 matRx = glm::rotate<float>(glm::mat4(1.0), (0. / 180.) * M_PI, glm::vec3(1.0, 0.0, 0.0));
	glm::mat4 matRy = glm::rotate<float>(glm::mat4(1.0), (-180. / 180.) * M_PI, glm::vec3(0.0, 1.0, 0.0));
	glm::mat4 matRz = glm::rotate<float>(glm::mat4(1.0), angleRad, glm::vec3(0.0, 0.0, 1.0));
	modelingMatrix = matT * matRz * matRy * matRx;

	// Set the active program and the values of its uniform variables
	glUseProgram(gProgram[activeProgramIndex]);
	glUniformMatrix4fv(projectionMatrixLoc[activeProgramIndex], 1, GL_FALSE, glm::value_ptr(projectionMatrix));
	glUniformMatrix4fv(viewingMatrixLoc[activeProgramIndex], 1, GL_FALSE, glm::value_ptr(viewingMatrix));
	glUniformMatrix4fv(modelingMatrixLoc[activeProgramIndex], 1, GL_FALSE, glm::value_ptr(modelingMatrix));
	glUniform3fv(eyePosLoc[activeProgramIndex], 1, glm::value_ptr(eyePos));

	// Draw the scene
	drawModel();

	angle += 0.5;
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
	if (key == GLFW_KEY_Q && action == GLFW_PRESS){
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	}
	else if (key == GLFW_KEY_G && action == GLFW_PRESS){
		//glShadeModel(GL_SMOOTH);
		activeProgramIndex = 0;
	}
	else if (key == GLFW_KEY_P && action == GLFW_PRESS){
		//glShadeModel(GL_SMOOTH);
		activeProgramIndex = 1;
	}
	else if (key == GLFW_KEY_F && action == GLFW_PRESS){
		//glShadeModel(GL_FLAT);
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
	ParseObj(argv[1]);

	GLFWwindow* window;
	if (!glfwInit()){
		exit(-1);
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	int width = 640, height = 640;
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
