#define GL_SILENCE_DEPRECATION

// Include necessary header files for the game, Pacman, and the Ghost
#include "game.h"
#include "pacman.h"
#include "ghost.h"

// Define constants for arrow key codes
#define LEFT_ARROW 37
#define UP_ARROW 38
#define RIGHT_ARROW 39
#define DOWN_ARROW 40

// Include OpenGL headers
#include <OpenGL/gl.h>
#include <GLUT/glut.h>

// Include other necessary standard libraries
#include <iostream>
#include <vector>
#include <deque>
#include <thread>
#include <atomic>
#include <string>
#include <memory>
#include <type_traits>

using namespace std;

class Drawable {
public:
    // Virtual destructor
    virtual ~Drawable() {}

    // Pure virtual function for drawing that will be implemented by derived classes
    virtual void draw() const = 0;
};


// ** PACMAN **
void Pacman::draw(float posX, float posY, float rot) {
    // Draw Pacman using OpenGL
    glBegin(GL_LINES);

    // Set Pacman color to yellow (RGB: 1.0, 1.0, 0.0)
    glColor3f(1.0, 1.0, 0.0);
    
    int x, y;
    
    // Loop to draw Pacman's mouth
    for (int k = 0; k < 32; k++) {
        x = (float)k / 2.0 * cos((30 + 90 * rotation) * M_PI / 180.0) + (posX * squareSize);
        y = (float)k / 2.0 * sin((30 + 90 * rotation) * M_PI / 180.0) + (posY * squareSize);
        
        // Loop to draw the arc of the mouth
        for (int i = 30; i < 330; i++) {
            // Draw a line segment from the previous point to the current point
            glVertex2f(x, y);

            // Calculate the next point on the arc
            x = (float)k / 2.0 * cos((i + 90 * rotation) * M_PI / 180.0) + (posX * squareSize);
            y = (float)k / 2.0 * sin((i + 90 * rotation) * M_PI / 180.0) + (posY * squareSize);

            // Connect the points once more
            glVertex2f(x, y);
        }
    }
    glEnd();    
}

// Set Pacman's rotation angle
void Pacman::rotate(int angle) { rotation = angle; }

// Move Pacman by the specified increments in x and y directions
void Pacman::move(float xIncrement, float yIncrement) {
    // Read the current position using atomic loads for thread safety
    float oldX = positionX.load();
    float oldY = positionY.load();

    // Update the position using atomic stores for thread safety
    positionX.store(oldX + xIncrement);
    positionY.store(oldY + yIncrement);
}

// Set Pacman's initial position
void Pacman::setInitialPosition(float x, float y) {
    positionX = x;
    positionY = y;
}

// ** GHOST **
    
void Ghost::draw(float posX, float posY) {
    int x, y;
    glBegin(GL_LINES);

    // Set the ghost's color to light pink
    glColor3f(1.0, 0.50, 0.75);
    
    // Draw the head of the ghost
    for (int k = 0; k < 32; k++) {
        // Calculate coordinates for the head's outline
        x = (float)k / 2.0 * cos(360 * M_PI / 180.0) + posX;
        y = (float)k / 2.0 * sin(360 * M_PI / 180.0) + posY;
        
        // Draw the circular outline of the head
        for (int i = 180; i <= 360; i++) {
            glVertex2f(x, y);
            x = (float)k / 2.0 * cos(i * M_PI / 180.0) + posX;
            y = (float)k / 2.0 * sin(i * M_PI / 180.0) + posY;
            glVertex2f(x, y);
        }
    }
    glEnd();

    // Draw the rectangular body of the ghost
    glRectf(posX - 17, posY, posX + 15, posY + 15);
    
    // Draw the eyes and legs of the ghost with points
    glBegin(GL_POINTS);
    glColor3f(0, 0.2, 0.4); // Set to dark blue
    glVertex2f(posX - 11, posY + 14); // Legs
    glVertex2f(posX - 1, posY + 14);  // Legs
    glVertex2f(posX + 8, posY + 14);  // Legs
    
    glVertex2f(posX + 4, posY - 3);   // Eye
    glVertex2f(posX - 7, posY - 3);   // Eye
    glEnd();
}

