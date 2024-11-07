
#include "GL/glew.h"
#include "GL/freeglut.h"
#include "GLFW/glfw3.h"
#include "LoadShaders.h"
#include <glm/glm.hpp> //includes GLM
#include "glm/ext/vector_float3.hpp"
#include "glm/fwd.hpp"
#include <glm/ext/matrix_transform.hpp> // GLM: translate, rotate
#include <glm/ext/matrix_clip_space.hpp> // GLM: perspective and ortho 
#include <glm/gtc/type_ptr.hpp> // GLM: access to the value_ptr
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <cmath>

#include <iostream>
#include<vector>

#include "packages/imGUI/imgui.h"
#include "packages/imGUI/imgui_impl_glfw_gl3.h"

float xPos = 0.0f, yPos = 0.0f, isOpen = 0.0f;
unsigned int cursorMax = 50, sensitivityCounter = 0, lineCount = 0, indiceCount = 0, positionsSize = 0, windowWidth = 1000, windowHeight = 800;
int sensitivity = 5, cursorScale = 10, change = 0;
bool drawing = false, outBounds = false;;

enum VAO_IDs { Cube, NumVAOs = 1 };
enum Buffer_IDs { Triangles, Colours, Normals, Textures, Indices, NumBuffers = 5 };

GLuint  VAOs[NumVAOs];
GLuint  Buffers[NumBuffers];

bool show_edit_window = true, line_normal = true, line_triangles = false;
ImVec4 clear_color = ImVec4(1.0f, 0.0f, 0.0f, 1.00f);

glm::vec3 position = glm::vec3(0.0f, 0.0f, -18.0f);
GLfloat allPoints[30000];
GLuint allIndices[30000];
GLfloat allTextures[30000];
GLfloat allColours[30000][4];
GLfloat allNormals[30000];

INT16 currentcolour = 0;
GLfloat palette[6][4] = { {1.0f, 0.0f, 0.0f, 1.0f}, //red
	{0.0f, 1.0f, 0.0f, 1.0f},
	{0.0f, 0.0f, 1.0f, 1.0f},
	{1.0f, 1.0f, 0.01, 1.0f},
	{1.0f, 0.0f, 1.0f, 1.0f},
	{0.0f, 1.0f, 1.0f, 1.0f} };

GLuint shader;
GLuint texture1;

unsigned int buffer;
unsigned int ibo;

unsigned int yVal = 0, xval = 0, zVal = 0;

#define BUFFER_OFFSET(a) ((void*)(a))

class lineShape
{
public:

	unsigned int size = 12;
	glm::vec3 centre = glm::vec3(0.0f, 0.0f, 0.0f);

	GLint curerntSquare[4]; //0=TL, 1=TR, 2=BR, 3=BL
	GLfloat positions[12];

	GLuint Indices[6] = { 0, 1, 2, 2, 3, 0 };
	GLfloat normals[12] = { -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f };
	GLfloat texture[8] = { 0.0f, 0.0f,
		1.0f, 0.0f,
		1.0f, 1.0f,
		0.0f, 1.0f };

