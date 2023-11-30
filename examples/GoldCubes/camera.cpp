#include "camera.hpp"

#include <glm/gtc/matrix_transform.hpp>

void Camera::initializeCamera(Maze maze) {
  m_maze = maze;

  m_eye = m_maze.m_startPosition;
  m_at  = m_atBase = m_maze.m_startPosition + glm::vec3(0.0f, 0.0f, 2.5f);
  m_maxDepth = std::max(maze.m_mazeMatrix.size(), maze.m_mazeMatrix[0].size());
}

void Camera::computeViewMatrix() {
  m_viewMatrix = glm::lookAt(m_eye, m_at, m_up);
}

void Camera::computeProjectionMatrix(glm::vec2 const &size) {
  m_projMatrix = glm::mat4(1.0f);
  auto const aspect{size.x / size.y};
  m_projMatrix = glm::perspective(glm::radians(70.0f), aspect, 0.1f, 5.0f);
}

void Camera::dolly(float speed) {
  glm::vec3 forward = glm::normalize(m_atBase - m_eye);

  if (m_maze.canMove(m_eye + forward * speed)) {
    m_eye += forward * speed;
    m_at += forward * speed;
    m_atBase += forward * speed;

    computeViewMatrix();
  }
}

void Camera::truck(float speed) {
  glm::vec3 forward = glm::normalize(m_atBase - m_eye);
  glm::vec3 left = glm::cross(m_up, forward);

  // Move eye and center to the left (speed < 0) or to the right (speed > 0)
  if (m_maze.canMove(m_eye - left * speed)) {
    m_eye -= left * speed;
    m_at -= left * speed;
    m_atBase -= left * speed;

    computeViewMatrix();
  }
}

void Camera::pan(float speed) {
  glm::mat4 transform{glm::mat4(1.0f)};

  // Rotate camera around its local y axis
  transform = glm::translate(transform, m_eye);
  transform = glm::rotate(transform, -speed, m_up);
  transform = glm::translate(transform, -m_eye);

  m_at = transform * glm::vec4(m_at, 1.0f);
  m_atBase = transform * glm::vec4(m_atBase, 1.0f);

  computeViewMatrix();
}