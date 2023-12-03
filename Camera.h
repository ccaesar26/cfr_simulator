#pragma once
#include <glm/glm.hpp>

class Camera
{
public:

private:
	const float defaultZNear = 0.1f;
	const float defaultZFar = 500.f;
	const float defaultYaw = -90.f;
	const float defaultPitch = 0.f;
	const float defaultFoV = 45.f;

	const float cameraSpeedFactor = 2.5f;
	const float mouseSensitivity = 0.1f;

	// Perspective properties
	float zNear;
	float zFar;
	float FoVy;
	int width;
	int height;
	bool isPerspective;


	glm::vec3 startPosition;

	glm::vec3 position;
	glm::vec3 forward;
	glm::vec3 right;
	glm::vec3 up;
	glm::vec3 worldUp;
};