// Move the Ghost by the specified increments in x and y directions
void Ghost::move(float xIncrementg, float yIncrementg) {
    // Read the current position using atomic loads for thread safety
    float oldX = positionXg.load();
    float oldY = positionYg.load();

    // Update the position using atomic stores for thread safety
    positionXg.store(oldX + xIncrementg);
    positionYg.store(oldY + yIncrementg);
}

// Set the initial position of the Ghost
void Ghost::setInitialPosition(float x, float y) {
    positionXg = x;
    positionYg = y;
}


// ** GAME **
Game::Game(Pacman& p, Ghost& g) : pacman(p), ghost(g), replay(false), over(true), squareSize(50.0), xIncrementp(0), yIncrementp(0), xIncrementg(0), yIncrementg(0), rotation(0), points(0) { 

        // Dynamically allocate the array to store key states
        keyStates = new bool[256];

        // Define the bitmap (game board layout) using a 2D array initialization
        bitmap1 = { { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 },
                    { 1,0,0,0,0,0,1,1,1,0,0,0,0,0,1 },
                    { 1,0,1,0,1,0,0,1,0,0,1,0,1,0,1 },
                    { 1,0,1,0,1,1,0,1,0,1,1,0,1,0,1 },
                    { 1,0,1,0,0,1,0,1,0,1,0,0,1,0,1 },
                    { 1,0,1,1,0,0,0,0,0,0,0,1,1,0,1 },
                    { 1,0,0,0,0,1,1,0,1,1,0,0,0,0,1 },
                    { 1,0,1,1,0,1,0,0,0,1,0,1,1,0,1 },
                    { 1,0,1,0,0,1,1,1,1,1,0,0,1,0,1 },
                    { 1,0,0,0,1,1,1,0,1,1,1,0,0,0,1 },
                    { 1,0,1,0,1,0,0,0,0,0,1,0,1,0,1 },
                    { 1,0,1,0,0,0,1,0,1,0,0,0,1,0,1 },
                    { 1,0,1,1,0,1,1,0,1,1,0,1,1,0,1 },
                    { 1,0,0,0,0,0,0,0,0,0,0,0,0,0,1 },
                    { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 } };

        // Set initial positions for Pacman and Ghost on the game board
        pacman.setInitialPosition(1.5,1.5);
        ghost.setInitialPosition(1.5, 1.5);
}

// Destructor for cleaning up resources allocated by the Game object
Game::~Game() {
    for (auto drawable : drawables) {
        delete drawable;
    }
}

// Initialize the game
void Game::init() {
    // Clear the screen with black
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glShadeModel(GL_FLAT);

    // Reset all key states to false
    for (int i = 0; i < 256; i++) { keyStates[i] = false; }    
}

// Draw the labyrinth based on the bitmap representation
void Game::drawLaberynth() {
    // Iterate through each row (y-axis) of the bitmap
    for (int y = 0; y < bitmap1.size(); ++y) {
        
        // Iterate through each column (x-axis) of the bitmap
        for (int x = 0; x < bitmap1[y].size(); ++x) {

            // Check if the current cell in the bitmap represents an obstacle (value 1)
            if (bitmap1[y][x] == 1) {
                // Calculate the coordinates of the rectangle to fill
                float left = x * squareSize;
                float bottom = y * squareSize;
                float right = (x + 1) * squareSize;
                float top = (y + 1) * squareSize;

                // Draw a filled rectangle for the obstacle
                glColor3f(0.0, 0.0, 0.0);
                glRectf(left, bottom, right, top);
            }
        }
    }

    // Draw the border of the labyrinth
    glColor3f(1.0, 1.0, 1.0); // White color for border
    
    for (int i = 0; i < border.size(); i = i + 4) {
        // Retrieve coordinates from the 'border' vector and draw rectangles for each border segment
        glRectf(border.at(i) * squareSize, border.at(i + 1) * squareSize, border.at(i + 2) * squareSize, border.at(i + 3) * squareSize);
    }
}

