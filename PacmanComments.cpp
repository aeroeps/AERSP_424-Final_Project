// Use preprocessor macros/ directives to make the code more readable
#define GL_SILENCE_DEPRECATION
#define MAP_SIZE 15

// Include the appropriate headers for using OpenGL and GLUT-specific OpenGL graphics.
#include <OpenGL/gl.h>
#include <GLUT/glut.h>

// Import standard C++ libraries to reference throughout the project. 
#include <iostream>
#include <vector>
#include <deque>
#include <cmath>
#include <thread>
#include <atomic>

// Increase the readability of the project by more conveniently accessing standard library functions and objects.
using namespace std;

class Drawable {
public:
    // Virtual destructor to destroy objects of class Drawable
    virtual ~Drawable() {} 

    // Pure virtual draw() function allows for polymorphism
    virtual void draw() const = 0; 
};


class Obstacle : public Drawable {
private:
    // Private member variable to store coordinates of the obstacle
    vector<int> coordinates; 

public:
    // Default constructor initializing coordinates to an empty vector
    Obstacle() : coordinates({}) {}

    // Constructor initializing coordinates with the provided vector of integers
    Obstacle(const vector<int>& coords) : coordinates(coords) {}

    // Defines the square size to be drawn
    static constexpr float squareSize = 50.0;

    // Overridden draw() function from the base class (Drawable)
    void draw() const override {
        // Set the drawing color to white (RGB: 1.0, 1.0, 1.0)
        glColor3f(1.0, 1.0, 1.0);

        // Iterate over coordinates, drawing rectangles based on coordinate pairs
        for (size_t i = 0; i < coordinates.size(); i += 4) {
            glRectf(coordinates[i] * squareSize,   // x of bottom left corner
                coordinates[i + 1] * squareSize,   // y of bottom left corner
                coordinates[i + 2] * squareSize,   // x of top right corner
                coordinates[i + 3] * squareSize);  // y of top right corner
        }
    }
};


class Monster : public Drawable {
private:
    float positionX;   // x position
    float positionY;   // y position
    float colorR;      
    float colorG;      
    float colorB;      
    float direction;   

public:
    // Constructor to initialize Monster with specified position and color
    Monster(float posX, float posY, float r, float g, float b)
        : positionX(posX), positionY(posY), colorR(r), colorG(g), colorB(b) {}

    // Getter for retrieving the monster's x position
    float getPositionX() const { return positionX; }

    // Getter for retrieving the monster's y position
    float getPositionY() const { return positionY; }

    // Overridden draw() function from the base class (Drawable)
    void draw() const override {
        int x, y;
        glBegin(GL_LINES);
        glColor3f(colorR, colorG, colorB);  // Set drawing color based on monster's color components

        // Draw the head of the monster
        for (int k = 0; k < 32; k++) {
            x = (float)k / 2.0 * cos(360 * M_PI / 180.0) + positionX;
            y = (float)k / 2.0 * sin(360 * M_PI / 180.0) + positionY;
            for (int i = 180; i <= 360; i++) {
                glVertex2f(x, y);
                x = (float)k / 2.0 * cos(i * M_PI / 180.0) + positionX;
                y = (float)k / 2.0 * sin(i * M_PI / 180.0) + positionY;
                glVertex2f(x, y);
            }
        }
        glEnd();

        // Draw the body of the monster using OpenGL rectangles
        glRectf(positionX - 17, positionY, positionX + 15, positionY + 15);

        // Draw eyes and legs of the monster
        glBegin(GL_POINTS);
        glColor3f(0, 0.2, 0.4);  // Set the color for the eyes and legs
        glVertex2f(positionX - 11, positionY + 14); // Leg
        glVertex2f(positionX - 1, positionY + 14);  // Leg
        glVertex2f(positionX + 8, positionY + 14);  // Leg
        glVertex2f(positionX + 4, positionY - 3);   // Eye
        glVertex2f(positionX - 7, positionY - 3);   // Eye
        glEnd();
    }

