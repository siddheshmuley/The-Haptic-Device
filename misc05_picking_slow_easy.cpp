// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <array>
#include <stack>   
#include <sstream>
// Include GLEW
#include <GL/glew.h>
// Include GLFW
#include <glfw3.h>
// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
using namespace glm;
// Include AntTweakBar
#include <AntTweakBar.h>

#include <common/shader.hpp>
#include <common/controls.hpp>
#include <common/objloader.hpp>
#include <common/vboindexer.hpp>

const int window_width = 1024, window_height = 768;

bool CameraMovement = false, BodyMovement = false, TopRotation = false;
bool ArmOneRotation = false, ArmTwoRotation = false, PenRotation = false;
bool Jump = false;
float theta = 45 * 0.0175, gamma = 45 * 0.0175;
typedef struct Vertex {
	float Position[4];
	float Color[4];
	float Normal[3];
	void SetPosition(float *coords) {
		Position[0] = coords[0];
		Position[1] = coords[1];
		Position[2] = coords[2];
		Position[3] = 1.0;
	}
	void SetColor(float *color) {
		Color[0] = color[0];
		Color[1] = color[1];
		Color[2] = color[2];
		Color[3] = color[3];
	}
	void SetNormal(float *coords) {
		Normal[0] = coords[0];
		Normal[1] = coords[1];
		Normal[2] = coords[2];
	}
};

// function prototypes
int initWindow(void);
void initOpenGL(void);
void loadObject(char*, glm::vec4, Vertex * &, GLushort* &, int);
void createVAOs(Vertex[], GLushort[], int);
void createObjects(void);
void pickObject(void);
void renderScene(void);
void cleanup(void);
void moveCamera(int);
static void keyCallback(GLFWwindow*, int, int, int, int);
static void mouseCallback(GLFWwindow*, int, int, int);

// GLOBAL VARIABLES
GLFWwindow* window;

glm::mat4 gProjectionMatrix;
glm::mat4 gViewMatrix;

GLuint gPickedIndex = -1;
std::string gMessage;

GLuint programID;
GLuint pickingProgramID;

const GLuint NumObjects = 10;	// ATTN: THIS NEEDS TO CHANGE AS YOU ADD NEW OBJECTS
GLuint VertexArrayId[NumObjects] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
GLuint VertexBufferId[NumObjects] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
GLuint IndexBufferId[NumObjects] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

size_t NumIndices[NumObjects] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
size_t VertexBufferSize[NumObjects] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
size_t IndexBufferSize[NumObjects] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

GLuint MatrixID;
GLuint ModelMatrixID;
GLuint ViewMatrixID;
GLuint ProjMatrixID;
GLuint PickingMatrixID;
GLuint pickingColorID;
GLuint LightID, LightID2;
GLuint ambientID;
Vertex *BaseVertices, *TopVertices, *ArmOneVertices, *JointVertices, *ArmTwoVertices, *PenVertices, *ButtonVertices, *BulletVertices;
GLushort *BaseIndices, *TopIndices, *ArmOneIndices, *JointIndices, *ArmTwoIndices, *PenIndices, *ButtonIndices, *BulletIndices;
GLint gX = 0.0;
GLint gZ = 0.0;

//light
glm::vec3 lightPos = glm::vec3(10 * cos(0.785) * cos(0.885), 10 * sin(0.785), 10 * cos(0.785) * sin(0.885));
glm::vec3 lightPos2 = glm::vec3(10 * cos(0.785) * cos(0.685), 10 * sin(0.785), 10 * cos(0.785) * sin(0.685));

//haptic device movement
float xShift = 0.0f, zShift = 0.0f;
bool baseChange = false;

//top rotation
float topAngle = 0.0f;
bool topChange = false;

//arm1 angle
float armOneAngle = 0.0f;
float armOneBottomEnd = 2.19f;
bool armOneChange = false;

//arm2 angle
float armTwoAngle = 0.0f;
float armTwoTopEndY = 2.01f, armTwoTopEndX = 2.95f, armTwoTopEndZ = 0.0f;
bool armTwoChange = false;

//pen angle
float penHorizontal = 0.0f, penVertical = 0.0f, penAxial = 0.0f;
float penJointX = 2.95f, penJointZ = 0.0f, penJointY = 0.34f;
bool ShiftPressed = false;
bool penChange = false;

//teleport
float teleportX, teleportZ;
float yPosition = 0.34f;
float projectileX = 4.41f, projectileY = 0.34f, projectileZ = 0.0f;
float topHeight = 2.41f, armOneLength = 2.95f, armTwoLength = 1.67f, penLength = 1.47f;
float yOfProjectile = 0.34f;
float angleOfProjectile = 0.0f;
float g = 9.8f;
float time = 0.0f;
float velocity = 4.0f;
float ttt = 1;
vec4 center;
vec4 penCenter, projectileCenter;
float xdiff = 0.0f, zdiff = 0.0f, ydiff = 0.0f, horizontal = 0.0f, vertical = 0.0f;
float v1, v2, posx, posy, posz;
bool abandonShip = false;

