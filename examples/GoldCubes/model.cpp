#include "model.hpp"

#include <fmt/core.h>
#include <tiny_obj_loader.h>

#include <cppitertools/itertools.hpp>
#include <filesystem>
#include <glm/gtx/hash.hpp>
#include <unordered_map>

namespace std {
template <>
struct hash<Vertex> {
  size_t operator()(Vertex const& vertex) const noexcept {
    std::size_t h1{std::hash<glm::vec3>()(vertex.position)};
    std::size_t h2{std::hash<glm::vec3>()(vertex.normal)};
    std::size_t h3{std::hash<glm::vec2>()(vertex.texCoord)};
    return h1 ^ h2 ^ h3;
  }
};
}

Model::~Model() {
  abcg::glDeleteTextures(1, &m_normalTexture);
  abcg::glDeleteTextures(1, &m_diffuseTexture);
  abcg::glDeleteBuffers(1, &m_EBO);
  abcg::glDeleteBuffers(1, &m_VBO);
  abcg::glDeleteVertexArrays(1, &m_VAO);
}

void Model::loadObj(std::string_view path, bool standardize) {
  auto const basePath{std::filesystem::path{path}.parent_path().string() + "/"};

  tinyobj::ObjReaderConfig readerConfig;
  readerConfig.mtl_search_path = basePath; // Path to material files

  tinyobj::ObjReader reader;

  if (!reader.ParseFromFile(path.data(), readerConfig)) {
    if (!reader.Error().empty()) {
      throw abcg::RuntimeError(
          fmt::format("Failed to load model {} ({})", path, reader.Error()));
    }
    throw abcg::RuntimeError(fmt::format("Failed to load model {}", path));
  }

  if (!reader.Warning().empty()) {
    fmt::print("Warning: {}\n", reader.Warning());
  }

  auto const &attrib{reader.GetAttrib()};
  auto const &shapes{reader.GetShapes()};
  auto const &materials{reader.GetMaterials()};

  m_vertices.clear();
  m_indices.clear();

  m_hasNormals = false;
  m_hasTexCoords = false;

  // A key:value map with key=Vertex and value=index
  std::unordered_map<Vertex, GLuint> hash{};

  // Loop over shapes
  for (auto const &shape : shapes) {
    // Loop over indices
    for (auto const offset : iter::range(shape.mesh.indices.size())) {
      // Access to vertex
      auto const index{shape.mesh.indices.at(offset)};

      // Position
      auto const startIndex{3 * index.vertex_index};
      glm::vec3 position{attrib.vertices.at(startIndex + 0),
                         attrib.vertices.at(startIndex + 1),
                         attrib.vertices.at(startIndex + 2)};

      // Normal
      glm::vec3 normal{};
      if (index.normal_index >= 0) {
        m_hasNormals = true;
        auto const normalStartIndex{3 * index.normal_index};
        normal = {attrib.normals.at(normalStartIndex + 0),
                  attrib.normals.at(normalStartIndex + 1),
                  attrib.normals.at(normalStartIndex + 2)};
      }

      // Texture coordinates
      glm::vec2 texCoord{};
      if (index.texcoord_index >= 0) {
        m_hasTexCoords = true;
        auto const texCoordsStartIndex{2 * index.texcoord_index};
        texCoord = {attrib.texcoords.at(texCoordsStartIndex + 0),
                    attrib.texcoords.at(texCoordsStartIndex + 1)};
      }

      Vertex const vertex{
          .position = position, .normal = normal, .texCoord = texCoord};

      // If hash doesn't contain this vertex
      if (!hash.contains(vertex)) {
        // Add this index (size of m_vertices)
        hash[vertex] = m_vertices.size();
        // Add this vertex
        m_vertices.push_back(vertex);
      }

      m_indices.push_back(hash[vertex]);
    }
  }

  // Use properties of first material, if available
  if (!materials.empty()) {
    auto const &mat{materials.at(0)}; // First material
    m_Ka = {mat.ambient[0], mat.ambient[1], mat.ambient[2], 1};
    m_Kd = {mat.diffuse[0], mat.diffuse[1], mat.diffuse[2], 1};
    m_Ks = {mat.specular[0], mat.specular[1], mat.specular[2], 1};
    m_shininess = mat.shininess;

    if (!mat.diffuse_texname.empty())
      loadDiffuseTexture(basePath + mat.diffuse_texname);

    if (!mat.normal_texname.empty()) {
      loadNormalTexture(basePath + mat.normal_texname);
    } else if (!mat.bump_texname.empty()) {
      loadNormalTexture(basePath + mat.bump_texname);
    }
  } else {
    // Default values
    m_Ka = {0.1f, 0.1f, 0.1f, 1.0f};
    m_Kd = {0.7f, 0.7f, 0.7f, 1.0f};
    m_Ks = {1.0f, 1.0f, 1.0f, 1.0f};
    m_shininess = 25.0f;
  }

  if (standardize) {
    Model::standardize();
  }

  if (!m_hasNormals) {
    computeNormals();
  }

  if (m_hasTexCoords) {
    computeTangents();
  }

  createBuffers();
}