    // Method to randomly update the monster's position
    void updatePosition() {
        // Calculate quadrants of the map based on current position and direction
        int x1Quadrant = (int)((positionX - (2 / squareSize)) - (16.0 * cos(360 * M_PI / 180.0)) / squareSize);
        int x2Quadrant = (int)((positionX + (2 / squareSize)) + (16.0 * cos(360 * M_PI / 180.0)) / squareSize);
        int y1Quadrant = (int)((positionY - (2 / squareSize)) - (16.0 * cos(360 * M_PI / 180.0)) / squareSize);
        int y2Quadrant = (int)((positionY + (2 / squareSize)) + (16.0 * cos(360 * M_PI / 180.0)) / squareSize);

        // Update position based on current direction and collision detection with bitmap
        switch ((int)direction) {
        case 1:
            if (!bitmap.at(x1Quadrant).at((int)positionY)) {
                positionX -= 2 / squareSize;
            }
            else {
                direction = (rand() % 4) + 1;
            }
            break;
        case 2:
            if (!bitmap.at(x2Quadrant).at((int)positionY)) {
                positionX += 2 / squareSize;
            }
            else {
                direction = (rand() % 4) + 1;
            }
            break;
        case 3:
            if (!bitmap.at((int)positionX).at(y1Quadrant)) {
                positionY -= 2 / squareSize;
            }
            else {
                direction = (rand() % 4) + 1;
            }
            break;
        case 4:
            if (!bitmap.at((int)positionX).at(y2Quadrant)) {
                positionY += 2 / squareSize;
            }
            else {
                direction = (rand() % 4) + 1;
            }
            break;
        default:
            break;
        }
    }
};



class Pacman : public Obstacle {
private:
    atomic<float> positionX;  // Atomic variable for Pacman's x coord.
    atomic<float> positionY;  // Atomic variable for Pacman's y coord.
    atomic<int> rotation;     // Atomic variable for Pacman's rotation angle

public:
    // Constructor to initialize Pacman's position and rotation
    Pacman() : positionX(0.0), positionY(0.0), rotation(0) {}

    // Draw Pacman at specified position and rotation
    void draw(float posX, float posY, float rot) {
        // Logic for drawing Pacman using OpenGL
        glBegin(GL_LINES);
        glColor3f(1.0, 1.0, 0.0);  // Set color to yellow

        int x, y;
        for (int k = 0; k < 32; k++) {
            // Calculate coordinates based on rotation and position
            x = (float)k / 2.0 * cos((30 + 90 * rotation) * M_PI / 180.0) + (positionX * squareSize);
            y = (float)k / 2.0 * sin((30 + 90 * rotation) * M_PI / 180.0) + (positionY * squareSize);

            for (int i = 30; i < 330; i++) {
                // Draw Pacman's shape
                glVertex2f(x, y);
                x = (float)k / 2.0 * cos((i + 90 * rotation) * M_PI / 180.0) + (positionX * squareSize);
                y = (float)k / 2.0 * sin((i + 90 * rotation) * M_PI / 180.0) + (positionY * squareSize);
                glVertex2f(x, y);
            }
        }
        glEnd();
    }

    // Set Pacman's rotation angle
    void rotate(int angle) { rotation = angle; }

    // Move Pacman in small increments
    void move(float xIncrement, float yIncrement) {
        float oldX = positionX.load();  // Load current x
        float oldY = positionY.load();  // Load current y
        positionX.store(oldX + xIncrement);  // Update x
        positionY.store(oldY + yIncrement);  // Update y
    }

    // Set Pacman's initial position
    void setInitialPosition(float x, float y) {
        positionX = x;  // Set initial x
        positionY = y;  // Set initial y
    }
};


class Game {
private:
    Pacman& pacman;                 // Reference to the Pacman object
    bool replay;                    // Flag indicating if game should be restarted
    bool over;                      // Flag indicating if game is over
    float squareSize;               // Size of each square in the game grid
    float xIncrement;               // X-increment for Pacman's movement
    float yIncrement;               // Y-increment for Pacman's movement
    int rotation;                   // Rotation state for Pacman
    vector<int> border;             // Coordinates of the border walls
    vector<int> obstaclesTop;       // Coordinates of top obstacles
    vector<int> obstaclesMiddle;    // Coordinates of middle obstacles
    vector<int> obstaclesBottom;    // Coordinates of bottom obstacles
    deque<float> food;              // Positions of food
    vector<vector<bool>> bitmap;    // Vector of objects to be drawn
    int points = 0;                 // Points earned in the game

    vector<Monster> monsters;       // Vector of Monster objects
    vector<Drawable*> drawables;    // Vector of objects to be drawn

public:
    vector<float> foodPositions;    // Positions of remaining food
    bool* keyStates;                // Array representing key states

