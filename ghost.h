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
    float posXg;
    float posYg;

public:
    // Ghost constructor to set initial position to 0
    Ghost() : positionXg(0.0), positionYg(0.0) {}

    void draw(float posX, float posY);

    float getPosXg() const { return positionXg.load(); }
    float getPosYg() const { return positionYg.load(); }

};

#endif // GHOST_H
