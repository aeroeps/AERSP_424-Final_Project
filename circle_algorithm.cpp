#define GL_SILENCE_DEPRECATION // Used new GL library for my Mac
#include <stdlib.h>
#include <vector>
#include <deque>
//#include <windows.h>
#include <GLUT/glut.h> 
#include <iostream>
#include <string>
#define _USE_MATH_DEFINES
#include <math.h>
using namespace std;

static bool replay = false; //check if starts a new game
static bool over = true; //check for the game to be over
static float squareSize = 50.0; //size of one square on the game
static float xIncrement = 0; // x movement on pacman
static float yIncrement = 0; // y movement on pacman
static int rotation = 0; // orientation of pacman
float* monster1 = new float[3] {10.5, 8.5, 1.0}; //coordinates and direction of first monster
float* monster2 = new float[3] {13.5, 1.5, 2.0}; //coordinates and direction of second monster
float* monster3 = new float[3] {4.5, 6.5, 3.0}; //coordinates and direction of third monster
float* monster4 = new float[3] {2.5, 13.5, 4.0}; //coordinates and direction of fourth monster
static vector<int> border = { 0, 0, 15, 1, 15, 15, 14, 1, 0, 14, 15, 15, 1, 14, 0, 0 }; //coordinates of the border walls

//coordinates of the obstacles (divided into 3 for clarity)
static vector<int> obstaclesTop = { 2, 2, 3, 6, 3, 6, 4, 5, 4, 2, 5, 4, 5, 3, 6, 5, 6, 1, 9, 2, 7, 2, 8, 5, 9, 5, 10, 3, 10, 4, 11, 2, 11, 5, 12, 6, 12, 6, 13, 2 };
static vector<int> obstaclesMiddle = { 2, 9, 3, 7, 3, 7, 4, 8, 4, 9, 5, 11, 5, 6, 6, 10, 6, 10, 7, 8, 7, 8, 8, 9, 6, 7, 7, 6, 8, 6, 9, 7, 10, 6, 9, 10, 9, 10, 8, 8, 11, 9, 10, 11, 11, 8, 12, 7, 12, 7, 13, 9 };
static vector<int> obstaclesBottom = { 2, 10, 3, 13, 3, 13, 4, 12, 5, 12, 6, 13, 6, 13, 7, 11, 8, 11, 9, 13, 9, 13, 10, 12, 11, 12, 12, 13, 12, 13, 13, 10 };
static deque<float> food = { 1.5, 1.5, 1.5, 2.5, 1.5, 3.5, 1.5, 4.5, 1.5, 5.5, 1.5, 6.5, 1.5, 7.5, 1.5, 8.5, 1.5, 9.5, 1.5, 10.5, 1.5, 11.5, 1.5, 12.5, 1.5, 13.5, 2.5, 1.5, 2.5, 6.5, 2.5, 9.5, 2.5, 13.5, 3.5, 1.5, 3.5, 2.5, 3.5, 3.5, 3.5, 4.5, 3.5, 6.5, 3.5, 8.5, 3.5, 9.5, 3.5, 10.5, 3.5, 11.5, 3.5, 13.5, 4.5, 1.5, 4.5, 4.5, 4.5, 5.5, 4.5, 6.5, 4.5, 7.5, 4.5, 8.5, 4.5, 11.5, 4.5, 12.5, 4.5, 13.5, 5.5, 1.5, 5.5, 2.5, 5.5, 5.5, 5.5, 10.5, 5.5, 11.5, 5.5, 13.5, 6.5, 2.5, 6.5, 3.5, 6.5, 4.5, 6.5, 5.5, 6.5, 7.5, 6.5, 10.5, 6.5, 13.5, 7.5, 5.5, 7.5, 6.5, 7.5, 7.5, 7.5, 9.5, 7.5, 10.5, 7.5, 11.5, 7.5, 12.5, 7.5, 13.5, 8.5, 2.5, 8.5, 3.5, 8.5, 4.5, 8.5, 5.5, 8.5, 7.5, 8.5, 10.5, 8.5, 13.5, 9.5, 1.5, 9.5, 2.5, 9.5, 5.5, 9.5, 10.5, 9.5, 11.5, 9.5, 13.5, 10.5, 1.5, 10.5, 4.5, 10.5, 5.5, 10.5, 6.5, 10.5, 7.5, 10.5, 8.5, 10.5, 11.5, 10.5, 12.5, 10.5, 13.5, 11.5, 1.5, 11.5, 2.5, 11.5, 3.5, 11.5, 4.5, 11.5, 5.5, 11.5, 6.5, 11.5, 8.5, 11.5, 9.5, 11.5, 10.5, 11.5, 11.5, 11.5, 13.5, 12.5, 1.5, 12.5, 6.5, 12.5, 9.5, 12.5, 13.5, 13.5, 1.5, 13.5, 2.5, 13.5, 3.5, 13.5, 4.5, 13.5, 5.5, 13.5, 6.5, 13.5, 7.5, 13.5, 8.5, 13.5, 9.5, 13.5, 10.5, 13.5, 11.5, 13.5, 12.5, 13.5, 13.5 };
static vector<vector<bool>> bitmap; // 2d image of which squares are blocked and which are clear for pacman to move in 
bool* keyStates = new bool[256]; // record of all keys pressed 
int points = 0; // total points collected


