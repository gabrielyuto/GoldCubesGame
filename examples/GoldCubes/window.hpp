#ifndef WINDOW_HPP_
#define WINDOW_HPP_

#include "abcgOpenGL.hpp"

#include <random>

#include "camera.hpp"
#include "ground.hpp"

struct Vertex {
  glm::vec3 position;

  friend bool operator==(Vertex const &, Vertex const &) = default;
};

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

private:
  struct Slenderman {
    GLuint m_vao{};
    GLuint m_vbo{};
    GLuint m_ebo{};
    std::vector<Vertex> m_vertices;
    std::vector<GLuint> m_indices;
  };

  struct Box {
    GLuint m_vao{};
    GLuint m_vbo{};
    GLuint m_ebo{};
    std::vector<Vertex> m_vertices;
    std::vector<GLuint> m_indices;
  };

  glm::ivec2 m_viewportSize{};

  Box m_box;
  Slenderman m_slenderman;

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

  Ground m_ground;

  std::tuple<std::vector<Vertex>, std::vector<GLuint>>
  loadModelFromFile(std::string_view path);

  void createCube();
};

#endif