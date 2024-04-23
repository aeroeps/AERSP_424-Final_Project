#define GL_SILENCE_DEPRECATION
#include "game.h"
#include "pacman.h"
#include "ghost.h"

#define LEFT_ARROW 37
#define UP_ARROW 38
#define RIGHT_ARROW 39
#define DOWN_ARROW 40
#include <OpenGL/gl.h>
#include <GLUT/glut.h>
#include <iostream>
#include <vector>
#include <deque>
#include <thread>
#include <atomic>
#include <string>
#include <memory>
#include <cmath>



using namespace std;

class Drawable {
public:
    virtual ~Drawable() {} // Virtual destructor
    virtual void draw() const = 0;
};


// ** PACMAN
    void Pacman::draw(float posXg, float posYg, float rot) {
        // Drawing logic for Pacman
        glBegin(GL_LINES);
        glColor3f(1.0, 1.0, 0.0);
        int x, y;
        for (int k = 0; k < 32; k++) {
            x = (float)k / 2.0 * cos((30 + 90 * rotation) * M_PI / 180.0) + (posXg * squareSize);
            y = (float)k / 2.0 * sin((30 + 90 * rotation) * M_PI / 180.0) + (posYg * squareSize);
            for (int i = 30; i < 330; i++) {
                glVertex2f(x, y);
                x = (float)k / 2.0 * cos((i + 90 * rotation) * M_PI / 180.0) + (posXg * squareSize);
                y = (float)k / 2.0 * sin((i + 90 * rotation) * M_PI / 180.0) + (posYg * squareSize);
                glVertex2f(x, y);
            }
        }
        glEnd();    
    }

    void Pacman::rotate(int angle) { rotation = angle; }

// ** GHOST
    
    void Ghost::draw(float posXg, float posYg) {
        
        posXg += 170;
        posYg += 150;
        
        int x, y;
        glBegin(GL_LINES);
        glColor3f(1.0, 0.50, 0.75);
        
        // Draw the head
        for (int k = 0; k < 32; k++) {
            x = (float)k / 2.0 * cos(360 * M_PI / 180.0) + posXg;
            y = (float)k / 2.0 * sin(360 * M_PI / 180.0) + posYg;
            for (int i = 180; i <= 360; i++) {
                glVertex2f(x, y);
                x = (float)k / 2.0 * cos(i * M_PI / 180.0) + posXg;
                y = (float)k / 2.0 * sin(i * M_PI / 180.0) + posYg;
                glVertex2f(x, y);
            }
        }
        glEnd();

        // Draw body
        glRectf(posXg - 17, posYg, posXg + 15, posYg + 15);
        
        // Draw eyes and legs
        glBegin(GL_POINTS);
        glColor3f(0, 0.2, 0.4);
        glVertex2f(posXg - 11, posYg + 14); // Legs
        glVertex2f(posXg - 1, posYg + 14);  // Legs
        glVertex2f(posXg + 8, posYg + 14);  // Legs
        glVertex2f(posXg + 4, posYg - 3);   // Eyes
        glVertex2f(posXg - 7, posYg - 3);   // Eyes
        glEnd();
    }