	void extrude(double xpos, double ypos)
	{
		float transformX = static_cast<float>(xpos - centre.x);
		float transformY = static_cast<float>(ypos - centre.y);
		double gradient = transformY / transformX;

		double Pgradient = -1 / (transformY / transformX); //perpendicular gradient is -1 over the gradient from centre of pervious "square" to cursor

		float x1 = 0.0f, x2 = 0.0f, y1 = 0.0f, y2 = 0.0f;

		//Calculating new vector points
		if (transformY == 0)
		{
			//If no change in Y, then moving straight across X axis
			x1 = xPos + transformX;
			x2 = xPos + transformX;

			//Y values would just add and take half the scale
			y1 = static_cast<float>(yPos - sqrt((pow((float)cursorScale / 50, 2)) / 2));
			y2 = static_cast<float>(yPos + sqrt((pow((float)cursorScale / 50, 2)) / 2));
		}
		else if (transformX == 0)
		{
			//If no change in X, then moving straight across Y axis
			y1 = yPos + transformX;
			y2 = yPos + transformX;

			//X values would just add and take half the scale
			x1 = static_cast<float>(xPos - sqrt((pow((float)cursorScale / 50, 2)) / 2));
			x2 = static_cast<float>(xPos + sqrt((pow((float)cursorScale / 50, 2)) / 2));
		}
		else //if neither are 0, then calculate positions
		{
			//Calculating co-ordinates based on brush scale and gradient
	//scale^2 = Pgx ^2 + x^2  <- (y = Pgx)
	//x^2(Pg^2 + 1) = scale^2
	//(Pg ^ 2 + 1)x^2 + 0x - scale^2 = 0
			float c = -1 * pow((float)cursorScale / 100.0f, 2.0f);
			float a = static_cast<float>(pow(Pgradient, 2.0f) + 1);
			//Quadratic formula returns 2 values for X, upper and lower
			//Equations must be split, otherwise C++ sometimes doesn't calculate correctly
			x1 = sqrt(0.0f - 4.0f * a * c);
			x1 = x1 / (2.0f * a);
			x2 = sqrt(0.0f - 4.0f * a * c);
			x2 = 0.0f - x2;
			x2 = x2 / (2.0f * a);

			y1 = static_cast<float>(Pgradient) * x1;
			y2 = static_cast<float>(Pgradient) * x2;

			//Translate the new co-ordinates by the mouse location
			x1 += xPos;
			x2 += xPos;
			y1 += yPos;
			y2 += yPos;
		}

		centre.x = (x1 + x2) / 2.0f;
		centre.y = (y1 + y2) / 2.0f;

		//Add new traingles to array
		allPoints[0 + positionsSize] = x1; //x1
		allPoints[1 + positionsSize] = -0.5f;
		allPoints[2 + positionsSize] = y1; //y1

		allNormals[0 + positionsSize] = -1.0f;
		allNormals[1 + positionsSize] = 0.0f;
		allNormals[2 + positionsSize] = 0.0f;

		allPoints[3 + positionsSize] = x2; //x2
		allPoints[4 + positionsSize] = -0.5f;
		allPoints[5 + positionsSize] = y2; //y2

		allNormals[3 + positionsSize] = -1.0f;
		allNormals[4 + positionsSize] = 0.0f;
		allNormals[5 + positionsSize] = 0.0f;

		for (size_t i = 0; i < 4; i++)
		{
			allColours[positionsSize / 3][i] = palette[currentcolour][i];
			allColours[(positionsSize / 3) + 1][i] = palette[currentcolour][i];
		}

		//Logic for which vertecies are part of the end square, shifts verticies along and then replaces with new 2
		if (fabs(transformX) > fabs(transformY)) //Left or Right, fabs returns absolute (+'ve) value
		{
			if (transformX > 0 && transformY > 0) //right down
			{
				curerntSquare[0] = curerntSquare[1];
				curerntSquare[3] = curerntSquare[2];
				curerntSquare[1] = positionsSize / 3; //new pos
				curerntSquare[2] = 1 + (positionsSize / 3); //new pos
			}
			else if (transformX > 0 && transformY < 0) //right up
			{
				curerntSquare[0] = curerntSquare[1];
				curerntSquare[3] = curerntSquare[2];
				curerntSquare[2] = positionsSize / 3; //new pos
				curerntSquare[1] = 1 + (positionsSize / 3); //new pos
			}
			else if (transformX < 0 && transformY < 0) //left up
			{
				curerntSquare[1] = curerntSquare[0];
				curerntSquare[2] = curerntSquare[3];
				curerntSquare[0] = positionsSize / 3; //new pos
				curerntSquare[3] = 1 + (positionsSize / 3); //new pos
			}
			else if (transformX < 0 && transformY > 0) //left down
			{
				curerntSquare[1] = curerntSquare[0];
				curerntSquare[2] = curerntSquare[3];
				curerntSquare[3] = positionsSize / 3; //new pos
				curerntSquare[0] = 1 + (positionsSize / 3); //new pos
			}
		}
		else if (fabs(transformX) < fabs(transformY)) //Up or Down
		{
			if (transformY < 0 && transformX < 0) //Y is negative so do reverse
			{
				curerntSquare[0] = curerntSquare[3];
				curerntSquare[1] = curerntSquare[2];
				curerntSquare[3] = positionsSize / 3; //new pos
				curerntSquare[2] = 1 + (positionsSize / 3); //new pos
			}
			else if (transformY < 0 && transformX > 0) //Y is negative so do reverse
			{
				curerntSquare[0] = curerntSquare[3];
				curerntSquare[1] = curerntSquare[2];
				curerntSquare[3] = positionsSize / 3; //new pos
				curerntSquare[2] = 1 + (positionsSize / 3); //new pos
			}
			else if (transformY > 0 && transformX < 0)
			{
				curerntSquare[2] = curerntSquare[1];
				curerntSquare[3] = curerntSquare[0];
				curerntSquare[0] = positionsSize / 3; //new pos
				curerntSquare[1] = 1 + (positionsSize / 3); //new pos
			}
			else if (transformY > 0 && transformX > 0)
			{
				curerntSquare[2] = curerntSquare[1];
				curerntSquare[3] = curerntSquare[0];
				curerntSquare[0] = positionsSize / 3; //new pos
				curerntSquare[1] = 1 + (positionsSize / 3); //new pos
			}
		}

		allIndices[0 + indiceCount] = curerntSquare[0]; //0
		allIndices[1 + indiceCount] = curerntSquare[1]; //1
		allIndices[2 + indiceCount] = curerntSquare[2]; //2

		allIndices[3 + indiceCount] = curerntSquare[2]; //2
		allIndices[4 + indiceCount] = curerntSquare[3]; //3
		allIndices[5 + indiceCount] = curerntSquare[0]; //0

		size += 6;
		positionsSize += 6;
		indiceCount += 6;
	}

