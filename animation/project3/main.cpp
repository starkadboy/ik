#include <stdio.h>
#include <time.h>
#include <iostream>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/vector_angle.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtx/quaternion.hpp"
#include "glm/gtx/vector_angle.hpp"

#include <freeglut.h>
#include <GLFW/glfw3.h>

#include "bone.h"

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 800

//defines angles are in degrees
#define CAMERA_STEP_IN_DEGREES 10.0f
#define CAMERA_INITIAL_ANGLE_X_AXIS -170.0f
#define CAMERA_INITIAL_ANGLE_Y_AXIS 0.0f
#define CAMERA_INITIAL_ANGLE_Z_AXIS 0.0f

#define PI 3.14159265f

//all values for models initialization in glut
#define TARGET_STACKS_SLICES 20
#define BONES_STACKS_SLICES 20
#define TARGET_RADIUS 0.1f
#define FLOOR_WIDTH_LENGTH 5.0f
#define FLOOR_THICKNESS 0.2f

//defines for target movement control
#define TARGET_CONTROL_SPEED 0.1f
#define TARGET_HEIGHT_MAX_LIMIT 4.0f
#define TARGET_HEIGHT_MIN_LIMIT 3.0f
#define TARGET_WIDTH_LENGTH_LIMIT 3.0f //center of this constrain box is at 0.0
#define ENABLE_HEIGHT_CAMERA_CONTROL 0

#define CCD_ITERATIONS_NUM 1000
#define EPSILON 0.001f

void Update(void);
void DrawSkeleton(Bone* bone);
void NextFrame(void);
void OnKeyUp(unsigned char c, int x, int y);
void OnKeyDown(unsigned char c, int x, int y);
float DegreesToRadians(float angle);
float RadiansToDegrees(float radians);
void UpdateBonesAngles();

glm::vec3 cameraAngle = glm::vec3(DegreesToRadians(CAMERA_INITIAL_ANGLE_X_AXIS), DegreesToRadians(CAMERA_INITIAL_ANGLE_Y_AXIS), DegreesToRadians(CAMERA_INITIAL_ANGLE_Z_AXIS));
glm::vec3 targetPos = glm::vec3(0.0f, 1.5f, 0.0f);

GLfloat lightAmbient[] = { 0.1, 0.1, 0.1, 1.0 };
GLfloat lightPosition[] = { 1.0, 0.0, 0.0, 1.0 };
GLfloat lightDiffuse[] = { 0.5, 0.5, 0.5, 1.0 };
GLfloat lightSpecular[] = { 0.5, 0.5, 0.5, 1.0 };

Bone* root;
Bone* endEffector;

int main(int argc, char* argv[])
{
	srand(time(0));
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	glutInitWindowPosition(0, 0);
	glutCreateWindow("Project 3");
	glutDisplayFunc(Update);
	glutIdleFunc(NextFrame);

	glutKeyboardFunc(OnKeyDown);
	glutKeyboardUpFunc(OnKeyUp);

	glShadeModel(GL_SMOOTH);

	glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);
	glEnable(GL_DEPTH_TEST);

	//creating bones structure
	root = new Bone(0.0f);

	endEffector = root->AddChild(new Bone(0.4f)) //1st bone
				      ->AddChild(new Bone(0.4f))->SetRotate(0.0f,  10.0f, 0.0f) //2nd bone
				      ->AddChild(new Bone(0.4f))->SetRotate(0.0f, -10.0f, 0.0f) //3nd bone
				      ->AddChild(new Bone(0.4f))->SetRotate(0.0f,  10.0f, 0.0f) //4nd bone
				      ->AddChild(new Bone(0.4f))->SetRotate(0.0f, -10.0f, 0.0f) //5th bone
				      ->AddChild(new Bone(0.4f))->SetRotate(0.0f,  10.0f, 0.0f);//6th end effector bone
				      
	glutMainLoop();

	delete root;

	return 0;
}

