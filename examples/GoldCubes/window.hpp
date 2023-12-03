#ifndef WINDOW_HPP_
#define WINDOW_HPP_

#include "abcgOpenGL.hpp"
#include "camera.hpp"
#include "maze.hpp"
#include "model.hpp"
#include <glm/fwd.hpp>

class Window : public abcg::OpenGLWindow {
protected:
  void onEvent(SDL_Event const &event) override;
  void onCreate() override;
  void onUpdate() override;
  void onPaint() override;
  void onPaintUI() override;
  void onResize(glm::ivec2 const &size) override;
  void onDestroy() override;

private:
  bool won{false};
  ImFont *m_font{};

  std::vector<char const *> m_shaderNames{"normalmapping"};

  glm::vec3 cube_pos{0.0f, 0.025f, 0.0f};
  glm::vec3 trophy_pos{0.0f, 0.025f, 0.0f};


  std::vector<GLuint> m_programs;

  glm::ivec2 m_viewportSize{};

  Model m_groundModel;
  Model m_wallModel;
  Model m_box;
  Model m_trophy;

  Maze m_maze;

  Camera m_camera;
  float m_dollySpeed{0.0f};
  float m_truckSpeed{0.0f};
  float m_panSpeed{};

  bool m_isFlashlightOn{true};

  // Mapping mode
  // 0: triplanar; 1: cylindrical; 2: spherical; 3: from mesh
  int m_mappingMode{};

  // Light properties
  glm::vec4 m_Ia{10.0f};
  glm::vec4 m_Id{1.0f};
  glm::vec4 m_Is{1.0f};
  float m_lightCutOff{0.98f};
  float m_lightOuterCutOff{0.92f};
  float m_lightOff{2.00f};

  // Material properties
  glm::vec4 m_Ka;
  glm::vec4 m_Kd;
  glm::vec4 m_Ks;
  float m_shininess{};

  void renderMaze();
  void renderBox();
  void renderTrophy();
};

#endif