	void triangleLine(double xpos, double ypos) // A failed extrude implementation kept for its unique design
	{
		double Pgradient = -1 / ((ypos - centre.y) / (xpos - centre.x));

		float c = -1 * pow((float)cursorScale / 100.0f, 2.0f);
		float a = static_cast<float>(pow(Pgradient, 2.0f) + 1);

		float x1 = sqrt(0.0f - 4.0f * a * c);
		x1 = x1 / (2.0f * a);
		float x2 = sqrt(0.0f - 4.0f * a * c);
		x2 = 0.0f - x2;
		x2 = x2 / (2.0f * a);

		float y1 = static_cast<float>(Pgradient) * x1;
		float y2 = static_cast<float>(Pgradient) * x2;

		x1 += static_cast<float>(xpos);
		x2 += static_cast<float>(xpos);
		y1 += static_cast<float>(ypos);
		y2 += static_cast<float>(ypos);

		centre.x = (x1 + x2) / 2.0f;
		centre.y = (y1 + y2) / 2.0f;

		//Add new traingles to array
		allPoints[0 + positionsSize] = x1; //x1
		allPoints[1 + positionsSize] = -0.5f;
		allPoints[2 + positionsSize] = y1; //y1

		allNormals[0 + positionsSize] = -1.0f;
		allNormals[1 + positionsSize] = 0.0f;
		allNormals[2 + positionsSize] = 0.0f;

		allPoints[3 + positionsSize] = x2; //x2
		allPoints[4 + positionsSize] = -0.5f;
		allPoints[5 + positionsSize] = y2; //y2

		allNormals[3 + positionsSize] = -1.0f;
		allNormals[4 + positionsSize] = 0.0f;
		allNormals[5 + positionsSize] = 0.0f;

		allIndices[0 + indiceCount] = -2 + (positionsSize / 3); //0
		allIndices[1 + indiceCount] = -3 + (positionsSize / 3); //1
		allIndices[2 + indiceCount] = 0 + (positionsSize / 3); //2

		allIndices[3 + indiceCount] = 0 + (positionsSize / 3); //2
		allIndices[4 + indiceCount] = 1 + (positionsSize / 3); //3
		allIndices[5 + indiceCount] = -3 + (positionsSize / 3); //0

		for (size_t i = 0; i < 4; i++)
		{
			allColours[positionsSize / 3][i] = palette[currentcolour][i];
			allColours[(positionsSize / 3) + 1][i] = palette[currentcolour][i];
		}

		size += 6;
		positionsSize += 6;
		indiceCount += 6;
	}

