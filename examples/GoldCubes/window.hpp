#ifndef WINDOW_HPP_
#define WINDOW_HPP_

#include "abcgOpenGL.hpp"

#include <glm/fwd.hpp>
#include <random>

#include "camera.hpp"
#include "ground.hpp"
#include "model.hpp"

enum class RotationDirection { Right, Left, None };

class Window : public abcg::OpenGLWindow {
protected:
  void onEvent(SDL_Event const &event) override;
  void onCreate() override;
  void onPaint() override;
  void onPaintUI() override;
  void onResize(glm::ivec2 const &size) override;
  void onDestroy() override;
  void onUpdate() override;
  void handleColision();

private:
  glm::ivec2 m_viewportSize{};

  GLuint m_VAO{};
  GLuint m_VBO{};
  GLuint m_EBO{};

  GLuint m_VAO_box{};
  GLuint m_VBO_box{};
  GLuint m_EBO_box{};

  GLuint m_VAO_slender{};
  GLuint m_VBO_slender{};
  GLuint m_EBO_slender{};

  GLuint m_program{};

  GLint m_viewMatrixLocation{};
  GLint m_projMatrixLocation{};
  GLint m_modelMatrixLocation{};
  GLint m_colorLocation{};
  GLint m_lightPosition{};
  GLint m_shadowRadius{};

  Camera m_camera;
  float m_dollySpeed{};
  float m_truckSpeed{};
  float m_panSpeed{};

  std::vector<glm::vec3> cubes_pos;
  bool colided = false;
  // ver numero de cubos

  Ground m_ground;

  Model m_model_box;
  int m_trianglesToDraw_box{};

  Model m_model_slenderman;
  int m_trianglesToDraw_slenderman{};


  glm::mat4 m_modelMatrix{1.0f};
  glm::mat4 m_viewMatrix{1.0f};
  glm::mat4 m_projMatrix{1.0f};

  void createCube();
};

#endif