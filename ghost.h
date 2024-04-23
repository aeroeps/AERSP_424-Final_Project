#define GL_SILENCE_DEPRECATION
#ifndef GHOST_H
#define GHOST_H

#include <cmath>
#include <atomic>
#include <OpenGL/gl.h>
#include <GLUT/glut.h>

class Ghost {
private:
    // Atomic variables for thread-safe access to position
    std::atomic<float> positionXg;
    std::atomic<float> positionYg;

    // Static constant for the size of each game square
    static constexpr float squareSize = 50.0;

public:
    // Constructor to initialize position
    Ghost() : positionXg(0.0), positionYg(0.0) {}

    // Method to draw Ghost at a specified position
    void draw(float posX, float posY);

    // Method to move the ghost by specified x and y increments
    void move(float xIncrementg, float yIncrementg);

    // Set the ghost's initial position
    void setInitialPosition(float x, float y);
};

#endif // GHOST_H