    // Constructor to initialize the game with a Pacman object
    Game(Pacman& p) : pacman(p), replay(false), over(true), squareSize(50.0), xIncrement(0), yIncrement(0), rotation(0), points(0) {

        keyStates = new bool[256];  // Allocate memory for key states

        // Initialize the game grid
        bitmap = {  { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 },
                    { 1,0,0,0,0,0,0,0,0,0,0,0,0,0,1 },
                    { 1,0,0,1,0,1,0,1,1,1,1,1,0,0,1 },
                    { 1,0,1,1,0,1,0,1,1,1,1,1,1,0,1 },
                    { 1,0,1,0,0,0,0,1,0,0,0,0,1,0,1 },
                    { 1,0,1,1,0,1,1,1,1,0,1,0,1,0,1 },
                    { 1,0,0,0,0,0,0,1,0,0,1,0,0,0,1 },
                    { 1,0,1,1,1,1,0,1,1,1,1,1,1,0,1 },
                    { 1,0,1,0,0,1,0,0,0,0,0,0,0,0,1 },
                    { 1,0,1,1,1,1,0,1,1,1,1,1,1,0,1 },
                    { 1,0,0,0,0,1,0,1,0,0,0,0,1,0,1 },
                    { 1,1,1,1,0,1,0,1,1,1,1,0,1,0,1 },
                    { 1,0,0,1,0,1,0,0,0,0,1,0,0,0,1 },
                    { 1,0,1,1,1,1,1,1,1,1,1,1,1,0,1 },
                    { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 } };

        // Set initial position of Pacman
        pacman.setInitialPosition(1.5, 1.5);
    }

    // Destructor to clean up memory allocated for drawables
    ~Game() {
        for (auto drawable : drawables) {
            delete drawable;
        }
    }

    // Initialize the game state
    void init() {
        // Clear screen
        glClearColor(0.0, 0.0, 0.0, 0.0);
        glShadeModel(GL_FLAT);

        // Reset key states
        for (int i = 0; i < 256; i++) { keyStates[i] = false; }

        // Initialize monsters
        monsters.push_back(Monster(10.5, 8.5, 0.0, 0.0, 0.1));
        Add more monster initialization here

        // Initialize obstacles
        drawables.push_back(new Obstacle(obstaclesTop));
        drawables.push_back(new Obstacle(obstaclesMiddle));
        drawables.push_back(new Obstacle(obstaclesBottom));
    }

    // Draw the game labyrinth
    void drawLaberynth() {
        // Set the coordinates of the border walls
        static vector<int> border = { 0, 0, 15, 1, 15, 15, 14, 1, 0, 14, 15, 15, 1, 14, 0, 0 };
        
        // Draw border walls
        glColor3f(1.0, 1.0, 1.0);

        for (int i = 0; i < border.size(); i = i + 4) {
            glRectf(border.at(i) * squareSize, border.at(i + 1) * squareSize, border.at(i + 2) * squareSize, border.at(i + 3) * squareSize);
        }

        // Draw bottom obstacles
        for (int j = 0; j < obstaclesBottom.size(); j = j + 4) {
            glRectf(obstaclesBottom.at(j) * squareSize, obstaclesBottom.at(j + 1) * squareSize, obstaclesBottom.at(j + 2) * squareSize, obstaclesBottom.at(j + 3) * squareSize);
        }

        // Draw middle obstacles
        for (int k = 0; k < obstaclesMiddle.size(); k = k + 4) {
            glRectf(obstaclesMiddle.at(k) * squareSize, obstaclesMiddle.at(k + 1) * squareSize, obstaclesMiddle.at(k + 2) * squareSize, obstaclesMiddle.at(k + 3) * squareSize);
        }

        // Draw top obstacles
        for (int p = 0; p < obstaclesTop.size(); p = p + 4) {
            glRectf(obstaclesTop.at(p) * squareSize, obstaclesTop.at(p + 1) * squareSize, obstaclesTop.at(p + 2) * squareSize, obstaclesTop.at(p + 3) * squareSize);
        }

        // Draw spaces between food
        for (size_t row = 0; row < bitmap.size(); row++) {
            for (size_t col = 0; col < bitmap[row].size(); col++) {
                if (!bitmap[row][col]) {
                    glRectf(col * squareSize, row * squareSize, (col + 1) * squareSize, (row + 1) * squareSize);
                }
            }
        }
    }

    // Check if food is eaten and update points
    bool foodEaten(int x, int y, float pacmanX, float pacmanY) {
        float radius = 16.0 * cos(359 * M_PI / 180.0);
        return (x >= pacmanX - radius && x <= pacmanX + radius) &&
            (y >= pacmanY - radius && y <= pacmanY + radius);
    }

    // Draw remaining food and update points
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

    // Draw all the monsters
    void drawMonsters() const {
        for (const auto& monster : monsters) {
            monster.draw();
        }
    }

    vector<Monster>& getMonsters() { return monsters; }

    // Handle key presses
    void keyPressed(unsigned char key, int x, int y) {
        keyStates[key] = true;
    }

    // Handle key releases
    void keyUp(unsigned char key, int x, int y) {
        keyStates[key] = false;
    }