	void start(double xpos, double ypos)
	{
		centre.x = static_cast<float>(xpos);
		centre.y = static_cast<float>(ypos);

		// Normalise offset based on cursor size
		float divergence = static_cast<float>(sqrt((pow((float)cursorScale / 50, 2)) / 2));

		// Calculate corners using offset
		positions[0] = { centre.x - divergence }; positions[1] = { -0.5f }; positions[2] = { centre.y - divergence };
		positions[3] = { centre.x + divergence }; positions[4] = { -0.5f }; positions[5] = { centre.y - divergence };
		positions[6] = { centre.x + divergence }; positions[7] = { -0.5f }; positions[8] = { centre.y + divergence };
		positions[9] = { centre.x - divergence }; positions[10] = { -0.5f }; positions[11] = { centre.y + divergence };

		// Update positions for all 6 points (2 old, 2 new, 2 re-used)
		for (unsigned int i = 0; i < 12; i++)
		{
			allPoints[i + positionsSize] = positions[i];
			allNormals[i + positionsSize] = normals[i];

			if (i < 6)
			{
				allIndices[i + indiceCount] = Indices[i] + (positionsSize / 3);

				if (i < 4)
				{
					for (unsigned int x = 0; x < 4; x++)
					{
						allColours[i + (positionsSize / 3)][x] = palette[currentcolour][x];
					}
					curerntSquare[i] = (positionsSize / 3) + i;
				}
			}
		}

		indiceCount += 6;
		positionsSize += 12;
	}
};

