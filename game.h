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
    float x_p;
    float y_p;
    float x_g;
    float y_g;
    std::vector<float> foodPositions;

public:
    Game(Pacman& p, Ghost& g);
    virtual ~Game();
    void init();
    void drawLaberynth();
    bool foodEaten(int x, int y, float pacmanX, float pacmanY);
    void drawFood(float pacmanX, float pacmanY);    
    void keyPressed(unsigned char key, int x, int y);
    void keyUp(unsigned char key, int x, int y);
    void resetGame();
    void keyOperations();
    void gameOver();
    void resultsDisplay();
    void welcomeScreen();
    void display();
    void reshape(int w, int h);
    std::vector<bool> keyStates;
};

#endif // GAME_H
