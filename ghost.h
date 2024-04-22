#define GL_SILENCE_DEPRECATION
#ifndef GHOST_H
#define GHOST_H

#include <cmath>
#include <atomic>
#include <OpenGL/gl.h>
#include <GLUT/glut.h>

class Ghost {
private:
    std::atomic<float> positionX;
    std::atomic<float> positionY;
    static constexpr float squareSize = 50.0;

public:
    Ghost() : positionX(0.0), positionY(0.0) {}

    void draw(float posX, float posY);

    void move(float xIncrement, float yIncrement);

    void setInitialPosition(float x, float y);
};

#endif // GHOST_H