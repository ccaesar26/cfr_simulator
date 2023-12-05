#include "Camera.h"

Camera::Camera(const int width, const int height, const glm::vec3& position)
{
	startPosition = position;
	Set(width, height, position);
}

void Camera::Set(const int width, const int height, const glm::vec3& position)
{
    this->isPerspective = true;
    this->yaw = defaultYaw;
    this->pitch = defaultPitch;

    this->FoVy = defaultFoV;
    this->width = width;
    this->height = height;
    this->zNear = defaultZNear;
    this->zFar = defaultZFar;

    this->worldUp = glm::vec3(0, 1, 0);
    this->position = position;

    lastX = width / 2.0f;
    lastY = height / 2.0f;
    bFirstMouseMove = true;

    UpdateCameraVectors();
}

void Camera::Reset(const int width, const int height)
{
	Set(width, height, startPosition);
}