void
init(void)
{
	glGenVertexArrays(NumVAOs, VAOs);
	glBindVertexArray(VAOs[Cube]);

	ShaderInfo  shaders[] =
	{
		{ GL_VERTEX_SHADER, "media/triangles.vert" },
		{ GL_FRAGMENT_SHADER, "media/triangles.frag" },
		{ GL_NONE, NULL }
	};

	shader = LoadShaders(shaders);
	glUseProgram(shader);

	// ambient light
	glm::vec4 ambient = glm::vec4(0.8f, 0.8f, 0.8f, 1.0f);
	//adding the Uniform to the shader
	GLuint aLoc = glGetUniformLocation(shader, "ambient");
	glUniform4fv(aLoc, 1, glm::value_ptr(ambient));

	// light object
	glm::vec3 lightPos = glm::vec3(0.0f, 500.0f, -25.0f);
	GLuint dLightPosLoc = glGetUniformLocation(shader, "lightPos");
	glUniform3fv(dLightPosLoc, 1, glm::value_ptr(lightPos));


	// diffuse light
	glm::vec3 diffuseLight = glm::vec3(0.5f, 0.2f, 0.7f);
	GLuint dLightLoc = glGetUniformLocation(shader, "dLight");
	glUniform3fv(dLightLoc, 1, glm::value_ptr(diffuseLight));

	// specular light
	glm::vec3 specularLight = glm::vec3(0.7f);
	GLfloat shininess = 256; //128 is max value
	GLuint sLightLoc = glGetUniformLocation(shader, "sLight");
	GLuint sShineLoc = glGetUniformLocation(shader, "sShine");
	glUniform3fv(sLightLoc, 1, glm::value_ptr(specularLight));
	glUniform1fv(sShineLoc, 1, &shininess);

	//Add background Square & Cursor
	allPoints[0] = { -10.0f }; allPoints[1] = { -0.5f }; allPoints[2] = { -10.0f };
	allPoints[3] = { 10.0f }; allPoints[4] = { -0.5f }; allPoints[5] = { -10.0f };
	allPoints[6] = { 10.0f }; allPoints[7] = { -0.5f }; allPoints[8] = { 10.0f };
	allPoints[9] = { -10.0f }; allPoints[10] = { -0.5f }; allPoints[11] = { 10.0f };

	allPoints[12] = { -0.1f }; allPoints[13] = { -0.5f }; allPoints[14] = { -0.1f };
	allPoints[15] = { 0.1f }; allPoints[16] = { -0.5f }; allPoints[17] = { -0.1f };
	allPoints[18] = { 0.1f }; allPoints[19] = { -0.5f }; allPoints[20] = { 0.1f };
	allPoints[21] = { -0.1f }; allPoints[22] = { -0.5f }; allPoints[23] = { 0.1f };

	allNormals[0] = { -1.0f }; allNormals[1] = { 0.0f }; allNormals[2] = { 0.0f };
	allNormals[3] = { -1.0f }; allNormals[4] = { 0.0f }; allNormals[5] = { 0.0f };
	allNormals[6] = { -1.0f }; allNormals[7] = { 0.0f }; allNormals[8] = { 0.0f };
	allNormals[9] = { -1.0f }; allNormals[10] = { 0.0f }; allNormals[11] = { 0.0f };
	allNormals[12] = { -1.0f }; allNormals[13] = { 0.0f }; allNormals[14] = { 0.0f };
	allNormals[15] = { -1.0f }; allNormals[16] = { 0.0f }; allNormals[17] = { 0.0f };
	allNormals[18] = { -1.0f }; allNormals[19] = { 0.0f }; allNormals[20] = { 0.0f };
	allNormals[21] = { -1.0f }; allNormals[22] = { 0.0f }; allNormals[23] = { 0.0f };

	allIndices[0] = { 0 }; allIndices[1] = { 1 }; allIndices[2] = { 2 };
	allIndices[3] = { 2 }; allIndices[4] = { 3 }; allIndices[5] = { 0 };

	allIndices[6] = { 4 }; allIndices[7] = { 5 }; allIndices[8] = { 6 };
	allIndices[9] = { 6 }; allIndices[10] = { 7 }; allIndices[11] = { 4 };

	for (unsigned int i = 0; i < 4; i++)
	{
		allColours[i][0] = { 1.0f };
		allColours[i][1] = { 1.0f };
		allColours[i][2] = { 1.0f };
		allColours[i][3] = { 1.0f };
	}

	for (unsigned int i = 4; i < 8; i++)
	{
		allColours[i][0] = palette[currentcolour][0];
		allColours[i][1] = palette[currentcolour][1];;
		allColours[i][2] = palette[currentcolour][2];
		allColours[i][3] = palette[currentcolour][3];
	}

	allTextures[0] = { 0.0f }; allTextures[1] = { 0.0f };
	allTextures[2] = { 1.0f }; allTextures[3] = { 0.0f };
	allTextures[4] = { 1.0f }; allTextures[5] = { 1.0f };
	allTextures[6] = { 0.0f }; allTextures[7] = { 1.0f };

	allTextures[8] = { 0.0f }; allTextures[9] = { 0.0f };
	allTextures[10] = { 1.0f }; allTextures[11] = { 0.0f };
	allTextures[12] = { 1.0f }; allTextures[13] = { 1.0f };
	allTextures[14] = { 0.0f }; allTextures[15] = { 1.0f };

	indiceCount = 12;
	positionsSize = 24;
	
}

