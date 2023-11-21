#ifndef CAMERA_HPP_
#define CAMERA_HPP_

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

class Camera {
public:
  void computeViewMatrix();
  void computeProjectionMatrix(glm::vec2 const &size);

  void dolly(float speed);
  void truck(float speed);
  void pan(float speed);

  glm::mat4 const &getViewMatrix() const { return m_viewMatrix; }
  glm::mat4 const &getProjMatrix() const { return m_projMatrix; }

private:
  glm::vec3 m_eye{0.5f, 0.5f, 3.0f};
  glm::vec3 m_at{0.0f, 0.5f, 1.0f};
  glm::vec3 m_up{0.0f, 1.0f, 0.0f};

  glm::mat4 m_viewMatrix;
  glm::mat4 m_projMatrix;
};

#endif