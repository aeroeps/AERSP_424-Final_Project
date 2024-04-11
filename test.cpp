#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl.h>
#include <GLUT/glut.h>
#include <iostream>
#include <vector>
#include <deque>
#include <cmath>

using namespace std;
float squareSize;
vector<vector<bool>> bitmap;

class Drawable {
public:
    virtual void draw() const = 0;
};

class Obstacle : public Drawable {
private:
    vector<int> coordinates;
    float squareSize; // Declare squareSize as a member variable

public:
    Obstacle(const vector<int>& coords) : coordinates(coords) {}

    void draw() const override {
        glColor3f(1.0, 1.0, 1.0);
        for (size_t i = 0; i < coordinates.size(); i +=4) {
            glRectf(coordinates[i] * squareSize, coordinates[i+1] * squareSize, coordinates [i + 2] * squareSize, coordinates[i + 3] * squareSize);
        }
    }
};

class Monster : public Drawable {
private:
    float positionX;
    float positionY;
    float colorR;
    float colorG;
    float colorB;
    float direction;

public:
    Monster(float posX, float posY, float r, float g, float b) : positionX(posX), positionY(posY), colorR(r), colorG(g), colorB(b) {}

    float getPositionX() const { return positionX; }

    float getPositionY() const { return positionY; }

    void draw() const override {
        int x, y;
        glBegin(GL_LINES);
        glColor3f(colorR, colorG, colorB);
        // Draw the head
        for (int k = 0; k < 32; k++) {
            x = (float)k / 2.0 * cos(360 * M_PI / 180.0) + (positionX);
            y = (float)k / 2.0 * sin(360 * M_PI / 180.0) + (positionY);
            for (int i = 180; i <= 360; i++) {
                glVertex2f(x, y);
                x = (float)k / 2.0 * cos(i * M_PI / 180.0) + (positionX);
                y = (float)k / 2.0 * sin(i * M_PI / 180.0) + (positionY);
                glVertex2f(x, y);
            }
        }
        glEnd();

        // Draw body
        glRectf((positionX) - 17, positionY, (positionX) + 15, (positionY) + 15);
        
        // Draw eyes and legs
        glBegin(GL_POINTS);
        glColor3f(0, 0.2, 0.4);
        glVertex2f((positionX) - 11, (positionY) + 14); // Legs
        glVertex2f((positionX) - 1, (positionY) + 14);  // Legs
        glVertex2f((positionX) + 8, (positionY) + 14);  // Legs
        glVertex2f((positionX) + 4, (positionY) - 3);   // Eyes
        glVertex2f((positionX) - 7, (positionY) - 3);   // Eyes
        glEnd();
    }
    
    // Method to update the position of the monster randomly
    void updatePosition() {
        int x1Quadrant = (int)((positionX - (2/squareSize)) - (16.0 *cos(360 * M_PI / 180.0)) / squareSize);
        int x2Quadrant = (int)((positionX + (2/squareSize)) + (16.0 *cos(360 * M_PI / 180.0)) / squareSize);
        int y1Quadrant = (int)((positionY - (2/squareSize)) - (16.0 *cos(360 * M_PI / 180.0)) / squareSize);
        int y2Quadrant = (int)((positionY + (2/squareSize)) + (16.0 *cos(360 * M_PI / 180.0)) / squareSize);
        
        switch ((int)direction) {
            case 1:
                if (!bitmap.at(x1Quadrant).at((int)positionY)) {
                    positionX -= 2 / squareSize;
                } else {
                    direction = (rand() % 4) + 1;
                }
                break;
            case 2:
                if (!bitmap.at(x2Quadrant).at((int)positionY)) {
                    positionX += 2 / squareSize;
                } else {
                    direction = (rand() % 4) + 1;
                }
                break;
            case 3:
                if (!bitmap.at((int)positionX).at(y1Quadrant)) {
                    positionY -= 2 / squareSize;
                } else {
                    direction = (rand() % 4) + 1;
                }
                break;
            case 4:
                if (!bitmap.at((int)positionX).at(y2Quadrant)) {
                    positionY += 2 / squareSize;
                } else {
                    direction = (rand() % 4) + 1;
                }
                break;
            default:
                break;
        }
    }
};