void Update(void)
{
	glClearColor(0.0f, 0.5f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::mat4 perspectiveCameraMatrix = glm::perspective(50.0f, 1.0f, 1.0f, 50.0f);

	glm::vec3 cameraCenter = glm::vec3(0.0f, -2.0f, 0.0f);
	glm::vec3 cameraEye = glm::vec3(0.0f, 0.0f, -30.0f);

	//camera controlled rotation
	glm::mat4 keyCameraRotation = glm::lookAt(cameraEye, cameraCenter, Y_AXIS_3);
	keyCameraRotation = glm::rotate(keyCameraRotation, cameraAngle.x, X_AXIS_3);
	keyCameraRotation = glm::rotate(keyCameraRotation, cameraAngle.y, Y_AXIS_3);
	keyCameraRotation = glm::rotate(keyCameraRotation, cameraAngle.z, Z_AXIS_3);

	//setting camera projection matrix
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(glm::value_ptr(perspectiveCameraMatrix));

	//setting model matrix that is just lookAt matrix as soon as we are not applying transformations to the model matrix (so it's identity)
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(glm::value_ptr(keyCameraRotation));

	//render target sphere
	glPushMatrix();
	GLfloat targetMaterialColor[] = { 0.5f, 0.1f, 0.1f, 1.0f };
	glMaterialfv(GL_FRONT, GL_SPECULAR, targetMaterialColor);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, targetMaterialColor);
	glLoadMatrixf(glm::value_ptr(glm::translate(keyCameraRotation, targetPos)));
	glutSolidSphere(TARGET_RADIUS, TARGET_STACKS_SLICES, TARGET_STACKS_SLICES);
	glPopMatrix();

	//render floor
	glPushMatrix();
	GLfloat floorMaterialColor[] = { 0.1f, 0.1f, 0.8f, 1.0f };
	glMaterialfv(GL_FRONT, GL_SPECULAR, floorMaterialColor);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, floorMaterialColor);
	glScalef(FLOOR_WIDTH_LENGTH, FLOOR_THICKNESS, FLOOR_WIDTH_LENGTH);
	glutSolidCube(1.0f);
	glPopMatrix();

	root->transform = glm::rotate(keyCameraRotation, -90.0f, X_AXIS_3);

	//start drawing sceleton recursively from the root bone
	DrawSkeleton(root);

	glutSwapBuffers();
}

void DrawSkeleton(Bone* bone)
{
	glPushMatrix();

	glm::mat4 lastBoneTransform = bone->transform;
	glm::mat4 newTransform = bone->transform;

	//calculating bone transform matrix
	if (bone->parent)
	{
		newTransform = glm::translate(newTransform, bone->parent->boneLength * Z_AXIS_3);
		newTransform = glm::rotate(newTransform, bone->rotation.x, glm::vec3(newTransform * X_AXIS_4));
		newTransform = glm::rotate(newTransform, bone->rotation.y, glm::vec3(newTransform * Y_AXIS_4));
		newTransform = glm::rotate(newTransform, bone->rotation.z, glm::vec3(newTransform * Z_AXIS_4));
		newTransform = bone->parent->transform * newTransform;
	}
	else
	{
		newTransform = glm::rotate(newTransform, bone->rotation.x, X_AXIS_3);
		newTransform = glm::rotate(newTransform, bone->rotation.y, Y_AXIS_3);
		newTransform = glm::rotate(newTransform, bone->rotation.z, Z_AXIS_3);
	}

	bone->transform = newTransform;

	//if bone is the last one, draw end effector
	if (bone->childrenBones.empty()) 
	{
		glPushMatrix();
		GLfloat endEffectorMaterialColor[] = { 0.5f, 0.0f, 0.0f, 1.0f };
		glm::mat4 endEffectorOffset = glm::translate(bone->transform, Z_AXIS_3 * bone->boneLength * 0.5f);
		glLoadMatrixf(glm::value_ptr(endEffectorOffset));
		glMaterialfv(GL_FRONT, GL_SPECULAR, endEffectorMaterialColor);
		glMaterialfv(GL_FRONT, GL_DIFFUSE, endEffectorMaterialColor);
		gluCylinder(gluNewQuadric(), 0.03f, 0.03f, 0.5f, BONES_STACKS_SLICES, BONES_STACKS_SLICES);
		glPopMatrix();
	}

	//draw connector between bones
	glPushMatrix();
	GLfloat boneConnectorMaterialColor[] = { 0.2f, 0.2f, 0.2f, 1.0 };
	glLoadMatrixf(glm::value_ptr(bone->transform));
	glMaterialfv(GL_FRONT, GL_SPECULAR, boneConnectorMaterialColor);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, boneConnectorMaterialColor);
	glutSolidSphere(0.05, BONES_STACKS_SLICES, BONES_STACKS_SLICES);
	glPopMatrix();

	//sending transform and material data to shader
	glLoadMatrixf(glm::value_ptr(bone->transform));

	GLfloat boneMaterialColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glMaterialfv(GL_FRONT, GL_SPECULAR, boneMaterialColor);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, boneMaterialColor);

	//draw bones
	gluCylinder(gluNewQuadric(), 0.01f, 0.01f, bone->boneLength, BONES_STACKS_SLICES, BONES_STACKS_SLICES);

	glPopMatrix();

	//call this function recursively for all children of the current bone
	for (auto it = bone->childrenBones.begin(); it != bone->childrenBones.end(); ++it)
		DrawSkeleton(*it);

	bone->transform = lastBoneTransform;
}

