#include "window.hpp"
#include "abcgShader.hpp"
#include "core.h"
#include <glm/gtx/fast_trigonometry.hpp>
#include <unordered_map>

template <> struct std::hash<Vertex> {
  size_t operator()(Vertex const &vertex) const noexcept {
    auto const h1{std::hash<glm::vec3>()(vertex.position)};
    return h1;
  }
};

void Window::onCreate() {
  auto const &assetsPath{abcg::Application::getAssetsPath()};

  abcg::glClearColor(0, 0, 0, 1);
  abcg::glEnable(GL_DEPTH_TEST);

  m_program = abcg::createOpenGLProgram({{.source = assetsPath + "shaders/lookat.vert",
                                  .stage = abcg::ShaderStage::Vertex},
                                 {.source = assetsPath + "shaders/lookat.frag",
                                  .stage = abcg::ShaderStage::Fragment}});

  // OBJ BOX
  m_model_box.loadObj(assetsPath + "objmodels/box.obj");
  m_model_box.setupVAO(m_program);
  m_trianglesToDraw_box = m_model_box.getNumTriangles();

  // OBJ SLENDERMAN
  m_model_slenderman.loadObj(assetsPath + "objmodels/slenderman.obj");
  m_model_slenderman.setupVAO(m_program);
  m_trianglesToDraw_slenderman = m_model_slenderman.getNumTriangles();

  m_ground.create(m_program);
}

void Window::onPaint() {
  abcg::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  abcg::glViewport(0, 0, m_viewportSize.x, m_viewportSize.y);
  abcg::glUseProgram(m_program);

  // Get location of uniform variables
  auto const viewMatrixLoc{abcg::glGetUniformLocation(m_program, "viewMatrix")};
  auto const projMatrixLoc{abcg::glGetUniformLocation(m_program, "projMatrix")};
  auto const modelMatrixLoc{abcg::glGetUniformLocation(m_program, "modelMatrix")};
  auto const colorLoc{abcg::glGetUniformLocation(m_program, "color")};

 // Set uniform variables that have the same value for every model
  abcg::glUniformMatrix4fv(viewMatrixLoc, 1, GL_FALSE, &m_viewMatrix[0][0]);
  abcg::glUniformMatrix4fv(projMatrixLoc, 1, GL_FALSE, &m_projMatrix[0][0]);

  abcg::glUniformMatrix4fv(viewMatrixLoc, 1, GL_FALSE, &m_camera.getViewMatrix()[0][0]);
  abcg::glUniformMatrix4fv(projMatrixLoc, 1, GL_FALSE, &m_camera.getProjMatrix()[0][0]);

  cubes_pos.clear();

  // DRAW CUBS
  const int limit_sup{10};
  const int limit_inf{-10};

  for (int x = limit_inf; x < limit_sup; x++) {
    for (int z = limit_inf; z < limit_sup; z++) {
      glm::mat4 model_box{1.0f};
      model_box = glm::translate(model_box, glm::vec3(x, 0.5f, z));
      model_box = glm::rotate(model_box, glm::radians(10.0f), glm::vec3(0, 1, 0));
      model_box = glm::scale(model_box, glm::vec3(0.5f));

      cubes_pos.push_back(glm::vec3(x, 0.5f, z));

      // Set uniform variables for the current model
      abcg::glUniformMatrix4fv(modelMatrixLoc, 1, GL_FALSE, &model_box[0][0]);
      abcg::glUniform4f(colorLoc, x, (x + z), z, 1.0f);
      m_model_box.render(m_trianglesToDraw_box);      
    }
  }

  // DRAW SLENDERMAN
  glm::mat4 model_slenderman{1.0f};
  model_slenderman = glm::translate(model_slenderman, glm::vec3(0, 0.5f, 2.3));
  model_slenderman = glm::rotate(model_slenderman, glm::radians(180.0f), glm::vec3(0, 1, 0));
  model_slenderman = glm::scale(model_slenderman, glm::vec3(0.5f));

  // Set uniform variables for the current model
  abcg::glUniformMatrix4fv(modelMatrixLoc, 1, GL_FALSE, &model_slenderman[0][0]);
  abcg::glUniform4f(colorLoc, 3.0, 1.0, 3.0, 1.0f);
  m_model_slenderman.render(m_trianglesToDraw_slenderman);

  m_ground.paint();

  abcg::glUseProgram(0);
}

