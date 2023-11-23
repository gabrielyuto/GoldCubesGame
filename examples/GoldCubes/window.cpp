#include "window.hpp"
#include "abcgShader.hpp"
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

  m_program =
      abcg::createOpenGLProgram({{.source = assetsPath + "lookat.vert",
                                  .stage = abcg::ShaderStage::Vertex},
                                 {.source = assetsPath + "lookat.frag",
                                  .stage = abcg::ShaderStage::Fragment}});

  m_ground.create(m_program);

  m_viewMatrixLocation = abcg::glGetUniformLocation(m_program, "viewMatrix");
  m_projMatrixLocation = abcg::glGetUniformLocation(m_program, "projMatrix");
  m_modelMatrixLocation = abcg::glGetUniformLocation(m_program, "modelMatrix");
  m_colorLocation = abcg::glGetUniformLocation(m_program, "color"); 

  // ---------------------------------CREATE OBJECT BOX------------------------------------------------
  auto const [vertices_box, indices_box] = loadModelFromFile(assetsPath + "/objmodels/box.obj");

  abcg::glGenBuffers(1, &m_VBO_box);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, m_VBO_box);
  abcg::glBufferData(GL_ARRAY_BUFFER,sizeof(vertices_box.at(0)) * vertices_box.size(), vertices_box.data(), GL_STATIC_DRAW);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, 0);

  abcg::glGenBuffers(1, &m_EBO_box);
  abcg::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO_box);
  abcg::glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices_box.at(0)) * indices_box.size(), indices_box.data(), GL_STATIC_DRAW);
  abcg::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  abcg::glGenVertexArrays(1, &m_VAO_box);
  abcg::glBindVertexArray(m_VAO_box);

  abcg::glBindBuffer(GL_ARRAY_BUFFER, m_VBO_box);

  auto const positionAttribute{
    abcg::glGetAttribLocation(m_program, "inPosition")
  };

  abcg::glEnableVertexAttribArray(positionAttribute);
  abcg::glVertexAttribPointer(positionAttribute, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), nullptr);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, 0);

  abcg::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO_box);

  abcg::glBindVertexArray(0);

  m_box = Box { m_VAO_box, m_VBO_box, m_EBO_box, vertices_box, indices_box };

  // ---------------------------------CREATE OBJECT SLENDERMAN ------------------------------------------------

  auto const [vertices_slender, indices_slender] = loadModelFromFile(assetsPath + "/objmodels/slenderman.obj");

  abcg::glGenBuffers(1, &m_VBO_slender);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, m_VBO_slender);
  abcg::glBufferData(GL_ARRAY_BUFFER, sizeof(vertices_slender.at(0)) * vertices_slender.size(), vertices_slender.data(), GL_STATIC_DRAW);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, 0);

  abcg::glGenBuffers(1, &m_EBO_slender);
  abcg::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO_slender);
  abcg::glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices_slender.at(0)) * indices_slender.size(), indices_slender.data(), GL_STATIC_DRAW);
  abcg::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  abcg::glGenVertexArrays(1, &m_VAO_slender);

  abcg::glBindVertexArray(m_VAO_slender);

  abcg::glBindBuffer(GL_ARRAY_BUFFER, m_VBO_slender);

  auto const positionAttribute_slenderman{
    abcg::glGetAttribLocation(m_program, "inPosition")
  };

  abcg::glEnableVertexAttribArray(positionAttribute_slenderman);
  abcg::glVertexAttribPointer(positionAttribute_slenderman, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), nullptr);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, 0);

  abcg::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO_slender);

  abcg::glBindVertexArray(0);

  m_slenderman = Slenderman{ m_VAO_slender, m_VBO_slender, m_EBO_slender, vertices_slender, indices_slender };
}