// animation control
bool animation = false;
GLfloat phi = 0.0;

void loadObject(char* file, glm::vec4 color, Vertex * &out_Vertices, GLushort* &out_Indices, int ObjectId)
{
	// Read our .obj file
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> normals;
	bool res = loadOBJ(file, vertices, normals);

	std::vector<GLushort> indices;
	std::vector<glm::vec3> indexed_vertices;
	std::vector<glm::vec2> indexed_uvs;
	std::vector<glm::vec3> indexed_normals;
	indexVBO(vertices, normals, indices, indexed_vertices, indexed_normals);

	const size_t vertCount = indexed_vertices.size();
	const size_t idxCount = indices.size();

	// populate output arrays
	out_Vertices = new Vertex[vertCount];
	for (int i = 0; i < vertCount; i++) {
		out_Vertices[i].SetPosition(&indexed_vertices[i].x);
		out_Vertices[i].SetNormal(&indexed_normals[i].x);
		out_Vertices[i].SetColor(&color[0]);
	}
	out_Indices = new GLushort[idxCount];
	for (int i = 0; i < idxCount; i++) {
		out_Indices[i] = indices[i];
	}

	// set global variables!!
	NumIndices[ObjectId] = idxCount;
	VertexBufferSize[ObjectId] = sizeof(out_Vertices[0]) * vertCount;
	IndexBufferSize[ObjectId] = sizeof(GLushort) * idxCount;
}


void createObjects(void)
{
	//-- COORDINATE AXES --//
	Vertex CoordVerts[] =
	{
		{ { 0.0, 0.0, 0.0, 1.0 }, { 1.0, 0.0, 0.0, 1.0 }, { 0.0, 0.0, 1.0 } },
		{ { 5.0, 0.0, 0.0, 1.0 }, { 1.0, 0.0, 0.0, 1.0 }, { 0.0, 0.0, 1.0 } },
		{ { 0.0, 0.0, 0.0, 1.0 }, { 0.0, 1.0, 0.0, 1.0 }, { 0.0, 0.0, 1.0 } },
		{ { 0.0, 5.0, 0.0, 1.0 }, { 0.0, 1.0, 0.0, 1.0 }, { 0.0, 0.0, 1.0 } },
		{ { 0.0, 0.0, 0.0, 1.0 }, { 0.0, 0.0, 1.0, 1.0 }, { 0.0, 0.0, 1.0 } },
		{ { 0.0, 0.0, 5.0, 1.0 }, { 0.0, 0.0, 1.0, 1.0 }, { 0.0, 0.0, 1.0 } },
	};
	Vertex GridVertices[44];
	VertexBufferSize[0] = sizeof(CoordVerts);	// ATTN: this needs to be done for each hand-made object with the ObjectID (subscript)
	createVAOs(CoordVerts, NULL, 0);
	
	//-- GRID --//
	
	// ATTN: create your grid vertices here!
	float pos = 5.0;
	float colorWhite[] = { 1.0, 1.0, 1.0, 1.0 };
	float gridNormal[] = { 0.0, 0.0, 1.0 };
	for (int i = 0; i < 11; i++)
	{
		GridVertices[2 * i].Position[0] = pos;
		GridVertices[2 * i].Position[1] = 0.0;
		GridVertices[2 * i].Position[2] = -5.0;
		GridVertices[2 * i].Position[3] = 1.0;
		GridVertices[2 * i].SetColor(colorWhite);
		GridVertices[2 * i].SetNormal(gridNormal);

		GridVertices[2 * i + 1].Position[0] = pos;
		GridVertices[2 * i + 1].Position[1] = 0.0;
		GridVertices[2 * i + 1].Position[2] = 5.0;
		GridVertices[2 * i + 1].Position[3] = 1.0;
		GridVertices[2 * i + 1].SetColor(colorWhite);
		GridVertices[2 * i + 1].SetNormal(gridNormal);

		GridVertices[22 + (2 * i)].Position[0] = -5.0;
		GridVertices[22 + (2 * i)].Position[1] = 0.0;
		GridVertices[22 + (2 * i)].Position[2] = pos;
		GridVertices[22 + (2 * i)].Position[3] = 1.0;
		GridVertices[22 + (2 * i)].SetColor(colorWhite);
		GridVertices[22 + (2 * i)].SetNormal(gridNormal);
					 
		GridVertices[22 + (2 * i + 1)].Position[0] = 5.0;
		GridVertices[22 + (2 * i + 1)].Position[1] = 0.0;
		GridVertices[22 + (2 * i + 1)].Position[2] = pos;
		GridVertices[22 + (2 * i + 1)].Position[3] = 1.0;
		GridVertices[22 + (2 * i + 1)].SetColor(colorWhite);
		GridVertices[22 + (2 * i + 1)].SetNormal(gridNormal);
		pos--;
	}
	//-- .OBJs --//
	VertexBufferSize[1] = sizeof(GridVertices);
	createVAOs(GridVertices, NULL, 1);
	// ATTN: load your models here
	//Vertex* Verts;
	//GLushort* Idcs;
	//loadObject("models/base.obj", glm::vec4(1.0, 0.0, 0.0, 1.0), Verts, Idcs, ObjectID);
	//createVAOs(Verts, Idcs, ObjectID);
	loadObject("Base.obj", glm::vec4(0.8, 0.0, 0.0, 1.0), BaseVertices, BaseIndices, 2);
	createVAOs(BaseVertices, BaseIndices, 2);
	loadObject("Top.obj", glm::vec4(0.0, 0.8, 0.0, 1.0), TopVertices, TopIndices, 3);
	createVAOs(TopVertices, TopIndices, 3);
	loadObject("Arm1.obj", glm::vec4(0.0, 0.0, 0.8, 1.0), ArmOneVertices, ArmOneIndices, 4);
	createVAOs(ArmOneVertices, ArmOneIndices, 4);
	loadObject("Joint.obj", glm::vec4(0.8, 0.0, 0.8, 1.0), JointVertices, JointIndices, 5);
	createVAOs(JointVertices, JointIndices, 5);
	loadObject("Arm2.obj", glm::vec4(0.0, 0.8, 0.8, 1.0), ArmTwoVertices, ArmTwoIndices, 6);
	createVAOs(ArmTwoVertices, ArmTwoIndices, 6);
	loadObject("Pen.obj", glm::vec4(0.8, 0.8, 0.0, 1.0), PenVertices, PenIndices, 7);
	createVAOs(PenVertices, PenIndices, 7);
	loadObject("Button.obj", glm::vec4(0.8, 0.8, 0.8, 1.0), ButtonVertices, ButtonIndices, 8);
	createVAOs(ButtonVertices, ButtonIndices, 8);
	loadObject("Bullet.obj", glm::vec4(1.0, 1.0, 1.0, 1.0), BulletVertices, BulletIndices, 9);
	createVAOs(BulletVertices, BulletIndices, 9);
}

