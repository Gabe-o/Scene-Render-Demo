// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <sstream>
#include <vector>
using namespace std;

#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>


#include <vector>
#include <iostream>

struct VertexData {
	glm::vec3 pos;
	glm::vec3 normal;
	glm::vec3 color;
	glm::vec2 uv;
	VertexData() {}
	VertexData(float x, float y, float z) : pos(x, y, z) {}
	VertexData(float x, float y, float z, float nx, float ny, float nz, float u, float v) : pos(x, y, z), normal(nx, ny, nz), uv(u, v) {}
	VertexData(float x, float y, float z, float nx, float ny, float nz, float r, float g, float b, float u, float v) : pos(x, y, z), normal(nx, ny, nz), color(r, g, b), uv(u, v) {}
};

struct TriData {
	unsigned int indices[3];
	TriData() {}
    TriData(unsigned int v1, unsigned int v2, unsigned int v3) {
        indices[0] = v1;
        indices[1] = v2;
        indices[2] = v3;
    }
};

void readPLYFile(const std::string& filePath, std::vector<VertexData>& vertices, std::vector<TriData>& triangles) {
    ifstream file(filePath);
    string line;
	int numVertices = 0, numFaces = 0;
	std::vector<std::string> vertexPropsOrder;

	while (std::getline(file, line)) {
		// Get number of vertices
		string delimiter = "element vertex ";
		size_t pos = line.find(delimiter);
		if (pos != string::npos) {
			numVertices = stoi(line.substr(pos + delimiter.length()));
		}

		// If vertices parse order of props
		if(numVertices > 0) {
			string delimiter = "property float ";
			size_t pos = line.find(delimiter);
			if (pos != string::npos) {
				std::string prop = line.substr(pos + delimiter.length());
        		vertexPropsOrder.push_back(prop);
			}
		}

		// Get number of faces
		delimiter = "element face ";
		pos = line.find(delimiter);
		if (pos != string::npos) {
			numFaces = stoi(line.substr(pos + delimiter.length()));
		}

		if (line == "end_header") {
			break;
		}
	}

	// Get vertex info
	for (int i = 0; i < numVertices; ++i) {
		getline(file, line);
		istringstream ss(line);
		string token;
		VertexData vertex;
		int propIndex = 0;
		while(std::getline(ss, token, ' ')) {
			if(vertexPropsOrder[propIndex] == "x") vertex.pos.x = stof(token);
			else if(vertexPropsOrder[propIndex] == "y") vertex.pos.y = stof(token);
			else if(vertexPropsOrder[propIndex] == "z") vertex.pos.z = stof(token);
			else if(vertexPropsOrder[propIndex] == "nx") vertex.normal.x = stof(token);
			else if(vertexPropsOrder[propIndex] == "ny") vertex.normal.y = stof(token);
			else if(vertexPropsOrder[propIndex] == "nz") vertex.normal.z = stof(token);
			else if(vertexPropsOrder[propIndex] == "red") vertex.color.r = stof(token);
			else if(vertexPropsOrder[propIndex] == "green") vertex.color.g = stof(token);
			else if(vertexPropsOrder[propIndex] == "blue") vertex.color.b = stof(token);
			else if(vertexPropsOrder[propIndex] == "u") vertex.uv.x = stof(token);
			else if(vertexPropsOrder[propIndex] == "v") vertex.uv.y = stof(token);
			propIndex++;
		}
		vertices.push_back(vertex);
	}

	// Get face info
	for (int i = 0; i < numFaces; ++i) {
		getline(file, line);
		istringstream ss(line);
		string token;
		TriData triangle;
		int index = 0;
		while(std::getline(ss, token, ' ')) {
			if(index > 0 && index < 4) { // Skip the first token (number of vertices in the face)
				triangle.indices[index - 1] = std::stoi(token);
			}
			index++;
		}
		triangles.push_back(triangle);
	}

    file.close();
}

/**
 * Given a file path imagepath, read the data in that bitmapped image
 * and return the raw bytes of color in the data pointer.
 * The width and height of the image are returned in the weight and height pointers,
 * respectively.
 *
 * usage:
 *
 * unsigned char* imageData;
 * unsigned int width, height;
 * loadARGB_BMP("mytexture.bmp", &imageData, &width, &height);
 *
 * Modified from https://github.com/opengl-tutorials/ogl.
 */
