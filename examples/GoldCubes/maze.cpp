#include "maze.hpp"

#include <fstream>
#include <string>

void Maze::initializeMaze(std::string path) {
  std::ifstream file(path);
  std::string fileLine;
  
  m_startPosition = getPositionFromFile(file);
  m_endPosition = getPositionFromFile(file);

  m_mazeMatrix.clear();
  while (std::getline(file, fileLine)){
    std::vector<char> matrixRow(fileLine.begin(), fileLine.end());    
    m_mazeMatrix.push_back(matrixRow);
  }
}

glm::vec3 Maze::getPositionFromFile(std::ifstream &file) {
  std::string xPos;
  std::string yPos;

  std::getline(file, xPos);
  std::getline(file, yPos);

  return glm::vec3(stof(xPos), 0, stof(yPos));
}

bool Maze::canMove(glm::vec3 pos) {
  float minBorder = 0.2f;
  std::vector<float> borderDiffs = {0, minBorder, -minBorder};

  for (auto xBorder : borderDiffs) {
    for (auto yBorder : borderDiffs) {
      int xPos = static_cast<int>(pos[0] + 0.5f + xBorder);
      int yPos = static_cast<int>(pos[2] + 0.5f + yBorder);

      if (isBox(xPos, yPos)) {
        return false;
      }
    }
  }

  return true;
}

bool Maze::hasFinished(glm::vec3 pos) {
  int xPos = static_cast<int>(pos[0]);
  int yPos = static_cast<int>(pos[2]);

  return glm::vec3{xPos, 0, yPos} == m_endPosition;
}

bool Maze::isBox(int xPos, int yPos) {
  return m_mazeMatrix[xPos][yPos] == 'x';
}
