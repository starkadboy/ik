#ifndef BONE_HEADER
#define BONE_HEADER

#include <vector>
#include <algorithm>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#define X_AXIS_3 glm::vec3(1.0f, 0.0f, 0.0f)
#define Y_AXIS_3 glm::vec3(0.0f, 1.0f, 0.0f)
#define Z_AXIS_3 glm::vec3(0.0f, 0.0f, 1.0f)
#define X_AXIS_4 glm::vec4(1.0f, 0.0f, 0.0f, 0.0f)
#define Y_AXIS_4 glm::vec4(0.0f, 1.0f, 0.0f, 0.0f)
#define Z_AXIS_4 glm::vec4(0.0f, 0.0f, 1.0f, 0.0f)
#define W_AXIS_4 glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)

#define DEGREE_EPSILON 0.1f

glm::mat4 initialSceletonRotation = glm::rotate(glm::mat4(), -90.0f, X_AXIS_3);

class Bone
{
public:
	Bone* parent;
	std::vector<Bone*> childrenBones;
	glm::mat4 transform;
	glm::vec3 rotation;
	glm::vec3 constraint[2]; //angle constrains
	float boneLength;

	Bone(float length)
	{
		parent = NULL;
		transform = glm::mat4(1.0f);
		rotation = glm::vec3(0.0f);
		constraint[0] = glm::vec3(-360.0f);
		constraint[1] = glm::vec3(360.0f);
		boneLength = length;
	}

	Bone(const Bone& b)
	{
		parent = NULL;
		transform = glm::mat4(b.transform);
		rotation = glm::vec3(b.rotation);
		constraint[0] = glm::vec3(b.constraint[0]);
		constraint[1] = glm::vec3(b.constraint[1]);
		boneLength = b.boneLength;

		for (auto it = b.childrenBones.begin(); it != b.childrenBones.end(); ++it)
			AddChild(new Bone(**it));
	}


	Bone* AddChild(Bone* b)
	{
		b->parent = this;
		childrenBones.push_back(b);
		return b;
	}

	Bone* SetRotate(float x, float y, float z)
	{
		rotation = glm::vec3(fmod(x, 360.0f), fmod(y, 360.0f), fmod(z, 360.0f));
		return this;
	}

    Bone* CheckConstrainsAndMax(glm::vec3 delta)
	{
		CheckDelta(delta.x, rotation.x);
		CheckDelta(delta.y, rotation.y);
		CheckDelta(delta.z, rotation.z);

		SetRotate(rotation.x + delta.x, rotation.y + delta.y, rotation.z + delta.z);

		return this;
	}

	glm::vec4 GetBoneEnd()
	{
		return glm::translate(CalcTransform(), Z_AXIS_3 * boneLength) * W_AXIS_4;
	}

	void CheckDelta(float& d, float degree)
	{
		if (d + degree > 180.0f)
			d -= 360.0f;

		if (d + degree < -180.0f)
			d += 360.0f;

		if (d + degree > constraint[1].x)
			d = constraint[1].x - degree - DEGREE_EPSILON;

		if (d + degree < constraint[0].x)
			d = constraint[0].x - degree + DEGREE_EPSILON;
	}

private:
	glm::mat4 CalcTransform()
	{
		if (parent != NULL)
		{
			glm::mat4 parentTransform = glm::translate(glm::mat4(), Z_AXIS_3 * parent->boneLength);
			parentTransform = glm::rotate(parentTransform, rotation.x, glm::vec3(parentTransform * X_AXIS_4));
			parentTransform = glm::rotate(parentTransform, rotation.y, glm::vec3(parentTransform * Y_AXIS_4));
			parentTransform = glm::rotate(parentTransform, rotation.z, glm::vec3(parentTransform * Z_AXIS_4));
			parentTransform = parent->CalcTransform() * parentTransform;

			return parentTransform;
		}
		else
			return initialSceletonRotation;
	}
};

#endif