void Window::onPaintUI() { 
  abcg::OpenGLWindow::onPaintUI(); 
}

void Window::onResize(glm::ivec2 const &size) {
  m_viewportSize = size;
  m_camera.computeProjectionMatrix(size);
}

void Window::onDestroy() {
  m_ground.destroy();

  abcg::glDeleteProgram(m_program);
  abcg::glDeleteBuffers(1, &m_EBO);
  abcg::glDeleteBuffers(1, &m_VBO);
  abcg::glDeleteVertexArrays(1, &m_VAO);
}

void Window::onUpdate() {
  auto const deltaTime{gsl::narrow_cast<float>(getDeltaTime())};

  m_camera.dolly(m_dollySpeed * deltaTime);
  m_camera.truck(m_truckSpeed * deltaTime);
  m_camera.pan(m_panSpeed * deltaTime);

  handleColision();
}

void Window::onEvent(SDL_Event const &event) {
  if (event.type == SDL_KEYDOWN) {
    if (event.key.keysym.sym == SDLK_UP || event.key.keysym.sym == SDLK_w) {
      if (colided == false) {
        m_dollySpeed = 1.0f;
      } else {
        m_dollySpeed = 0.0f;
      }
    }
    if (event.key.keysym.sym == SDLK_DOWN || event.key.keysym.sym == SDLK_s)
      m_dollySpeed = -1.0f;
    if (event.key.keysym.sym == SDLK_LEFT || event.key.keysym.sym == SDLK_a)
      m_panSpeed = -1.0f;
    if (event.key.keysym.sym == SDLK_RIGHT || event.key.keysym.sym == SDLK_d)
      m_panSpeed = 1.0f;
    if (event.key.keysym.sym == SDLK_q)
      m_truckSpeed = -1.0f;
    if (event.key.keysym.sym == SDLK_e)
      m_truckSpeed = 1.0f;
  }
  if (event.type == SDL_KEYUP) {
    if ((event.key.keysym.sym == SDLK_UP || event.key.keysym.sym == SDLK_w) &&
        m_dollySpeed > 0) {
      if (colided == false) {
        m_dollySpeed = 0.0f;
      }
    }
    if ((event.key.keysym.sym == SDLK_DOWN || event.key.keysym.sym == SDLK_s) &&
        m_dollySpeed < 0)
      m_dollySpeed = 0.0f;
    if ((event.key.keysym.sym == SDLK_LEFT || event.key.keysym.sym == SDLK_a) &&
        m_panSpeed < 0)
      m_panSpeed = 0.0f;
    if ((event.key.keysym.sym == SDLK_RIGHT ||
         event.key.keysym.sym == SDLK_d) &&
        m_panSpeed > 0)
      m_panSpeed = 0.0f;
    if (event.key.keysym.sym == SDLK_q && m_truckSpeed < 0)
      m_truckSpeed = 0.0f;
    if (event.key.keysym.sym == SDLK_e && m_truckSpeed > 0)
      m_truckSpeed = 0.0f;
  }
}

void Window::handleColision() {
  for (size_t i = 0; i < cubes_pos.size(); i++) {
    float distance = glm::distance(cubes_pos[i], m_camera.getEye());

    if (distance < 0.5) {
      fmt::print("COLIDIU {}\n", distance);
      colided = true;
      return;
    }
  }

  colided = false;

  fmt::print("QUANTOS CUBOS {}\n", cubes_pos.size());
}