void NextFrame(void)
{
	glutPostRedisplay();
}

void OnKeyUp(unsigned char c, int x, int y)
{
	switch (c)
	{
	//run ccd
	case 'c':
		UpdateBonesAngles();
		break;
	}
}

void OnKeyDown(unsigned char c, int x, int y)
{
	switch (c) 
	{
	//target position control
	case 'd':
		if (targetPos.x + TARGET_CONTROL_SPEED > TARGET_WIDTH_LENGTH_LIMIT / 2)
			targetPos.x = TARGET_WIDTH_LENGTH_LIMIT / 2;
		else
			targetPos.x += TARGET_CONTROL_SPEED;
		break;
	case 'a':
		if (targetPos.x - TARGET_CONTROL_SPEED < -TARGET_WIDTH_LENGTH_LIMIT / 2)
			targetPos.x = -TARGET_WIDTH_LENGTH_LIMIT / 2;
		else
			targetPos.x -= TARGET_CONTROL_SPEED;
		break;
#if ENABLE_HEIGHT_CAMERA_CONTROL
	case 'r':
		if (targetPos.y + TARGET_CONTROL_SPEED > TARGET_HEIGHT_MAX_LIMIT)
			targetPos.y = TARGET_HEIGHT_MAX_LIMIT;
		else
			targetPos.y += TARGET_CONTROL_SPEED;
		break;
	case 'f':
		if (targetPos.y - TARGET_CONTROL_SPEED < TARGET_HEIGHT_MIN_LIMIT)
			targetPos.y = TARGET_HEIGHT_MIN_LIMIT;
		else
			targetPos.y -= TARGET_CONTROL_SPEED;
		break;
#endif // ENABLE_HEIGHT_CAMERA_CONTROL
	case 's':
		if (targetPos.z + TARGET_CONTROL_SPEED > TARGET_WIDTH_LENGTH_LIMIT / 2)
			targetPos.z = TARGET_WIDTH_LENGTH_LIMIT / 2;
		else
			targetPos.z += TARGET_CONTROL_SPEED;
		break;
	case 'w':
		if (targetPos.z - TARGET_CONTROL_SPEED < -TARGET_WIDTH_LENGTH_LIMIT / 2)
			targetPos.z = -TARGET_WIDTH_LENGTH_LIMIT / 2;
		else
			targetPos.z -= TARGET_CONTROL_SPEED;
		break;
	//camera rotation controls
	case 'q':
		cameraAngle.y += DegreesToRadians(CAMERA_STEP_IN_DEGREES);
		break;
	case 'e':
		cameraAngle.y -= DegreesToRadians(CAMERA_STEP_IN_DEGREES);
		break;
	}
}

float DegreesToRadians(float angle)
{
	return angle * PI / 180.0f;
}

float RadiansToDegrees(float radians)
{
	return radians * 180.0f / PI;
}

void UpdateBonesAngles()
{
	bool found = false;

	for (int i = 0; i < CCD_ITERATIONS_NUM; i++)
	{
		Bone* currentBone = endEffector;

		while (currentBone->parent != NULL)
		{
			glm::vec3 startPosition = glm::vec3(currentBone->parent->GetBoneEnd());
			glm::vec3 endPosition = glm::vec3(currentBone->GetBoneEnd());

			glm::vec3 dirToTarget = glm::normalize(targetPos - startPosition);
			glm::vec3 dirToEndPos = glm::normalize(endPosition - startPosition);

			if (glm::dot(dirToEndPos, dirToTarget) < 0.99f)
			{
				float angle = glm::angle(dirToTarget, dirToEndPos);
				glm::quat rotation = normalize(glm::angleAxis(angle, cross(dirToEndPos, dirToTarget)));
				glm::vec3 euler = glm::eulerAngles(rotation);
				currentBone->CheckConstrainsAndMax(euler);
			}

			glm::vec3 differnce = glm::vec3(endEffector->GetBoneEnd()) - targetPos;

			currentBone = currentBone->parent;

			if (dot(differnce, differnce) < EPSILON)
			{
				found = true;
				break;
			}
		}

		if (found)
			break;
	}
}