// ** GAME
    Game::Game(Pacman& p, Ghost& g) : pacman(p), ghost(g), replay(false), over(true), squareSize(50.0), xIncrementp(0), yIncrementp(0), xIncrementg(1.5), yIncrementg(1.5), rotation(0), points(0) {
        
            keyStates.resize(256);

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
    }

    Game::~Game() {
        for (auto drawable : drawables) {
            delete drawable;
        }
    }
    
    void Game::init() {
        // Clear screen
        //glClearColor(0.0, 0.0, 0.0, 0.0);
        //glShadeModel(GL_FLAT);

        // Reset all keys
        for (int i = 0; i < 256; i++) { keyStates[i] = false; }    
    }

    void Game::drawLaberynth() {
        for (int y = 0; y < bitmap1.size(); ++y) {
            for (int x = 0; x < bitmap1[y].size(); ++x) {
                if (bitmap1[y][x] == 1) {
                    // Calculate the coordinates of the rectangle to fill
                    float left = x * squareSize;
                    float bottom = y * squareSize;
                    float right = (x + 1) * squareSize;
                    float top = (y + 1) * squareSize;

                    // Draw a filled rectangle for the obstacle
                    glColor3f(0.0, 0.0, 0.0); // Red color for obstacles
                    glRectf(left, bottom, right, top);
                }
            }
        }

        // Draw the border
        glColor3f(1.0, 1.0, 1.0); // White color for border
        for (int i = 0; i < border.size(); i = i + 4) {
            glRectf(border.at(i) * squareSize, border.at(i + 1) * squareSize, border.at(i + 2) * squareSize, border.at(i + 3) * squareSize);
        }
    }

    // Method to check if the food has been eaten
    bool Game::foodEaten(int x, int y, float pacmanX, float pacmanY) {
        float radius = 16.0 * cos(359 * M_PI / 180.0);
        return (x >= pacmanX - radius && x <= pacmanX + radius) &&
            (y >= pacmanY - radius && y <= pacmanY + radius);
    }

    //Method to draw all the food left and delete the eaten one
    void Game::drawFood(float pacmanX, float pacmanY) {
        vector<float> temp;
        for (size_t i = 0; i < foodPositions.size(); i += 2) {
            if (!foodEaten(foodPositions[i] * squareSize, foodPositions[i + 1] * squareSize, pacmanX, pacmanY)) {
                temp.push_back(foodPositions[i]);
                temp.push_back(foodPositions[i + 1]);
            }
            else {
                points++;
            }
        }
        foodPositions = std::move(temp);

        glPointSize(5.0);
        glBegin(GL_POINTS);
        glColor3f(1.0, 1.0, 1.0);
        for (size_t j = 0; j < foodPositions.size(); j += 2) {
            glVertex2f(foodPositions[j] * squareSize, foodPositions[j + 1] * squareSize);
        }
        glEnd();
    }

    // Method to reset the game state
    void Game::resetGame() {
        over = false;
        xIncrementp = 0;
        yIncrementp = 0;
        xIncrementg = 1.5;
        yIncrementg = 1.5; 
        rotation = 0;
        points = 0;

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
    
    // Method to update the movement of the pacman according to the movement keys pressed
    void Game::keyOperations() {

        x_p = (1.5 + xIncrementp) * squareSize;
        y_p = (1.5 + yIncrementp) * squareSize;

        // Update according to keys pressed
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

        x_g = (1.5 + xIncrementg) * squareSize;
        y_g = (1.5 + yIncrementg) * squareSize;

        if (keyStates[LEFT_ARROW]) { xIncrementg -= 1.25 ; }
        
        if (keyStates[RIGHT_ARROW]) { xIncrementg += 1.25 ; }
            
        if (keyStates[UP_ARROW]) { yIncrementg -= 1.25 ; }
        
        if (keyStates[DOWN_ARROW]) { yIncrementg += 1.25 ; }

        if (keyStates[' ']) {
            if (!replay && over) {
                pacman.rotate(0);
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
        if (points == 105) {
            over = true;
            return;
        }

        if (xIncrementp == 0 && yIncrementp == 0 && xIncrementg == 1.5 && yIncrementg == 1.5) { return; }

        // Check if Pacman collides with a ghost
        if (x_p == x_g && y_p == y_g) {
            over = true;
            return;
        }
    }

    // Method to display the results of the game at the ends
    void Game::resultsDisplay() {
        glClearColor(0, 0, 0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);
        if (points == 105) {
            //Won
            const char* message = "*************************************";
            glRasterPos2f(170, 250);
            while (*message)
                glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *message++);
            message = "CONGRATULATIONS, PACMAN, YOU WON! ";
            glColor3f(1, 1, 1);
            glRasterPos2f(150, 300);
            while (*message)
                glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *message++);
            message = "*************************************";
            glRasterPos2f(170, 350);
            while (*message)
                glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *message++);
            message = "To start or restart the game, press the letter R.";
            glRasterPos2f(170, 550);
            while (*message)
                glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *message++);
        } else {
            //Lost
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
            message = "To start or restart the game, press the R key.";
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
        message = "To control Pacman use WASD.";
        glRasterPos2f(200, 400);
        while(*message)
            glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *message++);
        message = "To control the ghost use arrow keys.";
        glRasterPos2f(200, 450);
        while (*message)
            glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *message++);
        message = "To start the game, press the space key twice.";
        glRasterPos2f(200, 500);
        while (*message)
            glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *message++);
        message = "To reset the game, press the R key.";
        glRasterPos2f(200, 550);
        glFlush();
    }

    // Method to display the screen and its elements
    void Game::display() {
        if (this->points ==1) { this->over = false; }
        this->keyOperations();
        glClear(GL_COLOR_BUFFER_BIT);
        this->gameOver();
        if (this->replay) {
            if (!this->over) {
                this->drawLaberynth();
                this->drawFood((1.5 + this->xIncrementp) * this->squareSize, (1.5 + this->yIncrementp) * this->squareSize);
                this->pacman.draw(1.5 + this->xIncrementp, 1.5 + this->yIncrementp, this->rotation);
                this->ghost.draw(1.5 + this->xIncrementg, 1.5 + this->yIncrementg);

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
std::unique_ptr<Pacman> pacmanPtr(new Pacman);
std::unique_ptr<Ghost> ghostPtr(new Ghost);

float squareSize = 50.0;
        
Game game(*pacmanPtr, *ghostPtr);

    // Define static functions
    void displayCallback() { game.display(); }
    void reshapeCallback(int w, int h) { game.reshape(w, h); }
    void keyPressedCallback(unsigned char key, int x, int y){
        std::cout << "Pressed key: " << static_cast<int>(key) << std::endl;
        game.keyStates[key] = true;  
        }
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
    glutInitWindowSize(750, 750);
    glutInitWindowPosition(500, 50);
    glutCreateWindow("Pacman vs. Ghost");
    

    // Set GLUT callbacks
    glutKeyboardFunc(keyPressedCallback);
    glutKeyboardUpFunc(keyUpCallback);
    glutDisplayFunc(displayCallback);
    glutReshapeFunc(reshapeCallback);
    glutIdleFunc(displayCallback);

    glutSpecialFunc(specialKeyPressedCallback);
    glutSpecialUpFunc(specialKeyUpCallback);

    game.init();

    glutMainLoop();
    return 0;

}