void Window::onPaint() {
  abcg::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  abcg::glViewport(0, 0, m_viewportSize.x, m_viewportSize.y);
  abcg::glUseProgram(m_program);
  abcg::glUniformMatrix4fv(m_viewMatrixLocation, 1, GL_FALSE, &m_camera.getViewMatrix()[0][0]);
  abcg::glUniformMatrix4fv(m_projMatrixLocation, 1, GL_FALSE, &m_camera.getProjMatrix()[0][0]);

  const int limit_sup{50};
  const int limit_inf{-50};
  
  for (int x = limit_inf; x < limit_sup; x++) {
    for (int z = limit_inf; z < limit_sup; z++) {
      glm::mat4 model_box{1.0f};
      model_box = glm::translate(model_box, glm::vec3(x, 0.5f, z));
      model_box = glm::rotate(model_box, glm::radians(10.0f), glm::vec3(0, 1, 0));
      model_box = glm::scale(model_box, glm::vec3(0.5f));

      abcg::glBindVertexArray(m_box.m_vao);
      abcg::glUniformMatrix4fv(m_modelMatrixLocation, 1, GL_FALSE, &model_box[0][0]);
      abcg::glUniform4f(m_colorLocation, x, (x + z), z, 1.0f);
      abcg::glDrawElements(GL_TRIANGLES, m_box.m_indices.size(), GL_UNSIGNED_INT, nullptr);                  
    }
  }

  for (int x = limit_inf; x < limit_sup; x++) {
    for (int z = limit_inf; z < limit_sup; z++) {
      glm::mat4 model{1.0f};
      model = glm::translate(model, glm::vec3(x, 0.7f, z));
      model = glm::rotate(model, glm::radians(40.0f), glm::vec3(0, 1, 0));
      model = glm::scale(model, glm::vec3(0.5f));

      abcg::glBindVertexArray(m_box.m_vao);
      abcg::glUniformMatrix4fv(m_modelMatrixLocation, 1, GL_FALSE, &model[0][0]);
      abcg::glUniform4f(m_colorLocation, 0, 0, 0, 1.0f);
      abcg::glDrawElements(GL_TRIANGLES, m_box.m_indices.size(), GL_UNSIGNED_INT, nullptr);
    }
  }

  for (int x = limit_inf; x < limit_sup; x++) {
    for (int z = limit_inf; z < limit_sup; z++) {
      glm::mat4 model{1.0f};
      model = glm::translate(model, glm::vec3(x, 1.0f, z));
      model = glm::rotate(model, glm::radians(70.0f), glm::vec3(0, 1, 0));
      model = glm::scale(model, glm::vec3(0.5f));

      abcg::glBindVertexArray(m_box.m_vao);
      abcg::glUniformMatrix4fv(m_modelMatrixLocation, 1, GL_FALSE, &model[0][0]);
      abcg::glUniform4f(m_colorLocation, x, (x + z), z, 1.0f);
      abcg::glDrawElements(GL_TRIANGLES, m_box.m_indices.size(), GL_UNSIGNED_INT, nullptr);
    }
  }

  for (int x = limit_inf; x < limit_sup; x++) {
    for (int z = limit_inf; z < limit_sup; z++) {
      glm::mat4 model{1.0f};
      model = glm::translate(model, glm::vec3(x, 1.6f, z));
      model = glm::rotate(model, glm::radians(80.0f), glm::vec3(0, 1, 0));
      model = glm::scale(model, glm::vec3(0.5f));

      abcg::glBindVertexArray(m_box.m_vao);
      abcg::glUniformMatrix4fv(m_modelMatrixLocation, 1, GL_FALSE, &model[0][0]);
      abcg::glUniform4f(m_colorLocation, 0, 0, 0, 1.0f);
      abcg::glDrawElements(GL_TRIANGLES, m_box.m_indices.size(), GL_UNSIGNED_INT, nullptr);
    }
  }

  for (int x = limit_inf; x < limit_sup; x++) {
    for (int z = limit_inf; z < limit_sup; z++) {
      glm::mat4 model{1.0f};
      model = glm::translate(model, glm::vec3(x, 1.9f, z));
      model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0, 1, 0));
      model = glm::scale(model, glm::vec3(0.5f));

      abcg::glBindVertexArray(m_box.m_vao);
      abcg::glUniformMatrix4fv(m_modelMatrixLocation, 1, GL_FALSE, &model[0][0]);
      abcg::glUniform4f(m_colorLocation, x, (x + z), z, 1.0f);
      abcg::glDrawElements(GL_TRIANGLES, m_box.m_indices.size(), GL_UNSIGNED_INT, nullptr);
    }
  }

  const int count_gold_cube{10};

  for (int a = 0; a < count_gold_cube; a++) {
    for(int b = 0; b  < count_gold_cube; b++){
      glm::mat4 model{1.0f};
      model = glm::translate(model, glm::vec3(a/2, 0.5f, b/2));
      model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0, 1, 0));
      model = glm::scale(model, glm::vec3(0.1f));

      abcg::glBindVertexArray(m_box.m_vao);
      abcg::glUniformMatrix4fv(m_modelMatrixLocation, 1, GL_FALSE, &model[0][0]);
      abcg::glUniform4f(m_colorLocation, 255, 223, 0.0, 1.0f);
      abcg::glDrawElements(GL_TRIANGLES, m_box.m_indices.size(), GL_UNSIGNED_INT, nullptr);
    }
  }

  // DRAW SLENDERMAN
  glm::mat4 model_slenderman{1.0f};
  model_slenderman = glm::translate(model_slenderman, glm::vec3(0, 0.5f, 2.3));
  model_slenderman = glm::rotate(model_slenderman, glm::radians(180.0f), glm::vec3(0, 1, 0));
  model_slenderman = glm::scale(model_slenderman, glm::vec3(0.001f));

  abcg::glBindVertexArray(m_slenderman.m_vao);
  abcg::glUniformMatrix4fv(m_modelMatrixLocation, 1, GL_FALSE, &model_slenderman[0][0]);
  abcg::glUniform4f(m_colorLocation, 3.0, 1.0, 3.0, 1.0f);
  abcg::glDrawElements(GL_TRIANGLES, m_slenderman.m_indices.size(), GL_UNSIGNED_INT, nullptr);         

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
}

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