// Method to check if the food has been eaten
bool Game::foodEaten(int x, int y, float pacmanX, float pacmanY) {
    float radius = 16.0 * cos(359 * M_PI / 180.0);

    // Check if the food is within the radius of Pacman's mouth
    return (x >= pacmanX - radius && x <= pacmanX + radius) &&
        (y >= pacmanY - radius && y <= pacmanY + radius);
}

// Method to draw all remaining food items and delete the eaten ones
void Game::drawFood(float pacmanX, float pacmanY) {
    vector<float> temp;
    
    // Iterate through each food position
    for (size_t i = 0; i < foodPositions.size(); i += 2) {
        
        // Check if the current food position is not eaten
        if (!foodEaten(foodPositions[i] * squareSize, foodPositions[i + 1] * squareSize, pacmanX, pacmanY)) {
            temp.push_back(foodPositions[i]);
            temp.push_back(foodPositions[i + 1]);
        }
        else {
            // If eaten, increase the player's points counter
            points++;
        }
    }

    // Update the foodPositions vector with the remaining uneaten food positions
    foodPositions = std::move(temp);

    // Draw remaining food items as white points on the screen
    glPointSize(5.0);
    glBegin(GL_POINTS);
    glColor3f(1.0, 1.0, 1.0); // Set color to white for pellets
    
    for (size_t j = 0; j < foodPositions.size(); j += 2) {
        glVertex2f(foodPositions[j] * squareSize, foodPositions[j + 1] * squareSize);
    }
    
    glEnd();
}

// Method to handle key presses
void Game::keyPressed(unsigned char key, int x, int y) {
    // keyStates[key] = true;

    // Process different key presses using a switch statement
    switch(key) {
        // Move Pacman upwards and rotate
        case 'w':
            pacman.move(0.0f, 1.5f);
            pacman.rotate(90);
            break;
        // Move Pacman left and rotate
        case 'a':
            pacman.move(-1.5f, 0.0f);
            pacman.rotate(180);
            break;
        // Move Pacman down and rotate
        case 's':
            pacman.move(0.0f, -1.5f);
            pacman.rotate(270);
            break;
        // Move Pacman right and rotate
        case 'd':
            pacman.move(1.5f, 0.0f);
            pacman.rotate(0);
            break;
        // Move ghost up
        case UP_ARROW:
            ghost.move(0.0f, 1.5f);
            break;
        // Move ghost left
        case LEFT_ARROW:
            ghost.move(-1.5f, 0.0f);
            break;
        // Move ghost down
        case DOWN_ARROW:
            ghost.move(0.0f, -1.5f);
            break;
        // Move ghost right
        case RIGHT_ARROW:
            ghost.move(1.5f, 0.0f);
            break;
        // Reset the game
        case ' ':
            resetGame();
            break;
        default:
            break;
    }
}

