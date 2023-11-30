project(GoldCubes)
add_executable(${PROJECT_NAME} main.cpp model.cpp window.cpp camera.cpp  maze.cpp)
enable_abcg(${PROJECT_NAME})