void Display()
{
	glGenBuffers(NumBuffers, Buffers);

	glBindBuffer(GL_ARRAY_BUFFER, Buffers[Triangles]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(allPoints), allPoints, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Buffers[Indices]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,sizeof(allIndices), allIndices, GL_STATIC_DRAW);

	glVertexAttribPointer(Triangles, 3, GL_FLOAT,
		GL_FALSE, 0, BUFFER_OFFSET(0));

	//Colour Binding
	glBindBuffer(GL_ARRAY_BUFFER, Buffers[Colours]);
	glBufferStorage(GL_ARRAY_BUFFER, sizeof(allColours), allColours, 0);

	glVertexAttribPointer(Colours, 4, GL_FLOAT,
		GL_FALSE, 0, BUFFER_OFFSET(0));


	//Normal Binding
	glBindBuffer(GL_ARRAY_BUFFER, Buffers[Normals]);
	glBufferStorage(GL_ARRAY_BUFFER, sizeof(allNormals), allNormals, 0);


	glVertexAttribPointer(Normals, 3, GL_FLOAT,
		GL_FALSE, 0, BUFFER_OFFSET(0));

	//Texture Binding
	glBindBuffer(GL_ARRAY_BUFFER, Buffers[Textures]);
	glBufferData(GL_ARRAY_BUFFER,sizeof(allTextures), allTextures, GL_STATIC_DRAW);

	glVertexAttribPointer(Textures, 2, GL_FLOAT,
		GL_FALSE, 0, BUFFER_OFFSET(0));

	// load and create a texture
	glGenTextures(1, &texture1);
	glBindTexture(GL_TEXTURE_2D, texture1);
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// create texture and generate mipmaps
	glGenerateMipmap(GL_TEXTURE_2D);
	glUniform1i(glGetUniformLocation(shader, "texture1"), 0);

	glEnableVertexAttribArray(Triangles);
	glEnableVertexAttribArray(Colours);
	glEnableVertexAttribArray(Textures);
	glEnableVertexAttribArray(Normals);

	static const float black[] = { 0.0f, 0.0f, 0.0f, 0.0f };

	glClearBufferfv(GL_COLOR, 0, black);
	glClear(GL_COLOR_BUFFER_BIT);

	// bind textures on corresponding texture units
	glFrontFace(GL_CW);
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);

	// creating the model matrix
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::rotate(model, glm::radians(45 * 2.0f), glm::vec3(90.0f, 0.0f, 0.0f)); //from flat on floor to facing camera


	// creating the view matrix
	glm::mat4 view = glm::mat4(1.0f);
	view = glm::translate(view, glm::vec3(position));

	// creating the projection matrix
	glm::mat4 projection = glm::perspective(45.0f, 4.0f / 3, 0.1f, 20.0f);

	// Adding all matrices up to create combined matrix
	glm::mat4 mv = view * model;


	//adding the Uniform to the shader
	int mvLoc = glGetUniformLocation(shader, "mv_matrix");
	glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mv));
	//adding the Uniform to the shader
	int pLoc = glGetUniformLocation(shader, "p_matrix");
	glUniformMatrix4fv(pLoc, 1, GL_FALSE, glm::value_ptr(projection));

	glBindVertexArray(VAOs[Cube]);
	glBindTexture(GL_TEXTURE_2D, texture1);
	glDrawElements(GL_TRIANGLES, indiceCount, GL_UNSIGNED_INT, 0);
}

void lineManager()
{
	if (!line_normal && !line_triangles || line_normal && line_triangles)
	{
		if (change == 0)
		{
			line_normal = false;
			line_triangles = true;
			change = 1;
		}
		else
		{
			line_normal = true;
			line_triangles = false;
			change = 0;
		}
	}
}

static void cursorPositionCallBack(GLFWwindow *window, double Xpos, double Ypos);

void mouseButtonCallBack(GLFWwindow* window, int button, int action, int mods);

void scrollCallBack(GLFWwindow* window, double Xoffset, double Yoffset);

void generateCursor();


lineShape lines[100];
int
main(int argc, char** argv)
{
	glfwInit();

	GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, "C-Draw", NULL, NULL);
	glfwSetInputMode(window, GLFW_STICKY_MOUSE_BUTTONS, 1);

#pragma region MouseCallBacks
	glfwSetCursorPosCallback(window, cursorPositionCallBack);
	glfwSetMouseButtonCallback(window, mouseButtonCallBack);
	glfwSetScrollCallback(window, scrollCallBack);
#pragma endregion
	
	ImGui::CreateContext();
	ImGui_ImplGlfwGL3_Init(window, false);
	ImGui::StyleColorsDark();

	glfwMakeContextCurrent(window);
	glewInit();

	init();

	while (!glfwWindowShouldClose(window))
	{
#pragma region GUI
		ImGui_ImplGlfwGL3_NewFrame();
		{
			ImGui::Text("Close this Panel to Resume Drawing");
			ImGui::Text("");
			ImGui::Text("Edit Brush Width:");
			ImGui::SliderInt("Width", &cursorScale, 2.0f, cursorMax);
			ImGui::Text("");
			ImGui::Text("Edit Current Brush Colour");
			ImGui::ColorEdit3("Brush Color", (float*)&clear_color);
			ImGui::Text("");
			ImGui::Text("Edit Brush Sensitivity (frequency of draws)");
			ImGui::SliderInt("Sensitivity", &sensitivity, 1.0f, 5.0f);
			ImGui::Text("");
			ImGui::Checkbox("Normal Lines", &line_normal);
			ImGui::SameLine();
			ImGui::Checkbox("Triangle Lines (*experimental)", &line_triangles);
		}
		isOpen =  ImGui::GetCursorPos().y; //255 = open, 27 = closed

		if (isOpen == 255.0f)
		{
			palette[currentcolour][0] = clear_color.x;
			palette[currentcolour][1] = clear_color.y;
			palette[currentcolour][2] = clear_color.z;
			//Not using Alpha for now
		}
#pragma endregion

		lineManager();
		Display();

		ImGui::Render();
		ImGui_ImplGlfwGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	ImGui_ImplGlfwGL3_Shutdown();
	glfwTerminate();
	glfwTerminate();
}