// Method to reset the game state, initializing game parameters for a new game
void Game::resetGame() {
    over = false;
    xIncrementp = 0;
    yIncrementp = 0;
    xIncrementg = 0;
    yIncrementg = 0; 
    rotation = 0;
    
    // Reset initial positions
    pacman.setInitialPosition(1.5,1.5);
    ghost.setInitialPosition(1.5, 1.5);

    // Reset key states
    for (int i = 0; i < 256; i++){
        keyStates[i] = false;
    }
    
    // Reset food positions
    foodPositions = { 
        1.5, 1.5, 1.5, 2.5, 1.5, 3.5, 1.5, 4.5, 
        1.5, 5.5, 1.5, 6.5, 1.5, 7.5, 1.5, 8.5, 
        1.5, 9.5, 1.5, 10.5, 1.5, 11.5, 1.5, 12.5, 
        1.5, 13.5, 2.5, 1.5, 2.5, 6.5, 2.5, 9.5, 
        2.5, 13.5, 3.5, 1.5, 3.5, 2.5, 3.5, 3.5, 
        3.5, 4.5, 3.5, 6.5, 3.5, 8.5, 3.5, 9.5, 
        3.5, 10.5, 3.5, 11.5, 3.5, 13.5, 4.5, 1.5, 
        4.5, 4.5, 4.5, 5.5, 4.5, 6.5, 4.5, 7.5, 
        4.5, 8.5, 4.5, 11.5, 4.5, 12.5, 4.5, 13.5, 
        5.5, 1.5, 5.5, 2.5, 5.5, 5.5, 5.5, 10.5, 
        5.5, 13.5, 6.5, 2.5, 6.5, 3.5, 6.5, 4.5, 
        6.5, 5.5, 6.5, 7.5, 6.5, 10.5, 6.5, 13.5, 
        7.5, 5.5, 7.5, 6.5, 7.5, 7.5, 7.5, 9.5, 
        7.5, 10.5, 7.5, 11.5, 7.5, 12.5, 7.5, 13.5, 
        8.5, 2.5, 8.5, 3.5, 8.5, 4.5, 8.5, 5.5, 
        8.5, 7.5, 8.5, 10.5, 8.5, 13.5, 9.5, 1.5, 
        9.5, 2.5, 9.5, 5.5, 9.5, 10.5, 9.5, 11.5, 
        9.5, 13.5, 10.5, 1.5, 10.5, 4.5, 10.5, 5.5, 
        10.5, 6.5, 10.5, 7.5, 10.5, 8.5, 10.5, 11.5, 
        10.5, 12.5, 10.5, 13.5, 11.5, 1.5, 11.5, 2.5, 
        11.5, 3.5, 11.5, 4.5, 11.5, 6.5, 11.5, 8.5, 
        11.5, 9.5, 11.5, 10.5, 11.5, 11.5, 11.5, 13.5, 
        12.5, 1.5, 12.5, 6.5, 12.5, 9.5, 12.5, 13.5, 
        13.5, 1.5, 13.5, 2.5, 13.5, 3.5, 13.5, 4.5, 
        13.5, 5.5, 13.5, 6.5, 13.5, 7.5, 13.5, 8.5, 
        13.5, 9.5, 13.5, 10.5, 13.5, 11.5, 13.5, 12.5, 13.5, 13.5 
    };
}
    
// Method to update character movement based on pressed keys
void Game::keyOperations() {

    float x_p = (1.5 + xIncrementp) * squareSize;
    float y_p = (1.5 + yIncrementp) * squareSize;

    // Update Pacman's movement according to keys pressed
    
    if (keyStates['a']) {
        x_p -= 2;
        int x1Quadrant = (int)((x_p - 16.0 * cos(360 * M_PI / 180.0)) / squareSize);
        if (!this->bitmap1[(int)y_p / squareSize][x1Quadrant]) {
            xIncrementp -= 2 / squareSize;
            pacman.rotate(2);
        }
    }
    
    if (keyStates['d']) {
        x_p += 2;
        int x2Quadrant = (int)((x_p + 16.0 * cos(360 * M_PI / 180.0)) / squareSize);
        if (!this->bitmap1[(int)y_p / squareSize][x2Quadrant]) {
            xIncrementp += 2 / squareSize;
            pacman.rotate(0);
        }
    }
    
    if (keyStates['w']) {
        y_p -= 2;
        int y1Quadrant = (int)((y_p - 16.0 * cos(360 * M_PI / 180.0)) / squareSize);
        if (!this->bitmap1[y1Quadrant][(int)x_p / squareSize]) {
            yIncrementp -= 2 / squareSize;
            pacman.rotate(3);
        }
    }
    
    if (keyStates['s']) {
        y_p += 2;
        int y2Quadrant = (int)((y_p + 16.0 * cos(360 * M_PI / 180.0)) / squareSize);
        if (!this->bitmap1[y2Quadrant][(int)x_p / squareSize]) {
            yIncrementp += 2 / squareSize;
            pacman.rotate(1);
        }
    }

    // Update Ghost's movement according to keys pressed
    
    float x_g = (1.5 + xIncrementg) * squareSize;
    float y_g = (1.5 + yIncrementg) * squareSize;

    if (keyStates['j']) {
        x_g -= 2;
        int x1Quadrantg = (int)((x_g - 16.0 * cos(360 * M_PI / 180.0)) / squareSize);
        if (!this->bitmap1[(int)y_g / squareSize][x1Quadrantg]) {
            xIncrementg -= 2 ;
            
        }
    }
    
    if (keyStates['l']) {
        x_g += 2;
        int x2Quadrantg = (int)((x_g + 16.0 * cos(360 * M_PI / 180.0)) / squareSize);
        if (!this->bitmap1[(int)y_g / squareSize][x2Quadrantg]) {
            xIncrementg += 2  ;
            
        }
    }
    
    if (keyStates['i']) {
        y_g -= 2;
        int y1Quadrantg = (int)((y_g - 16.0 * cos(360 * M_PI / 180.0)) / squareSize);
        if (!this->bitmap1[y1Quadrantg][(int)x_g / squareSize]) {
            yIncrementg -= 2 ;
            
        }
    }
    
    if (keyStates['k']) {
        y_g += 2;
        int y2Quadrantg = (int)((y_g + 16.0 * cos(360 * M_PI / 180.0)) / squareSize);
        if (!this->bitmap1[y2Quadrantg][(int)x_g / squareSize]) {
            yIncrementg += 2 ;
            
        }
    }

    if (keyStates[' ']) {
        // Reset the game if replaying and game over
        if (!replay && over) {
            resetGame();
            replay = true;
        }
        else if (replay && over) {
            replay = false;
        }
    }

    if (keyStates['r']) 
    {
        pacman.rotate(0);
        resetGame();
    }
}