void loadARGB_BMP(const char* imagepath, unsigned char** data, unsigned int* width, unsigned int* height) {
    // Data read from the header of the BMP file
    unsigned char header[54];
    unsigned int dataPos;
    unsigned int imageSize;
    // Actual RGBA data

    // Open the file
    FILE * file = fopen(imagepath,"rb");
    if (!file){
        printf("%s could not be opened. Are you in the right directory?\n", imagepath);
        getchar();
        return;
    }

    // Read the header, i.e. the 54 first bytes

    // If less than 54 bytes are read, problem
    if ( fread(header, 1, 54, file)!=54 ){
        printf("Not a correct BMP file1\n");
        fclose(file);
        return;
    }

    // Read the information about the image
    dataPos    = *(int*)&(header[0x0A]);
    imageSize  = *(int*)&(header[0x22]);
    *width      = *(int*)&(header[0x12]);
    *height     = *(int*)&(header[0x16]);
    // A BMP files always begins with "BM"
    if ( header[0]!='B' || header[1]!='M' ){
        printf("Not a correct BMP file2\n");
        fclose(file);
        return;
    }
    // Make sure this is a 32bpp file
    if ( *(int*)&(header[0x1E])!=3  ) {
        printf("Not a correct BMP file3\n");
        fclose(file);
        return;
    }
    // fprintf(stderr, "header[0x1c]: %d\n", *(int*)&(header[0x1c]));
    // if ( *(int*)&(header[0x1C])!=32 ) {
    //     printf("Not a correct BMP file4\n");
    //     fclose(file);
    //     return;
    // }

    // Some BMP files are misformatted, guess missing information
    if (imageSize==0)    imageSize=(*width)* (*height)*4; // 4 : one byte for each Red, Green, Blue, Alpha component
    if (dataPos==0)      dataPos=54; // The BMP header is done that way

    // Create a buffer
    *data = new unsigned char [imageSize];

    if (dataPos != 54) {
        fread(header, 1, dataPos - 54, file);
    }

    // Read the actual data from the file into the buffer
    fread(*data,1,imageSize,file);

    // Everything is in memory now, the file can be closed.
    fclose (file);
}

class TexturedMesh {
public:
    GLuint vertexVBO, texCoordVBO, indexVBO, textureID, VAO, shaderProgram;
    std::vector<VertexData> vertices;
    std::vector<TriData> faces;

    TexturedMesh(const std::string& plyFilePath, const std::string& bmpFilePath) {
        // Read the PLY file
        readPLYFile(plyFilePath, vertices, faces);

        // Load the bitmap image
        unsigned char* imageData;
        unsigned int width, height;
        loadARGB_BMP(bmpFilePath.c_str(), &imageData, &width, &height);
	
        // Create and bind the VAO
        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);

        // Create and bind the VBO for vertex positions
        glGenBuffers(1, &vertexVBO);
        glBindBuffer(GL_ARRAY_BUFFER, vertexVBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(VertexData), &vertices[0], GL_STATIC_DRAW);

        // Position attribute
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)0);

        // Normal attribute
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offsetof(VertexData, normal));

        // Color attribute
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offsetof(VertexData, color));

        // Texture coordinates attribute
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offsetof(VertexData, uv));

        // Create and bind the VBO for face indices
        glGenBuffers(1, &indexVBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexVBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, faces.size() * sizeof(TriData), &faces[0], GL_STATIC_DRAW);

        // Create the Texture Object
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, imageData);
        glGenerateMipmap(GL_TEXTURE_2D);

        delete[] imageData;

        // Create and compile the vertex shader
		GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
		std::string VertexShaderCode = "\
			#version 440 core\n\
			// Input vertex data, different for all executions of this shader.\n\
			layout(location = 0) in vec3 pos;\n\
			layout(location = 1) in vec3 normal;\n\
			layout(location = 2) in vec3 color;\n\
			layout(location = 3) in vec2 uv;\n\
			// Output data ; will be interpolated for each fragment.\n\
			out vec2 uv_out;\n\
			// Values that stay constant for the whole mesh.\n\
			uniform mat4 MVP;\n\
			void main(){ \n\
				// Output position of the vertex, in clip space : MVP * position\n\
				gl_Position =  MVP * vec4(pos,1);\n\
				// The color will be interpolated to produce the color of each fragment\n\
				uv_out = uv;\n\
			}\n";
		const char* vertexShaderSource = VertexShaderCode.c_str();
		glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
		glCompileShader(vertexShader);

		// Create and compile the fragment shader
		GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		std::string FragmentShaderCode = "\
			#version 440 core\n\
			in vec2 uv_out; \n\
			uniform sampler2D tex;\n\
			out vec4 fragColor;\n\
			void main() {\n\
				fragColor = texture(tex, uv_out);\n\
			}\n";
		const char* fragmentShaderSource = FragmentShaderCode.c_str();
		glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
		glCompileShader(fragmentShader);

		// Create and link the shader program
		shaderProgram = glCreateProgram();
		glAttachShader(shaderProgram, vertexShader);
		glAttachShader(shaderProgram, fragmentShader);
		glLinkProgram(shaderProgram);

		// Delete the shaders as they're linked into our program now and no longer necessary
		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);

		// Unbind the VAO now that we've captured the state
		glBindVertexArray(0);
    }

    void draw(glm::mat4 MVP) {
		// Set the shader program
		glUseProgram(shaderProgram);
		glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureID);

		// Get the location of the "MVP" uniform variable
		GLuint MatrixID = glGetUniformLocation(shaderProgram, "MVP");

		// Pass the MVP matrix to the shader
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

		// Bind the VAO
		glBindVertexArray(VAO);

		// Draw the mesh
		glDrawElements(GL_TRIANGLES, faces.size() * 3, GL_UNSIGNED_INT, 0);

		// Unbind the VAO
		glBindVertexArray(0);
	}
};

