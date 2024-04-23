#define GL_SILENCE_DEPRECATION
#ifndef PACMAN_H
#define PACMAN_H

#include <cmath>
#include <atomic>
#include <OpenGL/gl.h>
#include <GLUT/glut.h>

class Pacman {
private:
    // Atomic variables for thread-safe access to position and rotation
    std::atomic<float> positionX;
    std::atomic<float> positionY;
    std::atomic<int> rotation;

    // Static constant for the size of each game square
    static constexpr float squareSize = 50.0;

public:
    // Constructor to initialize position and rotation
    Pacman() : positionX(0.0), positionY(0.0), rotation(0) {}

    // Method to draw Pacman at a specified position and rotation
    void draw(float posX, float posY, float rot);

    // Method to rotate Pacman
    void rotate(int angle);

    // Method to move Pacman by specified x and y increments
    void move(float xIncrement, float yIncrement);

    // Method to set Pacman's initial position
    void setInitialPosition(float x, float y);
};

#endif // PACMAN_H