void generateCursor()
{
	outBounds = false;

	// Normalise divergance to be +'ve
	float divergence = static_cast<float>(sqrt((pow((float)cursorScale / 50, 2)) / 2));

	if (xPos + divergence > 10.0f) { xPos = 10 - divergence, outBounds = true; }
	if (xPos - divergence < -10.0f) { xPos = -(10 - divergence), outBounds = true; }
	if (yPos + divergence > 10.0f) { yPos = 10 - divergence, outBounds = true; }
	if (yPos - divergence < -10.0f) { yPos = -(10 - divergence), outBounds = true; }

	//update cursor position
	allPoints[12] = { xPos - divergence }; allPoints[14] = { yPos - divergence };
	allPoints[15] = { xPos + divergence }; allPoints[17] = { yPos - divergence };
	allPoints[18] = { xPos + divergence }; allPoints[20] = { yPos + divergence };
	allPoints[21] = { xPos - divergence }; allPoints[23] = { yPos + divergence };

	//set cursor colour = selected colour
	for (unsigned int i = 4; i < 8; i++)
	{
		allColours[i][0] = palette[currentcolour][0];
		allColours[i][1] = palette[currentcolour][1];;
		allColours[i][2] = palette[currentcolour][2];
		allColours[i][3] = palette[currentcolour][3];
	}
}

static void cursorPositionCallBack(GLFWwindow* window, double Xpos, double Ypos)
{
	xPos = static_cast<float>((Xpos - (windowWidth / 2)) * (13.6915 / (windowWidth / 2))); //13.6915 is generated by scaling the square on screen to the actual screen size
	yPos = static_cast<float>((Ypos - (windowHeight / 2)) * (10.2597 / (windowHeight / 2)));

	generateCursor();
	if (drawing && isOpen == 27.0f && !outBounds)
	{
		
		sensitivityCounter++;
		if (sensitivityCounter == (6 - sensitivity)) //higher sensitivity == more frequent calls, range of 1-5
		{
			// only called when the mouse moves, so can always run "extrude" here
			if (line_normal) { lines[lineCount - 1].extrude(xPos, yPos); }
			else { lines[lineCount - 1].triangleLine(xPos, yPos); }

			sensitivityCounter = 0;
		}
	}
}

void determineBounds(bool state) {
	outBounds = state;
}

void mouseButtonCallBack(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS && isOpen == 27.0f && !outBounds)
	{
		drawing = true;

		lines[lineCount].start(xPos, yPos);
		lineCount++;
	}
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE || outBounds)
	{
		drawing = false;
	}

	sensitivityCounter = 0;

	//cycle colour with R click
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS && isOpen == 27.0f)
	{
		if (currentcolour < 6) { currentcolour++; }
		else { currentcolour = 0; }//cycle back to start
	}
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
	{
	}
}

void scrollCallBack(GLFWwindow* window, double Xoffset, double Yoffset) //x is 0 for normal scroll wheel
{
	if (cursorScale + (int)Yoffset < 2.0f) //restrict by Min & Max
	{
		cursorScale = 2;
		return;
	}
	
	if (cursorScale + (int)Yoffset > cursorMax+0.1f)
	{
		cursorScale = (int)cursorMax + 0;
		return;
	}

	cursorScale += static_cast<int>(Yoffset);

	generateCursor();
}