// Method to check if the game is over
void Game::gameOver() {
    // Calculate grid positions of Pacman and Ghost based on movement increments
    int pacmanX = (int)(1.5 + xIncrementp);
    int pacmanY = (int)(1.5 + yIncrementp);
    int ghostX = (int)(2.5 + xIncrementg);
    int ghostY = (int)(2.5 + yIncrementg);
    
    // Check if Pacman collides with the Ghost
    if (pacmanX == ghostX && pacmanY == ghostY) {
            over = true;
            return; // If Pacman collides with ghost, game over
        }

    // Check if all food (105 points) is eaten
    if (points == 105) {
        over = true; // If all food is eaten, game over
    }
}

// Method to display the results of the game at the end
void Game::resultsDisplay() {
    // Clear the screen with black
    glClearColor(0, 0, 0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    
    if (points == 105) {
        // Display message for winning the game
        const char* message = "*************************************";
        glRasterPos2f(170, 250);
        while (*message)
            glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *message++);
        
        message = "CONGRATULATIONS, PACMAN, YOU WON! ";
        glColor3f(1, 1, 1);
        glRasterPos2f(200, 300);
        while (*message)
            glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *message++);
        
        message = "*************************************";
        glRasterPos2f(170, 350);
        while (*message)
            glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *message++);
        
        message = "To start or restart the game, press the space key.";
        glRasterPos2f(170, 550);
        while (*message)
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *message++);
    } else {
        // Display message for losing the game
        const char* message = "*************************";
        glRasterPos2f(210, 250);
        while (*message)
            glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *message++);
        
        message = "SORRY, PACMAN, YOU LOST ... ";
        glColor3f(1, 1, 1);
        glRasterPos2f(250, 300);
        while (*message)
            glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *message++);
        
        message = "*************************";
        glRasterPos2f(210, 350);
        while (*message)
            glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *message++);
        
        message = "You got: ";
        glRasterPos2f(260, 400);
        while (*message)
            glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *message++);
        
        string result = to_string(points);
        message = (char*)result.c_str();
        glRasterPos2f(350, 400);
        while (*message)
            glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *message++);
        
        message = " points!";
        glRasterPos2f(385, 400);
        while (*message)
            glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *message++);
        
        message = "To start or restart the game, press the space key.";
        glRasterPos2f(170, 550);
        while (*message)
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *message++);
    }
    glFlush();
}