void renderScene(void)
{
	//ATTN: DRAW YOUR SCENE HERE. MODIFY/ADAPT WHERE NECESSARY!
	float deltaA1, deltaA2, deltaPenH, deltaPenV;

	// Dark blue background
	glClearColor(0.0f, 0.0f, 0.2f, 0.0f);
	// Re-clear the screen for real rendering
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(programID);
	{
		glm::mat4x4 ModelMatrix = glm::mat4(1.0);
		glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);
		glUniform3f(LightID2, lightPos2.x, lightPos2.y, lightPos2.z);
		glUniform3f(ambientID, 1, 1, 1);
		glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &gViewMatrix[0][0]);
		glUniformMatrix4fv(ProjMatrixID, 1, GL_FALSE, &gProjectionMatrix[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);

		glBindVertexArray(VertexArrayId[0]);	// draw CoordAxes
		glDrawArrays(GL_LINES, 0, 6);
		glBindVertexArray(0);

		glBindVertexArray(VertexArrayId[1]);	// draw CoordAxes
		glDrawArrays(GL_LINES, 0, 44);
		glBindVertexArray(0);
		if (BodyMovement) {
			if (baseChange) {
				loadObject("Base.obj", glm::vec4(1.0, 0.0, 0.0, 1.0), BaseVertices, BaseIndices, 2);
				createVAOs(BaseVertices, BaseIndices, 2);
				baseChange = false;
			}
			if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
				xShift = xShift - 0.1f;
			}
			else if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
			{
				zShift = zShift - 0.1f;
			}
			else if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
			{
				xShift = xShift + 0.1f;
			}
			else if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
			{
				zShift = zShift + 0.1f;
			}
		}
		else {
			if (baseChange) {
				loadObject("Base.obj", glm::vec4(0.8, 0.0, 0.0, 1.0), BaseVertices, BaseIndices, 2);
				createVAOs(BaseVertices, BaseIndices, 2);
				baseChange = false;
			}
		}
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(xShift, 0.0f, zShift));
		//ModelMatrix = glm::translate(mat4(1.0), glm::vec3(xShift, 0.0f, zShift))*ModelMatrix;
		glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);
		glUniform3f(LightID2, lightPos2.x, lightPos2.y, lightPos2.z);
		glUniform3f(ambientID, 0.2f, 0.2f, 0.2f);
		glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &gViewMatrix[0][0]);
		glUniformMatrix4fv(ProjMatrixID, 1, GL_FALSE, &gProjectionMatrix[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);

		glBindVertexArray(VertexArrayId[2]);
		glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[2]);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(BaseVertices), BaseVertices);
		glDrawElements(GL_TRIANGLES, VertexBufferSize[2], GL_UNSIGNED_SHORT, (void*)0);
		glBindVertexArray(0);
		//to rotate the top
		if (TopRotation) {
			if (topChange) {
				loadObject("Top.obj", glm::vec4(0.0, 1.0, 0.0, 1.0), TopVertices, TopIndices, 3);
				createVAOs(TopVertices, TopIndices, 3);
				topChange = false;
			}
			if (topAngle > 6.28) {
				topAngle = topAngle - 6.28;
			}
			else if(topAngle < 0.0f){
				topAngle = 6.28 + topAngle;
			}
			if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
			{
				topAngle = topAngle - 0.1f;
			}
			else if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
			{
				topAngle = topAngle + 0.1f;
			}
		}
		else {
			if (topChange) {
				loadObject("Top.obj", glm::vec4(0.0, 0.8, 0.0, 1.0), TopVertices, TopIndices, 3);
				createVAOs(TopVertices, TopIndices, 3);
				topChange = false;
			}
		}
		ModelMatrix = glm::rotate(ModelMatrix, topAngle, glm::vec3(0.0f, 1.0f, 0.0f));
		glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);
		glUniform3f(LightID2, lightPos2.x, lightPos2.y, lightPos2.z);
		glUniform3f(ambientID, 0.2f, 0.2f, 0.2f);
		glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &gViewMatrix[0][0]);
		glUniformMatrix4fv(ProjMatrixID, 1, GL_FALSE, &gProjectionMatrix[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);

		glBindVertexArray(VertexArrayId[3]);
		glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[3]);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(TopVertices), TopVertices);
		glDrawElements(GL_TRIANGLES, VertexBufferSize[3], GL_UNSIGNED_SHORT, (void*)0);
		glBindVertexArray(0);

		//arm1 movement
		if (ArmOneRotation) {
			if (armOneChange) {
				loadObject("Arm1.obj", glm::vec4(0.0, 0.0, 1.0, 1.0), ArmOneVertices, ArmOneIndices, 4);
				createVAOs(ArmOneVertices, ArmOneIndices, 4);
				armOneChange = false;
			}
			if (armOneAngle > 6.28) {
				armOneAngle = armOneAngle - 6.28;
			}
			else if (armOneAngle < 0.0f) {
				armOneAngle = 6.28 + armOneAngle;
			}
			if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
			{
				deltaA1 = armOneAngle;
				armOneAngle = armOneAngle + 0.1f;
				deltaA1 = armOneAngle - deltaA1;
			}
			else if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
			{
				deltaA1 = armOneAngle;
				armOneAngle = armOneAngle - 0.1f;
				deltaA1 = deltaA1 - armOneAngle;
			}
		}
		else {
			if (armOneChange) {
				loadObject("Arm1.obj", glm::vec4(0.0, 0.0, 0.8, 1.0), ArmOneVertices, ArmOneIndices, 4);
				createVAOs(ArmOneVertices, ArmOneIndices, 4);
				armOneChange = false;
			}
		}
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(0.0f, armOneBottomEnd, 0.0f));
		ModelMatrix = glm::rotate(ModelMatrix, armOneAngle, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(0.0f, -armOneBottomEnd, 0.0f));
		glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);
		glUniform3f(LightID2, lightPos2.x, lightPos2.y, lightPos2.z);
		glUniform3f(ambientID, 0.2f, 0.2f, 0.2f);
		glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &gViewMatrix[0][0]);
		glUniformMatrix4fv(ProjMatrixID, 1, GL_FALSE, &gProjectionMatrix[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);

		glBindVertexArray(VertexArrayId[4]);
		glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[4]);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(ArmOneVertices), ArmOneVertices);
		glDrawElements(GL_TRIANGLES, VertexBufferSize[4], GL_UNSIGNED_SHORT, (void*)0);
		glBindVertexArray(0);
		glBindVertexArray(VertexArrayId[5]);
		glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[5]);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(JointVertices), JointVertices);
		glDrawElements(GL_TRIANGLES, VertexBufferSize[5], GL_UNSIGNED_SHORT, (void*)0);
		glBindVertexArray(0);

		//arm2 movement
		if (ArmTwoRotation) {
			if (armTwoChange) {
				loadObject("Arm2.obj", glm::vec4(0.0, 1.0, 1.0, 1.0), ArmTwoVertices, ArmTwoIndices, 6);
				createVAOs(ArmTwoVertices, ArmTwoIndices, 6);
				armTwoChange = false;
			}
			if (armTwoAngle > 6.28) {
				armTwoAngle = armTwoAngle - 6.28;
			}
			else if (armTwoAngle < 0.0f) {
				armTwoAngle = 6.28 + armTwoAngle;
			}
			if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
			{
				armTwoAngle = armTwoAngle + 0.1f;
			}
			else if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
			{
				armTwoAngle = armTwoAngle - 0.1f;
			}
		}
		else {
			if (armTwoChange) {
				loadObject("Arm2.obj", glm::vec4(0.0, 0.8, 0.8, 1.0), ArmTwoVertices, ArmTwoIndices, 6);
				createVAOs(ArmTwoVertices, ArmTwoIndices, 6);
				armTwoChange = false;
			}
		}
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(armTwoTopEndX, armTwoTopEndY, armTwoTopEndZ));
		ModelMatrix = glm::rotate(ModelMatrix, armTwoAngle, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(-armTwoTopEndX, -armTwoTopEndY, -armTwoTopEndZ));
		glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);
		glUniform3f(LightID2, lightPos2.x, lightPos2.y, lightPos2.z);
		glUniform3f(ambientID, 0.2f, 0.2f, 0.2f);
		glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &gViewMatrix[0][0]);
		glUniformMatrix4fv(ProjMatrixID, 1, GL_FALSE, &gProjectionMatrix[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);

		glBindVertexArray(VertexArrayId[6]);
		glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[6]);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(ArmTwoVertices), ArmTwoVertices);
		glDrawElements(GL_TRIANGLES, VertexBufferSize[6], GL_UNSIGNED_SHORT, (void*)0);
		glBindVertexArray(0);

		//pen lat lang movement
		if (PenRotation) {
			if (penChange) {
				loadObject("Pen.obj", glm::vec4(1.0, 1.0, 0.0, 1.0), PenVertices, PenIndices, 7);
				createVAOs(PenVertices, PenIndices, 7);
				penChange = false;
			}
			if (penHorizontal > 6.28) {
				penHorizontal = penHorizontal - 6.28;
			}
			else if (penHorizontal < 0.0f) {
				penHorizontal = 6.28 + penHorizontal;
			}

			if (penVertical > 6.28) {
				penVertical = penVertical - 6.28;
			}
			else if (penVertical < 0.0f) {
				penVertical = 6.28 + penVertical;
			}

			if (penAxial > 6.28) {
				penAxial = penAxial - 6.28;
			}
			else if (penAxial < 0.0f) {
				penAxial = 6.28 + penAxial;
			}
			if (ShiftPressed) {
				if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
				{
					penAxial = penAxial - 0.1f;					
				}
				else if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
				{
					penAxial = penAxial + 0.1f;
				}
			}
			else {
				if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
				{
					penVertical = penVertical + 0.1f;
				}
				else if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
				{
					penVertical = penVertical - 0.1f;
				}
				else if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
				{
					penHorizontal = penHorizontal - 0.1f;
				}
				else if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
				{
					penHorizontal = penHorizontal + 0.1f;
				}				
			}
		}
		else {
			if (penChange) {
				loadObject("Pen.obj", glm::vec4(0.8, 0.8, 0.0, 1.0), PenVertices, PenIndices, 7);
				createVAOs(PenVertices, PenIndices, 7);
				penChange = false;
			}
		}
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(penJointX, penJointY, penJointZ));
		ModelMatrix = glm::rotate(ModelMatrix, penHorizontal, glm::vec3(0.0f, 1.0f, 0.0f));
		ModelMatrix = glm::rotate(ModelMatrix, penVertical, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelMatrix = glm::rotate(ModelMatrix, penAxial, glm::vec3(1.0f, 0.0f, 0.0f));
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(-penJointX, -penJointY, -penJointZ));
		
		glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);
		glUniform3f(LightID2, lightPos2.x, lightPos2.y, lightPos2.z);
		glUniform3f(ambientID, 0.2f, 0.2f, 0.2f);
		glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &gViewMatrix[0][0]);
		glUniformMatrix4fv(ProjMatrixID, 1, GL_FALSE, &gProjectionMatrix[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);

		glBindVertexArray(VertexArrayId[7]);
		glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[7]);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(PenVertices), PenVertices);
		glDrawElements(GL_TRIANGLES, VertexBufferSize[7], GL_UNSIGNED_SHORT, (void*)0);
		glBindVertexArray(0);
		glBindVertexArray(VertexArrayId[8]);
		glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[8]);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(ButtonVertices), ButtonVertices);
		glDrawElements(GL_TRIANGLES, VertexBufferSize[8], GL_UNSIGNED_SHORT, (void*)0);
		glBindVertexArray(0);
		penCenter = ModelMatrix * glm::vec4(penJointX, penJointY, penJointZ, 1.0f);
		projectileCenter = ModelMatrix * glm::vec4(projectileX, projectileY, projectileZ, 1.0f);
		

		if (Jump) {
			if (ttt == 1) {
				xdiff = projectileCenter[0] - penCenter[0];
				ydiff = projectileCenter[1] - penCenter[1];
				zdiff = projectileCenter[2] - penCenter[2];
				v1 = atanf(ydiff / sqrt((xdiff*xdiff) + (zdiff*zdiff)));
				v2 = atanf(zdiff / xdiff);
				ttt = 0;
			}
			horizontal = cos(v1)*velocity*time;
			vertical = sin(v1)*velocity*time - (0.5*g*time*time);
			posy = vertical;
			if (xdiff<0 && zdiff<0) {
				posx = -horizontal*cos(v2);
				posz = -horizontal*sin(v2);
			}
			else if (xdiff>=0 && zdiff>=0) {
				posx = horizontal*cos(v2);
				posz = horizontal*sin(v2);
			}
			else if (xdiff < 0 && zdiff >= 0) {
				posx = -horizontal*cos(v2);
				posz = -horizontal*sin(v2);
			}
			else if (xdiff >= 0 && zdiff < 0) {
				posx = horizontal*cos(v2);
				posz = horizontal*sin(v2);
			}
			

			time += 0.01f;
			glm::mat4 Vector;
			Vector = glm::translate(glm::mat4(), glm::vec3(posx, posy, posz));
			ModelMatrix = Vector*ModelMatrix;
			projectileCenter = ModelMatrix * glm::vec4(projectileX, projectileY, projectileZ, 1.0f);
			if (projectileCenter[1] <= 0.0f) {
				printf("called...");
				Jump = false;
				ttt = 1;
				printf("xpos == %f", posx);
				printf("xpos == %f", posz);
				xShift = projectileCenter[0];
				zShift = projectileCenter[2];
				time = 0.0f;
			}
			glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);
			glUniform3f(LightID2, lightPos2.x, lightPos2.y, lightPos2.z);
			glUniform3f(ambientID, 0.2f, 0.2f, 0.2f);
			glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &gViewMatrix[0][0]);
			glUniformMatrix4fv(ProjMatrixID, 1, GL_FALSE, &gProjectionMatrix[0][0]);
			glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
			glBindVertexArray(VertexArrayId[9]);
			glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[9]);
			glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(BulletVertices), BulletVertices);
			glDrawElements(GL_TRIANGLES, VertexBufferSize[9], GL_UNSIGNED_SHORT, (void*)0);
			glBindVertexArray(0);
		}
	}
	glUseProgram(0);
	// Draw GUI
	TwDraw();

	// Swap buffers
	glfwSwapBuffers(window);
	glfwPollEvents();
}

