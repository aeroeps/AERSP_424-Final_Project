#define GL_SILENCE_DEPRECATION
#ifndef GHOST_H
#define GHOST_H

#include <cmath>
#include <atomic>
#include <OpenGL/gl.h>
#include <GLUT/glut.h>

class Ghost {
private:
    std::atomic<float> positionXg;
    std::atomic<float> positionYg;
    static constexpr float squareSize = 50.0;

public:
    Ghost() : positionXg(0.0), positionYg(0.0) {}

    void draw(float posX, float posY);

    void move(float xIncrementg, float yIncrementg);

    void setInitialPosition(float x, float y);
};

#endif // GHOST_H
