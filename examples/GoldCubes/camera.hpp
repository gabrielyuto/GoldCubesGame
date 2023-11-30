#ifndef CAMERA_HPP_
#define CAMERA_HPP_

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include "maze.hpp"

class Window;

class Camera {
 public:
  void initializeCamera(Maze maze);
  
  void computeViewMatrix();
  void computeProjectionMatrix(glm::vec2 const &size);

  void dolly(float speed);
  void truck(float speed);
  void pan(float speed);
  void tilt(float speed);

 private:
  friend Window;

  Maze m_maze;

  glm::vec3 m_eye{glm::vec3(0.0f, 0.0f, 2.5f)};     
  glm::vec3 m_at{glm::vec3(0.0f, 0.0f, 0.0f)};     
  glm::vec3 m_up{glm::vec3(0.0f, 1.0f, 0.0f)};      
  glm::vec3 m_atBase{glm::vec3(0.0f, 0.0f, 0.0f)};

  glm::mat4 m_viewMatrix;
  glm::mat4 m_projMatrix;

  float m_maxDepth{};
};

#endif