void pickObject(void)
{
	// Clear the screen in white
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(pickingProgramID);
	{
		glm::mat4 ModelMatrix = glm::mat4(1.0); // TranslationMatrix * RotationMatrix;
		glm::mat4 MVP = gProjectionMatrix * gViewMatrix * ModelMatrix;

		// Send our transformation to the currently bound shader, in the "MVP" uniform
		glUniformMatrix4fv(PickingMatrixID, 1, GL_FALSE, &MVP[0][0]);
		
		// ATTN: DRAW YOUR PICKING SCENE HERE. REMEMBER TO SEND IN A DIFFERENT PICKING COLOR FOR EACH OBJECT BEFOREHAND
		glBindVertexArray(0);

	}
	glUseProgram(0);
	// Wait until all the pending drawing commands are really done.
	// Ultra-mega-over slow ! 
	// There are usually a long time between glDrawElements() and
	// all the fragments completely rasterized.
	glFlush();
	glFinish();

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	// Read the pixel at the center of the screen.
	// You can also use glfwGetMousePos().
	// Ultra-mega-over slow too, even for 1 pixel, 
	// because the framebuffer is on the GPU.
	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);
	unsigned char data[4];
	glReadPixels(xpos, window_height - ypos, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, data); // OpenGL renders with (0,0) on bottom, mouse reports with (0,0) on top

	// Convert the color back to an integer ID
	gPickedIndex = int(data[0]);
	
	if (gPickedIndex == 255){ // Full white, must be the background !
		gMessage = "background";
	}
	else {
		std::ostringstream oss;
		oss << "point " << gPickedIndex;
		gMessage = oss.str();
	}

	// Uncomment these lines to see the picking shader in effect
	//glfwSwapBuffers(window);
	//continue; // skips the normal rendering
}