    // Reset the game state
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
        for (int i = 0; i < 256; i++) {
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

    // Update Pacman's movement based on key presses
    void keyOperations() {

        // Get current position
        float x = (1.5 + xIncrement) * squareSize;
        float y = (1.5 + yIncrement) * squareSize;

        // Update based on left key movement
        if (keyStates['a']) {
            x -= 2;
            int x1Quadrant = (int)((x - 16.0 * cos(360 * M_PI / 180.0)) / squareSize);
            if (!bitmap.at(x1Quadrant).at((int)y / squareSize)) {
                xIncrement -= 2 / squareSize;
                rotation = 2;
            }
        }
        // Update based on right key movement
        if (keyStates['d']) {
            x += 2;
            int x2Quadrant = (int)((x + 16.0 * cos(360 * M_PI / 180.0)) / squareSize);
            if (!bitmap.at(x2Quadrant).at((int)y / squareSize)) {
                xIncrement += 2 / squareSize;
                rotation = 0;
            }
        }
        // Update based on up key movement
        if (keyStates['w']) {
            y -= 2;
            int y1Quadrant = (int)((y - 16.0 * cos(360 * M_PI / 180.0)) / squareSize);
            if (!bitmap.at((int)x / squareSize).at(y1Quadrant)) {
                yIncrement -= 2 / squareSize;
                rotation = 3;
            }
        }
        // Update based on down key movement
        if (keyStates['s']) {
            y += 2;
            int y2Quadrant = (int)((y + 16.0 * cos(360 * M_PI / 180.0)) / squareSize);
            if (!bitmap.at((int)x / squareSize).at(y2Quadrant)) {
                yIncrement += 2 / squareSize;
                rotation = 1;
            }
        }
        // Handle the space key for the game restart
        if (keyStates[' ']) {
            if (!replay && over) {
                resetGame();
                replay = true;
            }
            else if (replay && over) {
                replay = false;
            }
        }
    }

    // Check if the game is over
    void gameOver() {
        int pacmanX = (int)(1.5 + xIncrement);
        int pacmanY = (int)(1.5 + yIncrement);

        // If Pacman collides with any monster, game over
        for (const auto& monster : monsters) {
            int monsterX = static_cast<int>(monster.getPositionX());
            int monsterY = static_cast<int>(monster.getPositionY());

            if (pacmanX == monsterX && pacmanY == monsterY) {
                over = true;
                return; 
            }
        }

        // If all food is eaten, game over
        if (points == 106) {
            over = true; 
        }
    }

    // Display the results of the game at the ends
    void resultsDisplay() {

        glClearColor(0, 0, 0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);

        if (points == 106) {

            // Display win screen with points
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
        }

        else {

            // Display lost screen with points
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

    // Display the welcome screen with starting instructions
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

        message = "To start the game, press the space key twice.";
        glRasterPos2f(170, 450);

        while (*message)
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *message++);

        glFlush();
    }

    // Display the game and its elements
    void display() {
        if (this->points == 1) { this->over = false; }

        // Update game state based on key presses
        this->keyOperations();
        glClear(GL_COLOR_BUFFER_BIT);
        this->gameOver();
 
        // Draw game elements (labyrinth, food, Pacman, monsters)
        if (this->replay) {
            if (!this->over) {
                this->drawLaberynth();
                this->drawFood((1.5 + this->xIncrement) * this->squareSize, (1.5 + this->yIncrement) * this->squareSize);
                this->pacman.draw(1.5 + this->xIncrement, 1.5 + this->yIncrement, this->rotation);
                for (auto& monster : this->getMonsters()) {
                    monster.updatePosition();
                    monster.draw();
                }
            }
            else {
                // Display welcome screen or game results based on game state
                this->resultsDisplay();
            }
        }
        else {
            this->welcomeScreen();
        }
        glutSwapBuffers();
    }

    // Reshape the game if the screen size changes
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
Game game(*(new Pacman));

// Define static functions
void displayCallback() { game.display(); }
void reshapeCallback(int w, int h) { game.reshape(w, h); }
void keyPressedCallback(unsigned char key, int x, int y) { game.keyStates[key] = true; }
void keyUpCallback(unsigned char key, int x, int y) { game.keyStates[key] = false; }

int main(int argc, char** argv) {

    // Initialize GLUT
    glutInit(&argc, argv);

    // Set up display mode and window properties
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(750, 750);
    glutInitWindowPosition(500, 50);
    glutCreateWindow("Pacman");

    // Set GLUT callbacks
    glutKeyboardFunc(keyPressedCallback);   // Register key press callback
    glutKeyboardUpFunc(keyUpCallback);      // Register key release callback
    glutDisplayFunc(displayCallback);       // Register display callback
    glutReshapeFunc(reshapeCallback);       // Register reshape callback
    glutIdleFunc(displayCallback);          // Register idle function (display callback)

    // Instantiate a local Pacman object
    Pacman pacman(400, 300, 0);
    Pacman pacman;

    // Create a Game object with the Pacman object
    Game game(pacman);
    game.init();

    // Enter the GLUT event processing loop
    glutMainLoop();

    return 0;
}