class Pacman {
private:
    float positionX;
    float positionY;
    float rotation;

public:
    Pacman(float posX, float posY, float rot) : positionX(posX), positionY(posY), rotation(rot) {}

    float getPositionX() const { return positionX; }

    float getPositionY() const { return positionY; }

    void draw() const {
        int x, y;
        glBegin(GL_LINES);
        glColor3f(1.0, 1.0, 0.0);
        for (int k = 0; k < 32; k++) {
            x = (float)k / 2.0 * cos((30 + 90 * rotation) * M_PI / 180.0) + (positionX);
            y = (float)k / 2.0 * sin((30 + 90 * rotation) * M_PI / 180.0) + (positionY);
            for (int i = 30; i < 330; i++) {
                glVertex2f(x, y);
                x = (float)k / 2.0 * cos((i + 90 * rotation) * M_PI / 180.0) + (positionX);
                y = (float)k / 2.0 * sin((i + 90 * rotation) * M_PI / 180.0) + (positionY);
                glVertex2f(x, y);
            }
        }
        glEnd();
    }
};

class Game {
private:
    Pacman& pacman;
    bool replay;
    bool over;
    float squareSize;
    float xIncrement;
    float yIncrement;
    int rotation;
    vector<int> border;
    vector<int> obstaclesTop;
    vector<int> obstaclesMiddle;
    vector<int> obstaclesBottom;
    deque<float> food;
    vector<vector<bool>> bitmap;
    bool* keyStates;
    int points = 0;
    vector<Monster> monsters;
    vector<Drawable*> drawables;
    vector<float> foodPositions;

public:
Game(Pacman& p) : pacman(p), replay(false), over(true), squareSize(50.0), xIncrement(0), yIncrement(0), rotation(0), points(0) {
    keyStates = new bool[256];
}

~Game() {
    delete[] keyStates;
    for (auto drawable : drawables) {
        delete drawable;
    }
}

    void init() {
        // Clear screen
        glClearColor(0.0, 0.0, 0.0, 0.0);
        glShadeModel(GL_FLAT);

        // Reset all keys
        for (int i = 0; i < 256; i++) {
            keyStates[i] = false;
        }

        // Initialize bitmap with obstacles
        bitmap.push_back({ true, true, true, true, true, true, true, true, true, true, true, true, true, true, true });
        // Add more bitmap initialization here

        // Initialize monsters
        monsters.push_back(Monster(10.5, 8.5, 0.0, 0.0, 0.1)); // blue colored monster
        // Add more monster initialization here

        // Initialize obstacles
        drawables.push_back(new Obstacle(obstaclesTop));
        drawables.push_back(new Obstacle(obstaclesMiddle));
        drawables.push_back(new Obstacle(obstaclesBottom));
    }

    void drawLaberynth() const {
        for (auto drawable : drawables) {
            drawable->draw();
        }
    }

    // Method to check if the food has been eaten
    bool foodEaten(int x, int y, float pacmanX, float pacmanY) {
        float radius = 16.0 * cos(359 * M_PI / 180.0);
        return (x >= pacmanX - radius && x <= pacmanX + radius) &&
               (y >= pacmanY - radius && y <= pacmanY + radius);
    }

