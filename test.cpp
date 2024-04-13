#define GL_SILENCE_DEPRECATION
#define LEFT_ARROW 37
#define UP_ARROW 38
#define RIGHT_ARROW 39
#define DOWN_ARROW 40
#include <OpenGL/gl.h>
#include <GLUT/glut.h>
#include <iostream>
#include <vector>
#include <deque>
#include <cmath>
#include <thread>
#include <atomic>
#include <string>
#include <memory>

using namespace std;

class Drawable {
public:
    virtual ~Drawable() {} // Virtual destructor
    virtual void draw() const = 0;
};

class Obstacle : public Drawable {
private:
    vector<int> coordinates;

public:
    Obstacle() : coordinates({}) {} // Default constructor
    Obstacle(const vector<int>& coords) : coordinates(coords) {}
    static constexpr float squareSize = 50.0;
    void draw() const override {
        glColor3f(1.0, 1.0, 1.0);
        for (size_t i = 0; i < coordinates.size(); i +=4) {
            glRectf(coordinates[i] * squareSize, coordinates[i+1] * squareSize, coordinates [i + 2] * squareSize, coordinates[i + 3] * squareSize);
        }
    }
};

class Monster : public Obstacle {
private:
    atomic<float> positionX;
    atomic<float> positionY;
    float colorR;
    float colorG;
    float colorB;
    atomic<int> direction;
    std::string name;

public:
    Monster() : positionX(0.0), positionY(0.0), direction (0) {}

    Monster(float x, float y, float r, float g, float b)
        : positionX(x), positionY(y), colorR(r), colorG(g), colorB(b), direction(0) {}

    float getPositionX() const { return positionX; }
    float getPositionY() const { return positionY; }
    float getColorR() const { return colorR; }
    float getColorG() const { return colorG; }
    float getColorB() const { return colorB; }