int initWindow(void)
{
	// Initialise GLFW
	if (!glfwInit()) {
		fprintf(stderr, "Failed to initialize GLFW\n");
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	// Open a window and create its OpenGL context
	window = glfwCreateWindow(window_width, window_height, "Muley,Siddhesh (25901911)", NULL, NULL);
	if (window == NULL) {
		fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		return -1;
	}

	// Initialize the GUI
	TwInit(TW_OPENGL_CORE, NULL);
	TwWindowSize(window_width, window_height);
	TwBar * GUI = TwNewBar("Picking");
	TwSetParam(GUI, NULL, "refresh", TW_PARAM_CSTRING, 1, "0.1");
	TwAddVarRW(GUI, "Last picked object", TW_TYPE_STDSTRING, &gMessage, NULL);

	// Set up inputs
	glfwSetCursorPos(window, window_width / 2, window_height / 2);
	glfwSetKeyCallback(window, keyCallback);
	glfwSetMouseButtonCallback(window, mouseCallback);

	return 0;
}

void initOpenGL(void)
{
	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);
	// Cull triangles which normal is not towards the camera
	glEnable(GL_CULL_FACE);

	// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	gProjectionMatrix = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.0f);
	// Or, for an ortho camera :
	//gProjectionMatrix = glm::ortho(-4.0f, 4.0f, -3.0f, 3.0f, 0.0f, 100.0f); // In world coordinates

	// Camera matrix
	gViewMatrix = glm::lookAt(glm::vec3(10.0, 10.0, 10.0f),	// eye
		glm::vec3(0.0, 0.0, 0.0),	// center
		glm::vec3(0.0, 1.0, 0.0));	// up

	// Create and compile our GLSL program from the shaders
	programID = LoadShaders("StandardShading.vertexshader", "StandardShading.fragmentshader");
	pickingProgramID = LoadShaders("Picking.vertexshader", "Picking.fragmentshader");

	// Get a handle for our "MVP" uniform
	MatrixID = glGetUniformLocation(programID, "MVP");
	ModelMatrixID = glGetUniformLocation(programID, "M");
	ViewMatrixID = glGetUniformLocation(programID, "V");
	ProjMatrixID = glGetUniformLocation(programID, "P");
	
	PickingMatrixID = glGetUniformLocation(pickingProgramID, "MVP");
	// Get a handle for our "pickingColorID" uniform
	pickingColorID = glGetUniformLocation(pickingProgramID, "PickingColor");
	// Get a handle for our "LightPosition" uniform
	LightID = glGetUniformLocation(programID, "LightPosition_worldspace");
	LightID2 = glGetUniformLocation(programID, "LightPosition_worldspace2");
	ambientID = glGetUniformLocation(programID, "AmbientLight");

	createObjects();
}