    //Method to draw all the food left and delete the ate one
    void drawFood(float pacmanX, float pacmanY) {
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

    // Method to draw all the monsters
    void drawMonsters() const {
        for (const auto& monster : monsters) {
            monster.draw();
        }
    }

    vector<Monster>& getMonsters() { return monsters; }

    // Method to set the pressed key
    void keyPressed(unsigned char key, int x, int y) {
        keyStates[key] = true;
    }

    // Method to unset the released key
    void keyUp(unsigned char key, int x, int y) {
        keyStates[key] = false;
    }

    // Method to reset the game state
    void resetGame() {
        over = false;
        xIncrement = 0;
        yIncrement = 0; 
        rotation = 0;
        
        // Reset monster positions
        monsters.clear();
        monsters.push_back(Monster(10.5, 8.5, 0.0, 0.0, 1.0)); // Monster 1
        monsters.push_back(Monster(13.5, 1.5, 0.0, 0.0, 1.0)); // Monster 2
        monsters.push_back(Monster(4.5, 6.5, 0.0, 0.0, 1.0)); // Monster 3
        monsters.push_back(Monster(2.5, 13.5, 0.0, 0.0, 1.0)); // Monster 4
        points = 0;
        
        // Reset key states
        for (int i = 0; i < 256; i++){
            keyStates[i] = false;
        }
        
        // Reset food positions
        foodPositions = { 
            1.5, 1.5, 1.5, 2.5, 1.5, 3.5, 1.5, 4.5, 1.5, 5.5, 1.5, 6.5, 1.5, 7.5, 1.5, 8.5, 1.5, 9.5, 1.5, 10.5, 1.5, 11.5, 1.5, 12.5, 1.5, 13.5, 
            2.5, 1.5, 2.5, 6.5, 2.5, 9.5, 2.5, 13.5, 3.5, 1.5, 3.5, 2.5, 3.5, 3.5, 3.5, 4.5, 3.5, 6.5, 3.5, 8.5, 3.5, 9.5, 3.5, 10.5, 3.5, 11.5, 3.5, 13.5, 
            4.5, 1.5, 4.5, 4.5, 4.5, 5.5, 4.5, 6.5, 4.5, 7.5, 4.5, 8.5, 4.5, 11.5, 4.5, 12.5, 4.5, 13.5, 5.5, 1.5, 5.5, 2.5, 5.5, 5.5, 5.5, 10.5, 5.5, 11.5, 5.5, 13.5, 
            6.5, 2.5, 6.5, 3.5, 6.5, 4.5, 6.5, 5.5, 6.5, 7.5, 6.5, 10.5, 6.5, 13.5, 7.5, 5.5, 7.5, 6.5, 7.5, 7.5, 7.5, 9.5, 7.5, 10.5, 7.5, 11.5, 7.5, 12.5, 7.5, 13.5, 
            8.5, 2.5, 8.5, 3.5, 8.5, 4.5, 8.5, 5.5, 8.5, 7.5, 8.5, 10.5, 8.5, 13.5, 9.5, 1.5, 9.5, 2.5, 9.5, 5.5, 9.5, 10.5, 9.5, 11.5, 9.5, 13.5, 10.5, 1.5, 10.5, 4.5, 10.5, 5.5, 
            10.5, 6.5, 10.5, 7.5, 10.5, 8.5, 10.5, 11.5, 10.5, 12.5, 10.5, 13.5, 11.5, 1.5, 11.5, 2.5, 11.5, 3.5, 11.5, 4.5, 11.5, 5.5, 11.5, 6.5, 11.5, 8.5, 11.5, 9.5, 11.5, 10.5, 11.5, 11.5, 11.5, 13.5, 
            12.5, 1.5, 12.5, 6.5, 12.5, 9.5, 12.5, 13.5, 13.5, 1.5, 13.5, 2.5, 13.5, 3.5, 13.5, 4.5, 13.5, 5.5, 13.5, 6.5, 13.5, 7.5, 13.5, 8.5, 13.5, 9.5, 13.5, 10.5, 13.5, 11.5, 13.5, 12.5, 13.5 
        };
    }

    // Method to update the movement of the pacman according to the movement keys pressed
    void keyOperations() {
        // Get current position
        float x = (1.5 + xIncrement) * squareSize;
        float y = (1.5 + yIncrement) * squareSize;
        
        // Update according to keys pressed
        if (keyStates['a']) {
            x -= 2;
            int x1Quadrant = (int)((x - 16.0 *cos(360 * M_PI / 180.0)) / squareSize);
            if (!bitmap.at(x1Quadrant).at((int)y/squareSize)){
                xIncrement -= 2 / squareSize;
                rotation = 2;
            }
        }
        if (keyStates['d']) {
            x += 2;
            int x2Quadrant = (int)((x + 16.0 *cos(360 * M_PI / 180.0)) / squareSize);
            if (!bitmap.at(x2Quadrant).at((int)y / squareSize)){
                xIncrement += 2 / squareSize;
                rotation = 0;
            }
        }
        if (keyStates['w']) {
            y -= 2;
            int y1Quadrant = (int)((y - 16.0 *cos(360 * M_PI / 180.0)) / squareSize);
            if (!bitmap.at((int)x/squareSize).at(y1Quadrant)){
                yIncrement -= 2 / squareSize;
                rotation = 3;
            }
        }
        if (keyStates['s']) {
            y += 2;
            int y2Quadrant = (int)((y + 16.0 *cos(360 * M_PI / 180.0)) / squareSize);
            if (!bitmap.at((int)x / squareSize).at(y2Quadrant)){
                yIncrement += 2 / squareSize;
                rotation = 1;
            }
        }
        if (keyStates[' ']) {
            if (!replay && over){
                resetGame();
                replay = true;
            }
            else if (replay && over){
                replay = false;
            }
        }
    }

    // Method to check if the game is over
    void gameOver() {
        int pacmanX = (int)(1.5 + xIncrement);
        int pacmanY = (int)(1.5 + yIncrement);
        for (const auto& monster : monsters) {
            int monsterX = (int)monster.getPositionX();
            int monsterY = (int)monster.getPositionY();
            if (pacmanX == monsterX && pacmanY == monsterY) {
                over = true;
                return;
            }
        }
        if (points == 106) {
            over = true;
        }
    }

    // Method to display the results of the game at the ends
    void resultsDisplay() {
        glClearColor(0, 0, 0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);
        if (points == 106) {
            //Won
            const char* message = "*************************************";
            glRasterPos2f(170, 250);
            while (*message)
                glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *message++);
            message = "CONGRATULATIONS, YOU WON! ";
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
            //Lost
            const char* message = "*************************";
            glRasterPos2f(210, 250);
            while (*message)
                glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *message++);
            message = "SORRY, YOU LOST ... ";
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
    void welcomeScreen() {
        glClearColor(0, 0.2, 0.4, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);
        const char* message = "*************************************";
        glRasterPos2f(150, 200);
        while (*message)
            glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *message++);
        message = "PACMAN";
        glColor3f(1, 1, 1);
        glRasterPos2f(225, 250);
        while (*message)
            glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *message++);
        message = "*************************************";
        glRasterPos2f(150, 300);
        while (*message)
            glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *message++);
        message = "To control Pacman use A to go right, D to go left, W to go up and S to go down.";
        glRasterPos2f(50, 400);
        while (*message)
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *message++);
        message = "To start or restart the game, press the space key.";
        glRasterPos2f(170, 450);
        while (*message)
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *message++);
        glFlush();
    }

    // Method to display the screen and its elements
    void display() {
        if (this->points ==1) { this->over = false; }

        this->keyOperations();
        glClear(GL_COLOR_BUFFER_BIT);
        this->gameOver();
        if (this->replay) {
            if (!this->over) {
                this->drawLaberynth();
                this->drawFood((1.5 + this->xIncrement) * this->squareSize, (1.5 + this->yIncrement) * this->squareSize);
                this->pacman.draw();
                for (auto& monster : this->getMonsters()) {
                    monster.updatePosition();
                    monster.draw();
                }
            } else {
                this->resultsDisplay();
            }
        } else {
            this->welcomeScreen();
        }
        glutSwapBuffers();
    }

    // Method to reshape the game if the screen size changes
    void reshape(int w, int h) {
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glViewport(0, 0, (GLsizei)w, (GLsizei)h);
        glOrtho(0, 750, 750, 0, -1.0, 1.0);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
    }
};

// Declare the game object globally
Game game(*(new Pacman(0,0,0)));

// Define static functions
void keyPressedCallback(unsigned char key, int x, int y) {
    game.keyPressed(key, x, y);
}

void keyUpCallback(unsigned char key, int x, int y) {
    game.keyUp(key, x, y);
}

void displayCallback() {
    game.display();
}

void reshapeCallback(int w, int h) {
    game.reshape(w, h);
}


int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(800, 600);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Pacman");
    
    // Pacman object
    Pacman pacman(400, 300, 0);

    // Game object
    Game game(pacman);
    game.init();

    // Set GLUT callbacks
    glutKeyboardFunc(keyPressedCallback);
    glutKeyboardUpFunc(keyUpCallback);
    glutDisplayFunc(displayCallback);
    glutReshapeFunc(reshapeCallback);

    glutMainLoop();
    return 0;
}