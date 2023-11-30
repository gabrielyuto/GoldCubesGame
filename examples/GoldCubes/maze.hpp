#ifndef MAZE_HPP_
#define MAZE_HPP_

#include "abcgOpenGL.hpp"
#include <glm/vec3.hpp>

class Window;
class Camera;

class Maze {
 public:
  void initializeMaze(std::string path);
  bool canMove(glm::vec3 position);
  bool hasFinished(glm::vec3 position);
  bool isBox(int xpos, int ypos);

 private:
  friend Window;
  friend Camera;

  std::vector<std::vector<char>> m_mazeMatrix;
  
  glm::vec3 m_startPosition;
  glm::vec3 m_endPosition;

  glm::vec3 getPositionFromFile(std::ifstream &file);
};

#endif