void Model::loadDiffuseTexture(std::string_view path) {
  if (!std::filesystem::exists(path)) return;

  abcg::glDeleteTextures(1, &m_diffuseTexture);
  m_diffuseTexture = abcg::loadOpenGLTexture({.path = path});
}

void Model::loadNormalTexture(std::string_view path) {
  if (!std::filesystem::exists(path)) return;

  abcg::glDeleteTextures(1, &m_normalTexture);
  m_normalTexture = abcg::loadOpenGLTexture({.path = path});
}

void Model::createBuffers() {
  abcg::glDeleteBuffers(1, &m_EBO);
  abcg::glDeleteBuffers(1, &m_VBO);

  abcg::glGenBuffers(1, &m_VBO);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
  abcg::glBufferData(GL_ARRAY_BUFFER, sizeof(m_vertices[0]) * m_vertices.size(), m_vertices.data(), GL_STATIC_DRAW);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, 0);

  abcg::glGenBuffers(1, &m_EBO);
  abcg::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
  abcg::glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(m_indices[0]) * m_indices.size(),m_indices.data(), GL_STATIC_DRAW);
  abcg::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Model::standardize() {
  // Center to origin and normalize largest bound to [-1, 1]

  // Get bounds
  glm::vec3 max(std::numeric_limits<float>::lowest());
  glm::vec3 min(std::numeric_limits<float>::max());
  for (const auto& vertex : m_vertices) {
    max.x = std::max(max.x, vertex.position.x);
    max.y = std::max(max.y, vertex.position.y);
    max.z = std::max(max.z, vertex.position.z);
    min.x = std::min(min.x, vertex.position.x);
    min.y = std::min(min.y, vertex.position.y);
    min.z = std::min(min.z, vertex.position.z);
  }

  // Center and scale
  const auto center{(min + max) / 2.0f};
  const auto scaling{2.0f / glm::length(max - min)};
  for (auto& vertex : m_vertices) {
    vertex.position = (vertex.position - center) * scaling;
  }
}

void Model::computeNormals() {
  // Clear previous vertex normals
  for (auto& vertex : m_vertices) {
    vertex.normal = glm::zero<glm::vec3>();
  }

  // Compute face normals
  for (const auto offset : iter::range<int>(0, m_indices.size(), 3)) {
    // Get face vertices
    Vertex& a{m_vertices.at(m_indices.at(offset + 0))};
    Vertex& b{m_vertices.at(m_indices.at(offset + 1))};
    Vertex& c{m_vertices.at(m_indices.at(offset + 2))};

    // Compute normal
    const auto edge1{b.position - a.position};
    const auto edge2{c.position - b.position};
    glm::vec3 normal{glm::cross(edge1, edge2)};

    // Accumulate on vertices
    a.normal += normal;
    b.normal += normal;
    c.normal += normal;
  }

  // Normalize
  for (auto& vertex : m_vertices) {
    vertex.normal = glm::normalize(vertex.normal);
  }

  m_hasNormals = true;
}