//Method to draw the pacman character through consecutive circle algorithm
void drawPacman(float positionX, float positionY, float rotation){
	int x, y;
	glBegin(GL_LINES);
	glColor3f(1.0, 1.0, 0.0);
	for (int k = 0; k < 32; k++){
		x = (float)k / 2.0 * cos((30 + 90*rotation) * M_PI / 180.0) + (positionX*squareSize);
		y = (float)k / 2.0* sin((30 + 90 * rotation) * M_PI / 180.0) + (positionY*squareSize);
		for (int i = 30; i < 330; i++){
			glVertex2f(x, y);
			x = (float)k / 2.0 * cos((i + 90 * rotation) * M_PI / 180.0) + (positionX*squareSize);
			y = (float)k / 2.0* sin((i + 90 * rotation) * M_PI / 180.0) + (positionY*squareSize);
			glVertex2f(x, y);
		}
	}
	glEnd();
}

//Method to draw the monster character through consecutive circles algorithm
void drawMonster(float positionX, float positionY, float r, float g, float b){
	int x, y;
	glBegin(GL_LINES);
	glColor3f(r, g, b);
	//draw the head
	for (int k = 0; k < 32; k++){
		x = (float)k / 2.0 * cos(360 * M_PI / 180.0) + (positionX*squareSize);
		y = (float)k / 2.0* sin(360 * M_PI / 180.0) + (positionY*squareSize);
		for (int i = 180; i <= 360; i++){
			glVertex2f(x, y);
			x = (float)k / 2.0 * cos(i * M_PI / 180.0) + (positionX*squareSize);
			y = (float)k / 2.0* sin(i * M_PI / 180.0) + (positionY*squareSize);
			glVertex2f(x, y);
		}
	}
	glEnd();	
	//draw body
	glRectf((positionX*squareSize) - 17, positionY*squareSize, (positionX*squareSize) + 15, (positionY*squareSize) + 15);
	glBegin(GL_POINTS);
	glColor3f(0, 0.2, 0.4);
	//draw eyes and legs
	glVertex2f((positionX*squareSize) - 11, (positionY*squareSize) + 14); //legs
	glVertex2f((positionX*squareSize) - 1, (positionY*squareSize) + 14); //legs
	glVertex2f((positionX*squareSize) + 8, (positionY*squareSize) + 14); //legs
	glVertex2f((positionX*squareSize) + 4, (positionY*squareSize) - 3); //eyes
	glVertex2f((positionX*squareSize) - 7, (positionY*squareSize) - 3); //eyes 
	glEnd();
}


// Main functions that control the running of the game
int main(int argc, char** argv){
	//initialize and create the screen
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(750, 750);
	glutInitWindowPosition(500, 50);
	glutCreateWindow("PACMAN - by Patricia Terol");

	//define all the control functions
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutIdleFunc(display);
	glutKeyboardFunc(keyPressed);
	glutKeyboardUpFunc(keyUp);

	//run the game
	init();
	glutMainLoop();
	return 0;
}