std::tuple<std::vector<Vertex>, std::vector<GLuint>>
Window::loadModelFromFile(std::string_view path) {
  tinyobj::ObjReader reader;
  std::vector<Vertex> vertices;
  std::vector<GLuint> indices;

  if (!reader.ParseFromFile(path.data())) {
    if (!reader.Error().empty()) {
      throw abcg::RuntimeError(
          fmt::format("Failed to load model {} ({})", path, reader.Error()));
    }
    throw abcg::RuntimeError(fmt::format("Failed to load model {}", path));
  }

  if (!reader.Warning().empty()) {
    fmt::print("Warning: {}\n", reader.Warning());
  }

  auto const &attributes{reader.GetAttrib()};
  auto const &shapes{reader.GetShapes()};

  vertices.clear();
  indices.clear();

  std::unordered_map<Vertex, GLuint> hash{};

  for (auto const &shape : shapes) {
    for (auto const offset : iter::range(shape.mesh.indices.size())) {
      auto const index{shape.mesh.indices.at(offset)};

      auto const startIndex{3 * index.vertex_index};
      auto const vx{attributes.vertices.at(startIndex + 0)};
      auto const vy{attributes.vertices.at(startIndex + 1)};
      auto const vz{attributes.vertices.at(startIndex + 2)};

      Vertex const vertex{.position = {vx, vy, vz}};

      if (!hash.contains(vertex)) {
        hash[vertex] = vertices.size();
        vertices.push_back(vertex);
      }

      indices.push_back(hash[vertex]);
    }
  }

  return std::make_tuple(vertices, indices);
}