void Model::computeTangents() {
  // Reserve space for bitangents
  std::vector<glm::vec3> bitangents(m_vertices.size(), glm::vec3(0));

  // Compute face tangents and bitangents
  for (const auto offset : iter::range<int>(0, m_indices.size(), 3)) {
    // Get face indices
    const auto i1{m_indices.at(offset + 0)};
    const auto i2{m_indices.at(offset + 1)};
    const auto i3{m_indices.at(offset + 2)};

    // Get face vertices
    Vertex& v1{m_vertices.at(i1)};
    Vertex& v2{m_vertices.at(i2)};
    Vertex& v3{m_vertices.at(i3)};

    const auto e1{v2.position - v1.position};
    const auto e2{v3.position - v1.position};
    const auto delta1{v2.texCoord - v1.texCoord};
    const auto delta2{v3.texCoord - v1.texCoord};

    // clang-format off
    glm::mat2 M;
    M[0][0] =  delta2.t;
    M[0][1] = -delta1.t;
    M[1][0] = -delta2.s;
    M[1][1] =  delta1.s;
    M *= (1.0f / (delta1.s * delta2.t - delta2.s * delta1.t));

    auto tangent{glm::vec4(M[0][0] * e1.x + M[0][1] * e2.x,
                           M[0][0] * e1.y + M[0][1] * e2.y,
                           M[0][0] * e1.z + M[0][1] * e2.z, 0.0f)};

    auto bitangent{glm::vec3(M[1][0] * e1.x + M[1][1] * e2.x,
                             M[1][0] * e1.y + M[1][1] * e2.y,
                             M[1][0] * e1.z + M[1][1] * e2.z)};
    // clang-format on

    // Accumulate on vertices
    v1.tangent += tangent;
    v2.tangent += tangent;
    v3.tangent += tangent;

    bitangents.at(i1) += bitangent;
    bitangents.at(i2) += bitangent;
    bitangents.at(i3) += bitangent;
  }

  for (auto&& [i, vertex] : iter::enumerate(m_vertices)) {
    const auto& n{vertex.normal};
    const auto& t{glm::vec3(vertex.tangent)};

    // Orthogonalize t with respect to n
    const auto tangent = t - n * glm::dot(n, t);
    vertex.tangent = glm::vec4(glm::normalize(tangent), 0);

    // Compute handedness of re-orthogonalized basis
    const auto b{glm::cross(n, t)};
    const auto handedness{glm::dot(b, bitangents.at(i))};
    vertex.tangent.w = (handedness < 0.0f) ? -1.0f : 1.0f;
  }
}

void Model::render() const {
  abcg::glBindVertexArray(m_VAO);

  abcg::glActiveTexture(GL_TEXTURE0);
  abcg::glBindTexture(GL_TEXTURE_2D, m_diffuseTexture);

  abcg::glActiveTexture(GL_TEXTURE1);
  abcg::glBindTexture(GL_TEXTURE_2D, m_normalTexture);

  abcg::glActiveTexture(GL_TEXTURE2);
  abcg::glBindTexture(GL_TEXTURE_CUBE_MAP, m_cubeTexture);

  // Set minification and magnification parameters
  abcg::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  abcg::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  // Set texture wrapping parameters
  abcg::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  abcg::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  abcg::glFrontFace(GL_CCW);
  abcg::glDepthFunc(GL_LEQUAL);
  
  abcg::glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, nullptr);

  abcg::glBindVertexArray(0);
}

void Model::setupVAO(GLuint program) {
  // Release previous VAO
  abcg::glDeleteVertexArrays(1, &m_VAO);

  // Create VAO
  abcg::glGenVertexArrays(1, &m_VAO);
  abcg::glBindVertexArray(m_VAO);

  // Bind EBO and VBO
  abcg::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, m_VBO);

  // Bind vertex attributes
  GLint positionAttribute = abcg::glGetAttribLocation(program, "inPosition");
  if (positionAttribute >= 0) {
    abcg::glEnableVertexAttribArray(positionAttribute);
    abcg::glVertexAttribPointer(positionAttribute, 3, GL_FLOAT, GL_FALSE,
                          sizeof(Vertex), nullptr);
  }

  GLint normalAttribute = abcg::glGetAttribLocation(program, "inNormal");
  if (normalAttribute >= 0) {
    abcg::glEnableVertexAttribArray(normalAttribute);
    GLsizei offset{sizeof(glm::vec3)};
    abcg::glVertexAttribPointer(normalAttribute, 3, GL_FLOAT, GL_FALSE,
                          sizeof(Vertex), reinterpret_cast<void*>(offset));
  }

  GLint texCoordAttribute{glGetAttribLocation(program, "inTexCoord")};
  if (texCoordAttribute >= 0) {
    abcg::glEnableVertexAttribArray(texCoordAttribute);
    GLsizei offset{sizeof(glm::vec3) + sizeof(glm::vec3)};
    abcg::glVertexAttribPointer(texCoordAttribute, 2, GL_FLOAT, GL_FALSE,
                          sizeof(Vertex), reinterpret_cast<void*>(offset));
  }
  
  GLint tangentCoordAttribute{glGetAttribLocation(program, "inTangent")};
  if (tangentCoordAttribute >= 0) {
    abcg::glEnableVertexAttribArray(tangentCoordAttribute);
    GLsizei offset{sizeof(glm::vec3) + sizeof(glm::vec3) + sizeof(glm::vec2)};
    abcg::glVertexAttribPointer(tangentCoordAttribute, 4, GL_FLOAT, GL_FALSE,
                          sizeof(Vertex), reinterpret_cast<void*>(offset));
  }

  // End of binding
  abcg::glBindBuffer(GL_ARRAY_BUFFER, 0);
  abcg::glBindVertexArray(0);
}