void createVAOs(Vertex Vertices[], unsigned short Indices[], int ObjectId) {

	GLenum ErrorCheckValue = glGetError();
	const size_t VertexSize = sizeof(Vertices[0]);
	const size_t RgbOffset = sizeof(Vertices[0].Position);
	const size_t Normaloffset = sizeof(Vertices[0].Color) + RgbOffset;

	// Create Vertex Array Object
	glGenVertexArrays(1, &VertexArrayId[ObjectId]);	//
	glBindVertexArray(VertexArrayId[ObjectId]);		//

	// Create Buffer for vertex data
	glGenBuffers(1, &VertexBufferId[ObjectId]);
	glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[ObjectId]);
	glBufferData(GL_ARRAY_BUFFER, VertexBufferSize[ObjectId], Vertices, GL_STATIC_DRAW);

	// Create Buffer for indices
	if (Indices != NULL) {
		glGenBuffers(1, &IndexBufferId[ObjectId]);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferId[ObjectId]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, IndexBufferSize[ObjectId], Indices, GL_STATIC_DRAW);
	}

	// Assign vertex attributes
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, VertexSize, 0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, VertexSize, (GLvoid*)RgbOffset); 
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, VertexSize, (GLvoid*)Normaloffset);

	glEnableVertexAttribArray(0);	// position
	glEnableVertexAttribArray(1);	// color
	glEnableVertexAttribArray(2);	// normal

	// Disable our Vertex Buffer Object 
	glBindVertexArray(0);

	ErrorCheckValue = glGetError();
	if (ErrorCheckValue != GL_NO_ERROR)
	{
		fprintf(
			stderr,
			"ERROR: Could not create a VBO: %s \n",
			gluErrorString(ErrorCheckValue)
			);
	}
}