    void draw(float posX, float posY, float r, float g, float b) const {
        int x, y;
        glBegin(GL_LINES);
        glColor3f(r, g, b);
        
        // Draw the head
        for (int k = 0; k < 32; k++) {
            x = (float)k / 2.0 * cos(360 * M_PI / 180.0) + (posX);
            y = (float)k / 2.0 * sin(360 * M_PI / 180.0) + (posY);
            for (int i = 180; i <= 360; i++) {
                glVertex2f(x, y);
                x = (float)k / 2.0 * cos(i * M_PI / 180.0) + (posX);
                y = (float)k / 2.0 * sin(i * M_PI / 180.0) + (posY);
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

    void setInitialPosition(float x, float y) { 
        positionX = x;
        positionY = y;
    }
};


class Pacman : public Obstacle {
private:
    atomic<float> positionX;
    atomic<float> positionY;
    atomic<int> rotation;
    // int x, y; // Pacman's position
public:
    Pacman() : positionX(0.0), positionY(0.0), rotation(0) {}


    void draw(float posX, float posY, float rot) {
        // Drawing logic for Pacman
        glBegin(GL_LINES);
        glColor3f(1.0, 1.0, 0.0);
        int x, y;
        for (int k = 0; k < 32; k++) {
            x = (float)k / 2.0 * cos((30 + 90 * rotation) * M_PI / 180.0) + (posX*squareSize);
            y = (float)k / 2.0 * sin((30 + 90 * rotation) * M_PI / 180.0) + (posY*squareSize);
            for (int i = 30; i < 330; i++) {
                glVertex2f(x, y);
                x = (float)k / 2.0 * cos((i + 90 * rotation) * M_PI / 180.0) + (posX*squareSize);
                y = (float)k / 2.0 * sin((i + 90 * rotation) * M_PI / 180.0) + (posY*squareSize);
                glVertex2f(x, y);
            }
        }
        glEnd();
    }

    void rotate(int angle) { rotation = angle; }

    void move(float xIncrement, float yIncrement) {
        float oldX = positionX.load();
        float oldY = positionY.load();
        positionX.store(oldX + xIncrement);
        positionY.store(oldY + yIncrement);
    }

    void setInitialPosition(float x, float y) { 
        positionX = x;
        positionY = y;
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
    vector<vector<bool>> bitmap1;
    int points = 0;
    vector<Monster>& monsters;
    vector<Drawable*> drawables;
    bool moveUp = false;
    bool moveDown = false;
    bool moveLeft = false;
    bool moveRight = false;
    float x;
    float y;

public:
    vector<float> foodPositions;
    bool* keyStates;

    Game(Pacman& p, vector<Monster>& m) : pacman(p), monsters(m), replay(false), over(true), squareSize(50.0), xIncrement(0), yIncrement(0), rotation(0), points(0) {
    
        keyStates = new bool[256];

        bitmap1 = {  { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 },
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

        pacman.setInitialPosition(1.5,1.5); // IMPORTANT. SET'S INITIAL POSITION

        // monsters.push_back(Monster(7.5, 8.5, 1.0, 0.0, 0.0)); // Red
        // monsters.push_back(Monster(8.5, 8.5, 0.0, 1.0, 0.0)); // Green
        // monsters.push_back(Monster(9.5, 8.5, 0.0, 0.0, 1.0));  // Blue
        // monsters.push_back(Monster(8.5, 9.5, 1.0, 1.0, 0.0)); // Yellow    
    
    }

    ~Game() {
        for (auto drawable : drawables) {
            delete drawable;
        }
    }
    
    void init() {
        // Clear screen
        //glClearColor(0.0, 0.0, 0.0, 0.0);
        //glShadeModel(GL_FLAT);

        // Reset all keys
        for (int i = 0; i < 256; i++) { keyStates[i] = false; }    
        
        // Initialize monsters
        // monsters.push_back(Monster(10.5, 8.5, 0.0, 0.0, 0.1)); // blue colored monster
        // Add more monster initialization here

        // Initialize obstacles
        drawables.push_back(new Obstacle(obstaclesTop));
        drawables.push_back(new Obstacle(obstaclesMiddle));
        drawables.push_back(new Obstacle(obstaclesBottom));
    }

    void drawLaberynth() {
        for (int y = 0; y < bitmap1.size(); ++y) {
            for (int x = 0; x < bitmap1[y].size(); ++x) {
                if (bitmap1[y][x] == 1) {
                    // Calculate the coordinates of the rectangle to fill
                    float left = x * squareSize;
                    float bottom = y * squareSize;
                    float right = (x + 1) * squareSize;
                    float top = (y + 1) * squareSize;

                    // Draw a filled rectangle for the obstacle
                    glColor3f(1.0, 0.0, 0.0); // Red color for obstacles
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
    bool foodEaten(int x, int y, float pacmanX, float pacmanY) {
        float radius = 16.0 * cos(359 * M_PI / 180.0);
        return (x >= pacmanX - radius && x <= pacmanX + radius) &&
               (y >= pacmanY - radius && y <= pacmanY + radius);
    }

    //Method to draw all the food left and delete the eaten one
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

    // Method to update the position of the monster randomly
    virtual void updatePosition(float* monster, int id) {
        int x1Quadrant = (int)((monster[0] - (2/squareSize)) - (16.0 *cos(360 * M_PI / 180.0)) / squareSize);
		int x2Quadrant = (int)((monster[0] + (2/squareSize)) + (16.0 *cos(360 * M_PI / 180.0)) / squareSize);
		int y1Quadrant = (int)((monster[1] - (2/squareSize)) - (16.0 *cos(360 * M_PI / 180.0)) / squareSize);
		int y2Quadrant = (int)((monster[1] + (2/squareSize)) + (16.0 *cos(360 * M_PI / 180.0)) / squareSize);
        
        switch ((int)monster[2]){
		case 1:
			if (!bitmap1.at(x1Quadrant).at((int)monster[1])){ 
				monster[0] -= 2 / squareSize;
			}else {
				int current = monster[2];
				do{
					monster[2] =  (rand() % 4) + 1;
				} while (current == (int) monster[2]);
			}
			break;
		case 2:
			if (!bitmap1.at(x2Quadrant).at((int)monster[1])){
				monster[0] += 2 / squareSize;
			}
			else {
				int current = monster[2];
				do{
					monster[2] = (rand() % 4) + 1;
				} while (current == (int)monster[2]);
			}
			break;
		case 3:
			if (!bitmap1.at((int)monster[0]).at(y1Quadrant)){
				monster[1] -= 2 / squareSize;
			}
			else {
				int current = monster[2];
				do{
					monster[2] = (rand() % 4) + 1;
				} while (current == (int)monster[2]);
			}
			break;
		case 4:
			if (!bitmap1.at((int)monster[0]).at(y2Quadrant)){
				monster[1] += 2 / squareSize;
			}
			else {
				int current = monster[2];
				do{
					monster[2] = (rand() % 4) + 1;
				} while (current == (int)monster[2]);
			}
			break;
		default:
			break;
		}
    }

    // Method to set the pressed key
    void keyPressed(unsigned char key, int x, int y) {
        // keyStates[key] = true;
        switch(key) {
            case 'w':
            case UP_ARROW:
                this->xIncrement = 0.0f;
                this->yIncrement = 1.5f;
                this->rotation = 90.0f;
                break;
            case 'a':
            case LEFT_ARROW:
                this->xIncrement = -1.5f;
                this->yIncrement = 0.0f;
                this->rotation = 180.0f;
                break;
            case 's':
            case DOWN_ARROW:
                this->xIncrement = 0.0f;
                this->yIncrement = -1.5f;
                this->rotation = 270.0f;
                break;
            case 'd':
            case RIGHT_ARROW:
                this->xIncrement = 1.5f;
                this->yIncrement = 0.0f;
                this->rotation = 0.0f;
                break;
            default:
                break;
    }
    }

    // Method to unset the released key
    void keyUp(unsigned char key, int x, int y) {
        // keyStates[key] = false;
        switch (key) {
            case 'w':
            case UP_ARROW:
                moveUp = false;
                break;
            case 's':
            case DOWN_ARROW:
                moveDown = false;
                break;
            case 'a':
            case LEFT_ARROW:
                moveLeft = false;
                break;
            case 'd':
            case RIGHT_ARROW:
                moveRight = false;
                break;
        }
    }

    // Method to reset the game state
    void resetGame() {
        over = false;
        xIncrement = 0;
        yIncrement = 0; 
        rotation = 0;

        pacman.setInitialPosition(1.5,1.5);
       
        // Reset each monster's position
        monsters[0].setInitialPosition(10.5, 8.5); // Blinky
        monsters[1].setInitialPosition(13.5, 1.5); // Pinky
        monsters[2].setInitialPosition(4.5, 6.5);  // Inky
        monsters[3].setInitialPosition(2.5, 13.5); // Clyde

        // Reset key states
        for (int i = 0; i < 256; i++){
            keyStates[i] = false;
        }
        
        // Reset food positions
        foodPositions = { 
            1.5, 1.5, 1.5, 2.5, 1.5, 3.5, 1.5, 4.5, 1.5, 5.5, 1.5, 6.5, 1.5, 7.5, 1.5, 8.5, 1.5, 9.5, 1.5, 10.5, 1.5, 11.5, 1.5, 12.5, 1.5, 13.5, 
            2.5, 1.5, 2.5, 6.5, 2.5, 9.5, 2.5, 13.5, 
            3.5, 1.5, 3.5, 2.5, 3.5, 3.5, 3.5, 4.5, 3.5, 6.5, 3.5, 8.5, 3.5, 9.5, 3.5, 10.5, 3.5, 11.5, 3.5, 13.5, 
            4.5, 1.5, 4.5, 4.5, 4.5, 5.5, 4.5, 6.5, 4.5, 7.5, 4.5, 8.5, 4.5, 11.5, 4.5, 12.5, 4.5, 13.5, 
            5.5, 1.5, 5.5, 2.5, 5.5, 5.5, 5.5, 10.5, 5.5, 13.5, 
            6.5, 2.5, 6.5, 3.5, 6.5, 4.5, 6.5, 5.5, 6.5, 7.5, 6.5, 10.5, 6.5, 13.5, 
            7.5, 5.5, 7.5, 6.5, 7.5, 7.5, 7.5, 9.5, 7.5, 10.5, 7.5, 11.5, 7.5, 12.5, 7.5, 13.5, 
            8.5, 2.5, 8.5, 3.5, 8.5, 4.5, 8.5, 5.5, 8.5, 7.5, 8.5, 10.5, 8.5, 13.5, 
            9.5, 1.5, 9.5, 2.5, 9.5, 5.5, 9.5, 10.5, 9.5, 11.5, 9.5, 13.5, 
            10.5, 1.5, 10.5, 4.5, 10.5, 5.5, 10.5, 6.5, 10.5, 7.5, 10.5, 8.5, 10.5, 11.5, 10.5, 12.5, 10.5, 13.5, 
            11.5, 1.5, 11.5, 2.5, 11.5, 3.5, 11.5, 4.5, 11.5, 6.5, 11.5, 8.5, 11.5, 9.5, 11.5, 10.5, 11.5, 11.5, 11.5, 13.5, 
            12.5, 1.5, 12.5, 6.5, 12.5, 9.5, 12.5, 13.5, 
            13.5, 1.5, 13.5, 2.5, 13.5, 3.5, 13.5, 4.5, 13.5, 5.5, 13.5, 6.5, 13.5, 7.5, 13.5, 8.5, 13.5, 9.5, 13.5, 10.5, 13.5, 11.5, 13.5, 12.5, 13.5, 13.5 
        };
    }
    
    // Method to update the movement of the pacman according to the movement keys pressed
    void keyOperations() {

        float x = (1.5 + xIncrement) * squareSize;
        float y = (1.5 + yIncrement) * squareSize;

        // Update according to keys pressed
        if (keyStates['a'] || keyStates[LEFT_ARROW]) {
            x -= 2;
            int x1Quadrant = (int)((x - 16.0 * cos(360 * M_PI / 180.0)) / squareSize);
            if (!this->bitmap1[x1Quadrant][(int)y / squareSize]) {
            //if (!bitmap1.at(x1Quadrant).at((int)y / squareSize)) {
                xIncrement -= 2 / squareSize;
                pacman.rotate(2);
            }
        }
        if (keyStates['d'] || keyStates[RIGHT_ARROW]) {
            x += 2;
            int x2Quadrant = (int)((x + 16.0 * cos(360 * M_PI / 180.0)) / squareSize);
            if (!this->bitmap1[x2Quadrant][(int)y / squareSize]) {
            //if (!bitmap1.at(x2Quadrant).at((int)y / squareSize)) {
                xIncrement += 2 / squareSize;
                pacman.rotate(0);
            }
        }
        if (keyStates['w'] || keyStates[UP_ARROW]) {
            y -= 2;
            int y1Quadrant = (int)((y - 16.0 * cos(360 * M_PI / 180.0)) / squareSize);
            if (!this->bitmap1[(int)x / squareSize][y1Quadrant]) {
            //if (!bitmap1.at((int)x / squareSize).at(y1Quadrant)) {
                yIncrement -= 2 / squareSize;
                pacman.rotate(3);
            }
        }
        if (keyStates['s'] || keyStates[DOWN_ARROW]) {
            y += 2;
            int y2Quadrant = (int)((y + 16.0 * cos(360 * M_PI / 180.0)) / squareSize);
            if (!this->bitmap1[(int)x / squareSize][y2Quadrant]) {
            //if (!bitmap1.at((int)x / squareSize).at(y2Quadrant)) {
                yIncrement += 2 / squareSize;
                pacman.rotate(1);
            }
        }
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

    // Method to check if the game is over
    void gameOver() {
        int pacmanX = (int)(1.5 + xIncrement);
        int pacmanY = (int)(1.5 + yIncrement);
    
        // for (const auto& monster : monsters) {
        //     int monsterX = static_cast<int>(monster.getPositionX());
        //     int monsterY = static_cast<int>(monster.getPositionY());
            
        //     if (pacmanX == monsterX && pacmanY == monsterY) {
        //         over = true;
        //         return; // If Pacman collides with any monster, game over
        //     }
        // }

        if (points == 106) {
            over = true; // If all food is eaten, game over
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
        message = "To control Pacman use WASD or Arrow Keys.";
        glRasterPos2f(50, 400);
        while (*message)
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *message++);
        message = "To start the game, press the space key twice.";
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
                this->pacman.draw(1.5 + this->xIncrement, 1.5 + this->yIncrement, this->rotation);
                for (const auto& monster : monsters) {
                    monster.draw(monster.getPositionX(), monster.getPositionY(), monster.getColorR(), monster.getColorG(), monster.getColorB());
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
std::unique_ptr<Pacman> pacmanPtr(new Pacman);
std::vector<Monster> monsters(4);
Game game(*pacmanPtr, monsters);

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
    glutInitWindowSize(750, 750);
    glutInitWindowPosition(500, 50);
    glutCreateWindow("Pacman");
    
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