// Method to display the starting instructions
void Game::welcomeScreen() {
        glClearColor(0, 0.2, 0.4, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);
        const char* message = "*************************************";
        glRasterPos2f(150, 200);
        while (*message)
            glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *message++);
        message = "PACMAN vs. GHOST";
        glColor3f(1, 1, 1);
        glRasterPos2f(245, 250);
        while (*message)
            glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *message++);
        message = "*************************************";
        glRasterPos2f(150, 300);
        while (*message)
            glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *message++);
        message = "To control Pacman use WASD";
        glRasterPos2f(200, 400);
        while(*message)
            glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *message++);
        message = "To control the ghost use arrow keys";
        glRasterPos2f(220, 500);
        while (*message)
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *message++);
        message = "To start the game, press the space key twice.";
        glRasterPos2f(170, 550);
        while (*message)
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *message++);
        glFlush();
    }

// Method to display the screen and its elements
void Game::display() {
    // If the game has just started with 1 point, set over to false
    if (this->points ==1) { this->over = false; }

    this->keyOperations();
    glClear(GL_COLOR_BUFFER_BIT);
    this->gameOver();

    // If the player is replaying and the game is over, draw the labyrinth
    if (this->replay) {
        if (!this->over) {
            this->drawLaberynth();
            this->drawFood((1.5 + this->xIncrementp) * this->squareSize, (1.5 + this->yIncrementp) * this->squareSize);
            this->pacman.draw(1.5 + this->xIncrementp, 1.5 + this->yIncrementp, this->rotation);
            this->ghost.draw(175 + this->xIncrementg, 150 + this->yIncrementg);

        } else {
            this->resultsDisplay();
        }
    } else {
        this->welcomeScreen();
    }
    glutSwapBuffers();
}

// Method to reshape the game if the screen size changes
void Game::reshape(int w, int h) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glViewport(0, 0, (GLsizei)w, (GLsizei)h);
    glOrtho(0, 750, 750, 0, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

// Declare the game object globally

// Create unique pointers for Pacman and Ghost objects
std::unique_ptr<Pacman> pacmanPtr(new Pacman);
std::unique_ptr<Ghost> ghostPtr(new Ghost);

float squareSize = 50.0;
        
Game game(*pacmanPtr, *ghostPtr);

// Define static functions
void displayCallback() { game.display(); }
void reshapeCallback(int w, int h) { game.reshape(w, h); }
void keyPressedCallback(unsigned char key, int x, int y){ game.keyStates[key] = true; }
void keyUpCallback(unsigned char key, int x, int y){ game.keyStates[key] = false; }

void specialKeyPressedCallback(int key, int x, int y) {
    switch (key) {
        case GLUT_KEY_UP:
            game.keyStates[UP_ARROW] = true;
            break;
        case GLUT_KEY_DOWN:
            game.keyStates[DOWN_ARROW] = true;
            break;
        case GLUT_KEY_LEFT:
            game.keyStates[LEFT_ARROW] = true;
            break;
        case GLUT_KEY_RIGHT:
            game.keyStates[RIGHT_ARROW] = true;
            break;
    }
}

void specialKeyUpCallback(int key, int x, int y) {
    switch (key) {
        case GLUT_KEY_UP:
            game.keyStates[UP_ARROW] = false;
            break;
        case GLUT_KEY_DOWN:
            game.keyStates[DOWN_ARROW] = false;
            break;
        case GLUT_KEY_LEFT:
            game.keyStates[LEFT_ARROW] = false;
            break;
        case GLUT_KEY_RIGHT:
            game.keyStates[RIGHT_ARROW] = false;
            break;
    }
}


int main(int argc, char** argv) {

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);

    // Set window size and position
    glutInitWindowSize(750, 750);
    glutInitWindowPosition(500, 50);
    glutCreateWindow("Pacman vs. Ghost");
    

    // Set GLUT callbacks for keyboard and display events
    glutKeyboardFunc(keyPressedCallback);
    glutKeyboardUpFunc(keyUpCallback);
    glutDisplayFunc(displayCallback);
    glutReshapeFunc(reshapeCallback);
    glutIdleFunc(displayCallback);

    glutSpecialFunc(specialKeyPressedCallback);
    glutSpecialUpFunc(specialKeyUpCallback);

    // Initialize the game and enter the GLUT main loop
    game.init();
    glutMainLoop();
    
    return 0;
}
