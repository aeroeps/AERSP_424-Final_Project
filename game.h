#ifndef GAME_H
#define GAME_H
#include <vector>
#include <deque>
#include <string>
#include <OpenGL/gl.h>
#include <GLUT/glut.h>

// Forward declaration of Pacman and Ghost classes
class Pacman;
class Ghost;
class Drawable;

class Game {
private:
    Pacman& pacman;
    Ghost& ghost;
    bool replay;
    bool over;
    float squareSize;
    float xIncrementp;
    float yIncrementp;
    float xIncrementg;
    float yIncrementg;
    int rotation;
    std::vector<int> border;
    std::vector<int> obstaclesTop;
    std::vector<int> obstaclesMiddle;
    std::vector<int> obstaclesBottom;
    std::deque<float> food;
    std::vector<std::vector<bool>> bitmap1;
    int points;
    std::vector<Drawable*> drawables;
    bool moveUp;
    bool moveDown;
    bool moveLeft;
    bool moveRight;
    float x;
    float y;
    std::vector<float> foodPositions;

public:
    // Constructor taking references to Pacman and Ghost
    Game(Pacman& p, Ghost& g);

    // Destructor
    virtual ~Game();

    // Initialize gameplay
    void init();

    // Draw the labyrinth
    void drawLaberynth();

    // Check if the food at (x, y) is eaten based on Pacman's position
    bool foodEaten(int x, int y, float pacmanX, float pacmanY);

    // Draw all remaining food and update points
    void drawFood(float pacmanX, float pacmanY);    

    // Handle key press/ release events
    void keyPressed(unsigned char key, int x, int y);
    void keyUp(unsigned char key, int x, int y);

    // Reset the game and all necessary attributes to default status
    void resetGame();

    // Update game state based on key operations
    void keyOperations();

    // Check if the game is over
    void gameOver();

    // Display the game results
    void resultsDisplay();

    // Display the welcome screen
    void welcomeScreen();

    // Display game elements
    void display();

    // Reshape the game if the window size changes
    void reshape(int w, int h);

    bool* keyStates;
};

#endif // GAME_H
