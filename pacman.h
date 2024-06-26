#define GL_SILENCE_DEPRECATION
#ifndef PACMAN_H
#define PACMAN_H

#include <cmath>
#include <atomic>
#include <OpenGL/gl.h>
#include <GLUT/glut.h>

class Pacman {
private:
    std::atomic<float> positionX;
    std::atomic<float> positionY;
    std::atomic<int> rotation;
    static constexpr float squareSize = 50.0;

public:
    // Pacman constructor to set initial position and rotation to 0
    Pacman() : positionX(0.0), positionY(0.0), rotation(0) {}

    void draw(float posX, float posY, float rot);

    void rotate(int angle);
};

#endif // PACMAN_H