void cleanup(void)
{
	// Cleanup VBO and shader
	for (int i = 0; i < NumObjects; i++) {
		glDeleteBuffers(1, &VertexBufferId[i]);
		glDeleteBuffers(1, &IndexBufferId[i]);
		glDeleteVertexArrays(1, &VertexArrayId[i]);
	}
	glDeleteProgram(programID);
	glDeleteProgram(pickingProgramID);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();
}

static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	// ATTN: MODIFY AS APPROPRIATE
	if (action == GLFW_PRESS) {
		switch (key)
		{
		case GLFW_KEY_1:
			if (ArmOneRotation) {
				ArmOneRotation = false;
				armOneChange = true;
				gMessage = "";
			}
			else {
				ArmOneRotation = true;
				BodyMovement = false;
				CameraMovement = false;
				TopRotation = false;
				ArmTwoRotation = false;
				PenRotation = false;
				armOneChange = true;
				baseChange = true;
				topChange = true;
				armTwoChange = true;
				penChange = true;
				gMessage = "Arm One";
			}
			break;
		case GLFW_KEY_2:
			if (ArmTwoRotation) {
				ArmTwoRotation = false;
				armTwoChange = true;
				gMessage = "";
			}
			else {
				ArmTwoRotation = true;
				BodyMovement = false;
				CameraMovement = false;
				TopRotation = false;
				ArmOneRotation = false;
				PenRotation = false;
				armOneChange = true;
				baseChange = true;
				topChange = true;
				armTwoChange = true;
				penChange = true;
				gMessage = "Arm Two";
			}
			break;
		case GLFW_KEY_A:
			break;
		case GLFW_KEY_B:
			if (BodyMovement) {
				BodyMovement = false;
				baseChange = true;
				gMessage = "";
			}
			else {
				BodyMovement = true;
				CameraMovement = false;
				TopRotation = false;
				ArmOneRotation = false;
				ArmTwoRotation = false;
				PenRotation = false;
				armOneChange = true;
				baseChange = true;
				topChange = true;
				armTwoChange = true;
				penChange = true;
				gMessage = "Body";
			}
			break;
		case GLFW_KEY_C:
			if (CameraMovement) {
				CameraMovement = false;
				gMessage = "";
			}
			else {
				CameraMovement = true;
				BodyMovement = false;
				TopRotation = false;
				ArmOneRotation = false;
				ArmTwoRotation = false;
				PenRotation = false;
				armOneChange = true;
				baseChange = true;
				topChange = true;
				armTwoChange = true;
				penChange = true;
				gMessage = "Camera";
			}
			break;
		case GLFW_KEY_P:
			if (PenRotation) {
				PenRotation = false;
				penChange = true;
				gMessage = "";
			}
			else {
				PenRotation = true;
				BodyMovement = false;
				CameraMovement = false;
				TopRotation = false;
				ArmTwoRotation = false;
				ArmOneRotation = false;
				armOneChange = true;
				baseChange = true;
				topChange = true;
				armTwoChange = true;
				penChange = true;
				gMessage = "Pen";
			}
			break;
		case GLFW_KEY_J:
			if (Jump) {
				Jump = false;
				gMessage = "";
			}
			else {
				Jump = true;
				TopRotation = false;
				BodyMovement = false;
				CameraMovement = false;
				ArmOneRotation = false;
				ArmTwoRotation = false;
				PenRotation = false;
				gMessage = "Jump";
			}
			break;
		case GLFW_KEY_S:
			break;
		case GLFW_KEY_T:
			if (TopRotation) {
				TopRotation = false;
				topChange = true;
				gMessage = "";
			}
			else {
				TopRotation = true;
				BodyMovement = false;
				CameraMovement = false;
				ArmOneRotation = false;
				ArmTwoRotation = false;
				PenRotation = false;
				armOneChange = true;
				baseChange = true;
				topChange = true;
				armTwoChange = true;
				penChange = true;
				gMessage = "Top";
			}
			break;
		case GLFW_KEY_LEFT_SHIFT:
		case GLFW_KEY_RIGHT_SHIFT:
			if (ShiftPressed) {
				ShiftPressed = false;
			}
			else {
				ShiftPressed = true;
			}
			break;
		default:
			break;
		}
	}
}

