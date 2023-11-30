#include "window.hpp"

#include <imgui.h>
#include <fmt/core.h>

#include <cppitertools/itertools.hpp>
#include <glm/gtc/matrix_inverse.hpp>

void Window::onEvent(SDL_Event const &event) {
  if (event.type == SDL_KEYDOWN) {
    if (event.key.keysym.sym == SDLK_UP || event.key.keysym.sym == SDLK_w)
      m_dollySpeed = 1.0f;
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
        m_dollySpeed > 0)
      m_dollySpeed = 0.0f;
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

void Window::onCreate() {
  auto const assetsPath{abcg::Application::getAssetsPath()};

  abcg::glClearColor(0, 0, 0, 1);
  abcg::glEnable(GL_DEPTH_TEST);

  for (auto const &name : m_shaderNames) {
    auto const path{assetsPath + "shaders/" + name};
    auto const program{abcg::createOpenGLProgram(
        {{.source = path + ".vert", .stage = abcg::ShaderStage::Vertex},
         {.source = path + ".frag", .stage = abcg::ShaderStage::Fragment}})};
    m_programs.push_back(program);
  }

  // Load models
  m_grassModel.loadObj(assetsPath + "models/grass.obj", false);
  m_grassModel.setupVAO(m_programs.at(0));

  m_wallModel.loadObj(assetsPath + "models/wall.obj", false);
  m_wallModel.setupVAO(m_programs.at(0));

  // Use material properties from the loaded model (they are the same)
  m_Ka = m_wallModel.getKa();
  m_Kd = m_wallModel.getKd();
  m_Ks = m_wallModel.getKs();
  m_shininess = m_wallModel.getShininess();
  m_mappingMode = 3;

  m_maze.initializeMaze(assetsPath + "levels/level1.txt");
  m_camera.initializeCamera(m_maze);
}

void Window::onPaint() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glViewport(0, 0, m_viewportSize.x, m_viewportSize.y);

  renderMaze();
}

void Window::onPaintUI() {
  auto const assetsPath{abcg::Application::getAssetsPath()};

  ImGui::SetNextWindowPos(ImVec2{0, 0});
  ImGui::SetNextWindowSize(ImVec2{(float)m_viewportSize.x, (float)m_viewportSize.y});
}

void Window::onResize(glm::ivec2 const &size) {
  m_viewportSize = size;
  m_camera.computeProjectionMatrix(size);
}

void Window::onDestroy() { 
  for (auto const &program : m_programs) {
    abcg::glDeleteProgram(program);
  }
}

void Window::renderMaze() {
  abcg::glUseProgram(m_programs.at(0));

  // Get location of uniform variables (could be precomputed)
  GLint modelMatrixLoc{glGetUniformLocation(m_programs.at(0), "modelMatrix")};
  GLint viewMatrixLoc{glGetUniformLocation(m_programs.at(0), "viewMatrix")};
  GLint projMatrixLoc{glGetUniformLocation(m_programs.at(0), "projMatrix")};
  GLint normalMatrixLoc{glGetUniformLocation(m_programs.at(0), "normalMatrix")};
  
  GLint lightDirLoc{glGetUniformLocation(m_programs.at(0), "lightDirWorldSpace")};
  GLint lightPosLoc{glGetUniformLocation(m_programs.at(0), "lightPosWorldSpace")};
  GLint lightCutOffLoc{glGetUniformLocation(m_programs.at(0), "lightCutOff")};
  GLint lightOuterCutOffLoc{glGetUniformLocation(m_programs.at(0), "lightOuterCutOff")};

  GLint IaLoc{glGetUniformLocation(m_programs.at(0), "Ia")};
  GLint IdLoc{glGetUniformLocation(m_programs.at(0), "Id")};
  GLint IsLoc{glGetUniformLocation(m_programs.at(0), "Is")};
  GLint KaLoc{glGetUniformLocation(m_programs.at(0), "Ka")};
  GLint KdLoc{glGetUniformLocation(m_programs.at(0), "Kd")};
  GLint KsLoc{glGetUniformLocation(m_programs.at(0), "Ks")};
  GLint shininessLoc{glGetUniformLocation(m_programs.at(0), "shininess")};

  GLint diffuseTexLoc{glGetUniformLocation(m_programs.at(0), "diffuseTex")};
  GLint normalTexLoc{glGetUniformLocation(m_programs.at(0), "normalTex")};
  GLint mappingModeLoc{glGetUniformLocation(m_programs.at(0), "mappingMode")};

  // Set uniform variables that will be used for every scene object
  glUniformMatrix4fv(viewMatrixLoc, 1, GL_FALSE, &m_camera.m_viewMatrix[0][0]);
  glUniformMatrix4fv(projMatrixLoc, 1, GL_FALSE, &m_camera.m_projMatrix[0][0]);
  glUniform1i(diffuseTexLoc, 0);
  glUniform1i(normalTexLoc, 1);
  glUniform1i(mappingModeLoc, m_mappingMode);

  glm::vec4 lightDir(m_camera.m_at - m_camera.m_eye, 0.0f);
  glm::vec4 lightPos(m_camera.m_eye, 1.0f);
  glUniform4fv(lightDirLoc, 1, &lightDir.x);
  glUniform4fv(lightPosLoc, 1, &lightPos.x);
  glUniform1f(lightCutOffLoc, m_isFlashlightOn ? m_lightCutOff : m_lightOff);
  glUniform1f(lightOuterCutOffLoc, m_isFlashlightOn ? m_lightOuterCutOff : m_lightOff);

  glUniform4fv(IaLoc, 1, &m_Ia.x);
  glUniform4fv(IdLoc, 1, &m_Id.x);
  glUniform4fv(IsLoc, 1, &m_Is.x);
  glUniform4fv(KaLoc, 1, &m_Ka.x);
  glUniform4fv(KdLoc, 1, &m_Kd.x);
  glUniform4fv(KsLoc, 1, &m_Ks.x);
  glUniform1f(shininessLoc, m_shininess);

  // Draw all wall boxes by setting uniform variables of the current object
  for (size_t i = 0; i < m_maze.m_mazeMatrix.size(); i++) {
    for (size_t j = 0; j < m_maze.m_mazeMatrix[i].size(); j++) {
      float xPos =  static_cast<float>(i);
      float yPos =  static_cast<float>(j);

      glm::mat4 modelMatrix{1.0f};
      modelMatrix = glm::translate(modelMatrix, glm::vec3(xPos, 0.0f, yPos));
      glUniformMatrix4fv(modelMatrixLoc, 1, GL_FALSE, &modelMatrix[0][0]);

      auto modelViewMatrix{glm::mat3(m_camera.m_viewMatrix * modelMatrix)};
      glm::mat3 normalMatrix{glm::inverseTranspose(modelViewMatrix)};
      glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, &normalMatrix[0][0]);
        
      if (m_maze.isBox(i, j)) {
        m_wallModel.render();
      }
      else {
        m_grassModel.render();
      }
    }
  }
  glUseProgram(0);
}

void Window::onUpdate() {
  auto const deltaTime{gsl::narrow_cast<float>(getDeltaTime())};

  m_camera.dolly(m_dollySpeed * deltaTime);
  m_camera.truck(m_truckSpeed * deltaTime);
  m_camera.pan(m_panSpeed * deltaTime);

  // if (m_maze.hasFinished(m_camera.m_eye)) {
  //   m_gameOver = true;
  // }
}