//////////////////////////////////////////////////////////////////////////////
// Main
//////////////////////////////////////////////////////////////////////////////
int main()
{
	// Initialise GLFW
	if( !glfwInit() )
	{
		fprintf( stderr, "Failed to initialize GLFW\n" );
		getchar();
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);

	// Open a window and create its OpenGL context
	float screenW = 1400;
	float screenH = 900;
	window = glfwCreateWindow( screenW, screenH, "Exercise 1", NULL, NULL);
	if( window == NULL ){
		fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
		getchar();
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return -1;
	}

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	
	// Dark blue background
	glClearColor(0.2f, 0.2f, 0.3f, 0.0f);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	// Create a TexturedMesh object
	TexturedMesh floor("./LinksHouse/Floor.ply", "./LinksHouse/floor.bmp");
	TexturedMesh metalObjects("./LinksHouse/MetalObjects.ply", "./LinksHouse/metalobjects.bmp");
	TexturedMesh patio("./LinksHouse/Patio.ply", "./LinksHouse/patio.bmp");
	TexturedMesh table("./LinksHouse/Table.ply", "./LinksHouse/table.bmp");
	TexturedMesh walls("./LinksHouse/Walls.ply", "./LinksHouse/walls.bmp");
	TexturedMesh windowBG("./LinksHouse/WindowBG.ply", "./LinksHouse/windowbg.bmp");
	TexturedMesh woodObjects("./LinksHouse/WoodObjects.ply", "./LinksHouse/woodobjects.bmp");
	TexturedMesh bottles("./LinksHouse/Bottles.ply", "./LinksHouse/bottles.bmp");
	TexturedMesh curtains("./LinksHouse/Curtains.ply", "./LinksHouse/curtains.bmp");
	TexturedMesh doorBG("./LinksHouse/DoorBG.ply", "./LinksHouse/doorbg.bmp");

	// Camera position/direction
	glm::vec3 cameraPos = glm::vec3(0.5f, 0.4f, 0.5f);
	glm::vec3 cameraDir = glm::vec3(0.0f, 0.0f, -1.0f);

	do{
		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glMatrixMode(GL_PROJECTION);
		glPushMatrix();

		// Camera controls
		float cameraSpeed = 0.01f, rotationSpeed = 0.5f;
		if(glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
			cameraPos += cameraSpeed * cameraDir;
		if(glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
			cameraPos -= cameraSpeed * cameraDir;
		if(glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
		{
			// Rotate camera counter-clockwise
			glm::vec3 direction = glm::rotate(glm::mat4(1.0f), glm::radians(rotationSpeed), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(cameraDir, 0.0f);
			cameraDir = glm::normalize(direction);
		}
		if(glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
		{
			// Rotate camera clockwise
			glm::vec3 direction = glm::rotate(glm::mat4(1.0f), glm::radians(-rotationSpeed), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(cameraDir, 0.0f);
			cameraDir = glm::normalize(direction);
		}
		glm::mat4 Projection = glm::perspective(glm::radians(45.0f), screenW/screenH, 0.001f, 1000.0f);
		glm::vec3 up = {0.0f, 1.0f, 0.0f};
		glm::vec3 target = cameraPos + cameraDir;
		glm::mat4 V = glm::lookAt(cameraPos, target, up); 
		glm::mat4 M = glm::mat4(1.0f);
		glm::mat4 MVP = Projection * V * M;

		// Draw Scene
		glMatrixMode( GL_MODELVIEW );
		glPushMatrix();

		floor.draw(MVP);
		patio.draw(MVP);
		table.draw(MVP);
		walls.draw(MVP);
		windowBG.draw(MVP);
		woodObjects.draw(MVP);
		bottles.draw(MVP);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		doorBG.draw(MVP);
		curtains.draw(MVP);
		metalObjects.draw(MVP);
		glDisable(GL_BLEND);

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();


	} // Check if the ESC key was pressed or the window was closed
	while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
		glfwWindowShouldClose(window) == 0 );

	return 0;
}