static void mouseCallback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		pickObject();
	}
}

void moveCamera(int direction) {
	float radius, x, y, z, x2, y2, z2;
	float theta2, gamma2;
	radius = 10 * sqrt(3);
	vec3 up = glm::vec3(0.0, 1.0, 0.0), center = glm::vec3(0.0, 0.0, 0.0);
	if (theta > 6.28f) {
		theta = theta - 6.28f;
	}
	if (theta < 0.0f) {
		theta = 6.28f + theta;
	}
	if (gamma > 6.28f) {
		gamma = gamma - 6.28f;
	}
	if (gamma < 0.0f) {
		gamma = 6.28f + gamma;
	}
	/*if (theta >= 1.57f && theta <= 4.71f) {
		printf("in ");
		up = glm::vec3(0.0, -1.0, 0.0);
	}
	else {
		printf("out ");
		up = glm::vec3(0.0, 1.0, 0.0);
	}*/
	if (direction == 1) {
		gamma = gamma - 0.01f;
	}
	else if (direction == 2) {
		theta = theta + 0.01f;
	}
	else if (direction == 3) {
		gamma = gamma + 0.01f;
	}
	else {
		theta = theta - 0.01f;
	}
	x = radius * cos(theta) * cos(gamma);
	y = radius * sin(theta);
	z = radius * cos(theta) * sin(gamma);
	if (theta >= 1.57f && theta <= 4.71f) {
		gViewMatrix = glm::lookAt(glm::vec3(x, y, z), center, glm::vec3(0.0, -1.0, 0.0));
	}
	else {
		gViewMatrix = glm::lookAt(glm::vec3(x, y, z), center, glm::vec3(0.0, 1.0, 0.0));
	}
	//gViewMatrix = glm::lookAt(glm::vec3(x, y, z), center, up);
	lightPos = glm::vec3((10 * cos(theta) * cos(gamma + 0.1f)), 10 * sin(theta), (10 * cos(theta) * sin(gamma + 0.1f)));
	lightPos2 = glm::vec3((10 * cos(theta) * cos(gamma - 0.1f)), 10 * sin(theta), (10 * cos(theta) * sin(gamma - 0.1f)));
}

int main(void)
{
	// initialize window
	int errorCode = initWindow();
	if (errorCode != 0)
		return errorCode;
	// initialize OpenGL pipeline
	initOpenGL();

	// For speed computation
	double lastTime = glfwGetTime();
	int nbFrames = 0;
	do {
		//// Measure speed
		//double currentTime = glfwGetTime();
		//nbFrames++;
		//if (currentTime - lastTime >= 1.0){ // If last prinf() was more than 1sec ago
		//	// printf and reset
		//	printf("%f ms/frame\n", 1000.0 / double(nbFrames));
		//	nbFrames = 0;
		//	lastTime += 1.0;
		//}
		
		if (animation){
			phi += 0.01;
			if (phi > 360)
				phi -= 360;
		}
		if (CameraMovement && glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
		{
			moveCamera(1);
		}
		else if (CameraMovement && glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
		{
			moveCamera(2);
		}
		else if (CameraMovement && glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
		{
			moveCamera(3);
		}
		else if(CameraMovement && glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
		{
			moveCamera(4);
		}
		// DRAWING POINTS
		renderScene();


	} // Check if the ESC key was pressed or the window was closed
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
	glfwWindowShouldClose(window) == 0);

	cleanup();

	return 0;
}