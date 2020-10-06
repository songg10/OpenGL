//Full Name: Song Tung Nguyen
//Student ID: 301354423
//SFU email: stn5@sfu.ca

#include "include/Angel.h"
#include <cstdlib>
#include <iostream>
#include <ctime>
#include <cmath>

using namespace std;

// xSize and ySize represent the window size - updated if window is reshaped to prevent stretching of the game
int xSize = 400; 
int ySize = 720;

// current tile
vec2 tile[4]; // An array of 4 2d vectors representing displacement from a 'center' piece of the tile, on the grid
vec2 tilepos; // The position of the current tile using grid coordinates ((0,0) is the bottom left corner)
vec4 tilecol[144];

// An array storing all possible orientations of all possible tiles
// The 'tile' array will always be some element [i][j] of this array (an array of vec2)
// 0 - 3	= Right "L"
// 4 - 7	= Left "L"
// 8 - 11	= Right "S" x 2
// 12 - 15	= Left "S" x 2
// 16 - 19	= "T"
// 20 - 23	= "I" x 2
vec2 AllRotations[24][4] = 
{
	{vec2(-1,-1), vec2(-1,0), vec2(0,0), vec2(1,0)},
	{vec2(1,-1), vec2(0,-1), vec2(0,0), vec2(0,1)},     
	{vec2(1,1), vec2(1,0), vec2(0,0), vec2(-1,0)},  
	{vec2(-1,1), vec2(0,1), vec2(0,0), vec2(0,-1)},

	{vec2(1,-1),vec2(1,0),vec2(0,0),vec2(-1,0)},
	{vec2(1,1),vec2(0,1),vec2(0,0),vec2(0,-1)},
	{vec2(-1,1),vec2(-1,0),vec2(0,0),vec2(1,0)},
	{vec2(-1,-1),vec2(0,-1),vec2(0,0),vec2(0,1)},

	{vec2(-1,0),vec2(0,0),vec2(0,-1),vec2(1,-1)},
	{vec2(0,-1),vec2(0,0),vec2(1,0),vec2(1,1)},
	{vec2(1,-1),vec2(0,-1),vec2(0,0),vec2(-1,0)},
	{vec2(1,1),vec2(1,0),vec2(0,0),vec2(0,-1)},

	{vec2(-1,-1),vec2(0,-1),vec2(0,0),vec2(1,0)},
	{vec2(1,-1),vec2(1,0),vec2(0,0),vec2(0,1)},
	{vec2(1,0),vec2(0,0),vec2(0,-1),vec2(-1,-1)},
	{vec2(0,1),vec2(0,0),vec2(1,0),vec2(1,-1)},

	{vec2(-1,0),vec2(0,0),vec2(1,0),vec2(0,-1)},
	{vec2(0,-1),vec2(0,0),vec2(0,1),vec2(1,0)},
	{vec2(1,0),vec2(0,0),vec2(-1,0),vec2(0,1)},
	{vec2(0,1),vec2(0,0),vec2(0,-1),vec2(-1,0)},

	{vec2(-2,0),vec2(-1,0),vec2(0,0),vec2(1,0)},
	{vec2(0,-2),vec2(0,-1),vec2(0,0),vec2(0,1)},
	{vec2(1,0),vec2(0,0),vec2(-1,0),vec2(-2,0)},
	{vec2(0,1),vec2(0,0),vec2(0,-1),vec2(0,-2)}
};

// colors
vec4 purple = 	vec4(0.5, 0.0, 1.0, 1.0);
vec4 red 	= 	vec4(1.0, 0.1, 0.1, 1.0);
vec4 yellow = 	vec4(0.8, 0.8, 0.0, 1.0);
vec4 green 	= 	vec4(0.1, 0.8, 0.1, 1.0);
vec4 orange = 	vec4(1.0, 0.5, 0.0, 1.0); 

vec4 white  = 	vec4(1.0, 1.0, 1.0, 1.0);
vec4 black  = 	vec4(0.0, 0.0, 0.0, 1.0);
vec4 grey 	= 	vec4(0.5, 0.5, 0.5, 1.0);
vec4 darkgrey = vec4(0.1, 0.1, 0.1, 1.0);

vec4 transparent = vec4(0.0, 0.0, 0.0, 0.0);
vec4 translucent = vec4(1.0, 1.0, 1.0, 0.2);

vec4 blue = vec4(0.1, 0.1, 1.0, 1.0);

// colours array
vec4 allColours[5] = {purple, red, yellow, green, orange};
 
//board[x][y] represents whether the cell (x,y) is occupied
bool board[10][20]; 

//An array containing the colour of each of the 10*20*2*3 vertices that make up the board
//Initially, all will be set to black. As tiles are placed, sets of 6 vertices (2 triangles; 1 square)
//will be set to the appropriate colour in this array before updating the corresponding VBO
vec4 boardcolours[7200];

// location of vertex attributes in the shader program
GLuint vPosition;
GLuint vColor;

// locations of uniform variables in shader program
GLuint locxSize;
GLuint locySize;

GLuint mvp;

// VAO and VBO
// 0	- grid
// 1	- current piece
// 2 	- board
// 4 	- robot arm
// 5 	- on-screen timer
GLuint vaoIDs[5]; // One VAO for each object: the grid, the board, the current piece
// 0 to 1	- grid
// 2 to 3	- current piece
// 4 to 5 	- board
// 6 to 7 	- robot arm
// 8 to 9 	- on-screen timer
GLuint vboIDs[10]; // Two Vertex Buffer Objects for each VAO (specifying vertex positions and colours, respectively)

// trigger to pause game when it ends
bool endGame;

// used to tag fruits for deletion
bool fruitTag[10][20];

float camera_angle = 0;
float camera_height = 20.0;

vec4 robot_arm[108];

float arm_theta = 30;
float arm_phi = 90;

// used to determine whether the current tile is released from the arm
bool release;

int second_timer;

// colours all faces of the block at position (x, y)
void colourBlock(int x, int y, vec4 colour) {
	for (int i = 0; i < 6; i++) {
		boardcolours[x*6 + 60*y + i] = colour;
		boardcolours[x*6 + 60*y + i + 1200] = colour;
		boardcolours[x*6 + 60*y + i + 2400] = colour;
		boardcolours[x*6 + 60*y + i + 3600] = colour;
		boardcolours[x*6 + 60*y + i + 4800] = colour;
		boardcolours[x*6 + 60*y + i + 6000] = colour;
	}
}

// When the current tile is moved or rotated (or created), update the VBO containing its vertex position data
void updatetile()
{
	// Bind the VBO containing current tile vertex positions
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[2]); 

	// For each of the 4 'cells' of the tile,
	for (int i = 0; i < 4; i++) 
	{
		// Calculate the grid coordinates of the cell
		GLfloat x = tilepos.x + tile[i].x; 
		GLfloat y = tilepos.y + tile[i].y;

		// Create the 8 corners of the cube - these vertices are using location in pixels
		// These vertices are later converted by the vertex shader
		vec4 p1 = vec4(x, y, 1.0, 1); 
		vec4 p2 = vec4(x, y + 1.0, 1.0, 1);
		vec4 p3 = vec4(x + 1.0, y, 1.0, 1);
		vec4 p4 = vec4(x + 1.0, y + 1.0, 1.0, 1);
		vec4 p5 = vec4(x, y, 0.0, 1); 
		vec4 p6 = vec4(x, y + 1.0, 0.0, 1);
		vec4 p7 = vec4(x + 1.0, y, 0.0, 1);
		vec4 p8 = vec4(x + 1.0, y + 1.0, 0.0, 1);

		// Two points are used by two triangles each
		vec4 newpoints[36] = {
			p2, p1, p3, p2, p3, p4,
			p5, p6, p7, p6, p8, p7, 
			p2, p6, p5, p2, p5, p1, 
			p4, p3, p7, p4, p7, p8,
			p6, p2, p4, p6, p4, p8,
			p1, p5, p7, p1, p7, p3}; 

		// Put new data in the VBO
		glBufferSubData(GL_ARRAY_BUFFER, i*36*sizeof(vec4), 36*sizeof(vec4), newpoints); 
	}

	glBindVertexArray(0);
}


// chooses new orientation
void chooseorientation() {
	int orientation = rand() % 24;
	for (int i = 0; i < 4 ; i++)
		tile[i] = AllRotations[orientation][i];
}

// greys all laid pieces on the board to denote game end
// used only in newtile()
void greyboard() {
	int x, y;
	for (int i = 0; i < 200; i++) {
		x = i % 10;
		y = i / 10;
		if (board[x][y]) {
			colourBlock(x, y, grey);
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[5]);
	glBufferSubData(GL_ARRAY_BUFFER, 0, 7200*sizeof(vec4), boardcolours);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);
}

// updates tilepos based on current position of robot arm
void updatetileposition() {
	if (release) return;

	// convert current tile position to a vec4 so we can use matrix multiplcation
	// place at the tip of the second arm
	vec4 tile_position = vec4(12, 0, 0, 1);

// Translate(-7.0, -9.0, 0) *
	// perform transformation similar to when displaying second robot arm
	tile_position = Translate(-3, 0, 0) * RotateZ(90-arm_theta) *
		Translate(11, 0, 0) * RotateZ(arm_phi-90) * tile_position;

	// keep the x and y components
	tilepos[0] = tile_position[0];
	tilepos[1] = tile_position[1];

	// display updated tile
	updatetile();
}

// returns true if current tile is out-of-bounds or overlapping a board block
bool collisioncheck() {
	int rounded_x = (int)round(tilepos[0]);
	int rounded_y = (int)round(tilepos[1]);
	
	for (int i = 0; i < 4; i++) {
		int x = rounded_x + (int)tile[i][0];
		int y = rounded_y + (int)tile[i][1];

		// check for out-of-bounds
		if ( x < 0 || x > 9 || y < 0 || y > 19)
			return true;
		// check for collision with existing board element
		else if ( board[x][y] )
			return true;
	}
	
	return false;
}

// Called at the start of play and every time a tile is placed
void newtile()
{
	chooseorientation();

	updatetileposition(); 

	// Update the color VBO of current tile
	vec4 blockcolour;
	vec4 greyblock[144];
	for (int i = 0; i < 144; i+=36) {
		blockcolour = allColours[rand() % 5];
		for (int j = i; j < i+36; j++) {
			tilecol[j] = blockcolour;
			greyblock[j] = grey;
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[3]); // Bind the VBO containing current tile vertex colours
	
	if ( !collisioncheck() )
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(tilecol), tilecol); // Put the colour data in the VBO
	else
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vec4)*144, greyblock);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);
}

void initGrid()
{
	// ***Generate geometry data
	vec4 gridpoints[590];
	vec4 gridcolours[590]; // One colour per vertex

	// Front Side
	// Vertical lines 
	for (int i = 0; i < 11; i++){
		gridpoints[2*i] = vec4(i, 0, 1, 1);
		gridpoints[2*i + 1] = vec4(i, 20.0, 1, 1);
	}
	// Horizontal lines
	for (int i = 0; i < 21; i++){
		gridpoints[22 + 2*i] = vec4(0, i, 1, 1);
		gridpoints[22 + 2*i + 1] = vec4(10.0, i, 1, 1);
	}

	// Back Side
	// Vertical lines 
	for (int i = 0; i < 11; i++){
		gridpoints[64 + 2*i] = vec4(i, 0, 0, 1);
		gridpoints[64 + 2*i + 1] = vec4(i, 20.0, 0, 1);
	}
	// Horizontal lines
	for (int i = 0; i < 21; i++){
		gridpoints[86 + 2*i] = vec4(0, i, 0, 1);
		gridpoints[86 + 2*i + 1] = vec4(10.0, i, 0, 1);
	}

	// Z-lines
	for (int i = 0; i < 11; i++) {
		for (int j = 0; j < 21; j++) {
			gridpoints[128 + 42*i + 2*j] = vec4(i, j, 1, 1);
			gridpoints[128 + 42*i + 2*j + 1] = vec4(i, j, 0, 1);
		}
	}

	// Make all grid lines yellow
	for (int i = 0; i < 590; i++)
		gridcolours[i] = yellow;


	// *** set up buffer objects
	// Set up first VAO (representing grid lines)
	glBindVertexArray(vaoIDs[0]); // Bind the first VAO
	glGenBuffers(2, vboIDs); // Create two Vertex Buffer Objects for this VAO (positions, colours)

	// Grid vertex positions
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[0]); // Bind the first grid VBO (vertex positions)
	glBufferData(GL_ARRAY_BUFFER, 590*sizeof(vec4), gridpoints, GL_STATIC_DRAW); // Put the grid points in the VBO
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, 0); 
	glEnableVertexAttribArray(vPosition); // Enable the attribute
	
	// Grid vertex colours
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[1]); // Bind the second grid VBO (vertex colours)
	glBufferData(GL_ARRAY_BUFFER, 590*sizeof(vec4), gridcolours, GL_STATIC_DRAW); // Put the grid colours in the VBO
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vColor); // Enable the attribute
}


void initBoard()
{
	// *** Generate the geometric data
	vec4 boardpoints[7200];

	for (int i = 0; i < 7200; i++) {
		// Let the empty cells on the board be transparent
		boardcolours[i] = transparent;
	}
	// Each cell is a square (2 triangles with 6 vertices)
	for (int i = 0; i < 20; i++){
		for (int j = 0; j < 10; j++)
		{		
			vec4 p1 = vec4(j, i, 1.0, 1);
			vec4 p2 = vec4(j, i + 1.0, 1.0, 1);
			vec4 p3 = vec4(j + 1.0, i, 1.0, 1);
			vec4 p4 = vec4(j + 1.0, i + 1.0, 1.0, 1);

			vec4 p5 = vec4(j, i, 0.0, 1);
			vec4 p6 = vec4(j, i + 1.0, 0.0, 1);
			vec4 p7 = vec4(j + 1.0, i, 0.0, 1);
			vec4 p8 = vec4(j + 1.0, i + 1.0, 0.0, 1);
			
			// front side points
			boardpoints[6*(10*i + j)    ] = p2;
			boardpoints[6*(10*i + j) + 1] = p1;
			boardpoints[6*(10*i + j) + 2] = p3;
			boardpoints[6*(10*i + j) + 3] = p2;
			boardpoints[6*(10*i + j) + 4] = p3;
			boardpoints[6*(10*i + j) + 5] = p4;
			// back side points
			boardpoints[6*(10*i + j) + 1200] = p5;
			boardpoints[6*(10*i + j) + 1201] = p6;
			boardpoints[6*(10*i + j) + 1202] = p7;
			boardpoints[6*(10*i + j) + 1203] = p8;
			boardpoints[6*(10*i + j) + 1204] = p7;
			boardpoints[6*(10*i + j) + 1205] = p6;
			// left side points
			boardpoints[6*(10*i + j) + 2400] = p6;
			boardpoints[6*(10*i + j) + 2401] = p5;
			boardpoints[6*(10*i + j) + 2402] = p1;
			boardpoints[6*(10*i + j) + 2403] = p6;
			boardpoints[6*(10*i + j) + 2404] = p1;
			boardpoints[6*(10*i + j) + 2405] = p2;
			// right side points
			boardpoints[6*(10*i + j) + 3600] = p4;
			boardpoints[6*(10*i + j) + 3601] = p3;
			boardpoints[6*(10*i + j) + 3602] = p7;
			boardpoints[6*(10*i + j) + 3603] = p8;
			boardpoints[6*(10*i + j) + 3604] = p4;
			boardpoints[6*(10*i + j) + 3605] = p7;
			// top side points
			boardpoints[6*(10*i + j) + 4800] = p6;
			boardpoints[6*(10*i + j) + 4801] = p2;
			boardpoints[6*(10*i + j) + 4802] = p4;
			boardpoints[6*(10*i + j) + 4803] = p6;
			boardpoints[6*(10*i + j) + 4804] = p4;
			boardpoints[6*(10*i + j) + 4805] = p8;
			// bottom side points
			boardpoints[6*(10*i + j) + 6000] = p5;
			boardpoints[6*(10*i + j) + 6001] = p7;
			boardpoints[6*(10*i + j) + 6002] = p1;
			boardpoints[6*(10*i + j) + 6003] = p1;
			boardpoints[6*(10*i + j) + 6004] = p7;
			boardpoints[6*(10*i + j) + 6005] = p3;
		}
	}

	// Initially no cell is occupied
	for (int i = 0; i < 10; i++) {
		for (int j = 0; j < 20; j++) {
			board[i][j] = false; 
			fruitTag[i][j] = false;
		}
	}

	// *** set up buffer objects
	glBindVertexArray(vaoIDs[2]);
	glGenBuffers(2, &vboIDs[4]);

	// Grid cell vertex positions
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[4]);
	glBufferData(GL_ARRAY_BUFFER, 7200*sizeof(vec4), boardpoints, GL_STATIC_DRAW);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vPosition);

	// Grid cell vertex colours
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[5]);
	glBufferData(GL_ARRAY_BUFFER, 7200*sizeof(vec4), boardcolours, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vColor);
}

void initCurrentTile()
{
	glBindVertexArray(vaoIDs[1]);
	glGenBuffers(2, &vboIDs[2]);

	// Current tile vertex positions
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[2]);
	glBufferData(GL_ARRAY_BUFFER, 144*sizeof(vec4), NULL, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vPosition);

	// Current tile vertex colours
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[3]);
	glBufferData(GL_ARRAY_BUFFER, 144*sizeof(vec4), NULL, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vColor);
}

void robotBuilder(int segment, 	vec4 p1, vec4 p2, vec4 p3, vec4 p4,
									vec4 p5, vec4 p6, vec4 p7, vec4 p8) {
	if (segment < 0 || segment > 2) return;
	// front side
	robot_arm[segment*36 + 0] = p1; robot_arm[segment*36 + 1] = p3;
	robot_arm[segment*36 + 2] = p4;robot_arm[segment*36 + 3] = p1;
	robot_arm[segment*36 + 4] = p4; robot_arm[segment*36 + 5] = p2;
	// back side
	robot_arm[segment*36 + 6] = p8; robot_arm[segment*36 + 7] = p7;
	robot_arm[segment*36 + 8] = p6; robot_arm[segment*36 + 9] = p7;
	robot_arm[segment*36 + 10] = p5; robot_arm[segment*36 + 11] = p6;
	// left side
	robot_arm[segment*36 + 12] = p6; robot_arm[segment*36 + 13] = p1;
	robot_arm[segment*36 + 14] = p2; robot_arm[segment*36 + 15] = p6;
	robot_arm[segment*36 + 16] = p5; robot_arm[segment*36 + 17] = p1;
	// right side
	robot_arm[segment*36 + 18] = p4; robot_arm[segment*36 + 19] = p7;
	robot_arm[segment*36 + 20] = p8; robot_arm[segment*36 + 21] = p4;
	robot_arm[segment*36 + 22] = p3; robot_arm[segment*36 + 23] = p7;
	// top side
	robot_arm[segment*36 + 24] = p6; robot_arm[segment*36 + 25] = p2;
	robot_arm[segment*36 + 26] = p8; robot_arm[segment*36 + 27] = p2;
	robot_arm[segment*36 + 28] = p4; robot_arm[segment*36 + 29] = p8;
	// bottom side
	robot_arm[segment*36 + 30] = p1; robot_arm[segment*36 + 31] = p7;
	robot_arm[segment*36 + 32] = p3; robot_arm[segment*36 + 33] = p1;
	robot_arm[segment*36 + 34] = p5; robot_arm[segment*36 + 35] = p7;
}

void initRobotArm() {
	glBindVertexArray(vaoIDs[3]);
	glGenBuffers(2, &vboIDs[6]);

	// Define points for robot base
	vec4 p1 = vec4(0, 0, 2, 1);
	vec4 p2 = vec4(0, 1, 2, 1);
	vec4 p3 = vec4(2, 0, 2, 1);
	vec4 p4 = vec4(2, 1, 2, 1);

	vec4 p5 = vec4(0, 0, 0, 1);
	vec4 p6 = vec4(0, 1, 0, 1);
	vec4 p7 = vec4(2, 0, 0, 1);
	vec4 p8 = vec4(2, 1, 0, 1);

	// Build arm base
	robotBuilder(0, 	p1, p2, p3, p4,
						p5, p6, p7, p8);

	// Define points for robot arm 1
	p1 = vec4(0, 0, 1, 1);
	p2 = vec4(0, 1, 1, 1);
	p3 = vec4(11, 0, 1, 1);
	p4 = vec4(11, 1, 1, 1);

	p5 = vec4(0, 0, 0, 1);
	p6 = vec4(0, 1, 0, 1);
	p7 = vec4(11, 0, 0, 1);
	p8 = vec4(11, 1, 0, 1);

	// Build arm 1
	robotBuilder(1, 	p1, p2, p3, p4,
						p5, p6, p7, p8);

	// Points for robot arm 2
	p1 = vec4(0, 0, 0.5, 1);
	p2 = vec4(0, 0.5, 0.5, 1);
	p3 = vec4(12, 0, 0.5, 1);
	p4 = vec4(12, 0.5, 0.5, 1);

	p5 = vec4(0, 0, 0, 1);
	p6 = vec4(0, 0.5, 0, 1);
	p7 = vec4(12, 0, 0, 1);
	p8 = vec4(12, 0.5, 0, 1);

	// Build arm 2
	robotBuilder(2, 	p1, p2, p3, p4,
						p5, p6, p7, p8);

	// Robot arm colour
	vec4 arm_colour[108];
	for (int i = 0; i < 36; i++) {
		arm_colour[i     ] = blue;
		arm_colour[i + 36] = red;
		arm_colour[i + 72] = green;
	}

	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[6]);
	glBufferData(GL_ARRAY_BUFFER, 108*sizeof(vec4), robot_arm, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vPosition);

	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[7]);
	glBufferData(GL_ARRAY_BUFFER, 108*sizeof(vec4), arm_colour, GL_STATIC_DRAW);
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vColor);
}

void initTimer() {
	glBindVertexArray(vaoIDs[4]);
	glGenBuffers(2, &vboIDs[8]);

	// background = 6 points
	// three digits, one decimal = 132 points (42 + 42 + 42 + 6)
	// each number is 24 by 48
	// each line is 4 pixels wide
	// gap between numbers is 5
	// borders extend by 5 each side

	vec4 points[138];
	vec4 colours[138];
	vec4 temp[42];

	// top line
	temp[0] = vec4(4, 48, 0, 1); temp[1] = vec4(20, 44, 0, 1); temp[2] = vec4(20, 48, 0, 1);
	temp[3] = vec4(4, 44, 0, 1); temp[4] = vec4(20, 44, 0, 1); temp[5] = vec4(4, 48, 0, 1);
	// middle line
	temp[6] = vec4(4, 26, 0, 1); temp[7] = vec4(20, 22, 0, 1); temp[8] = vec4(20, 26, 0, 1);
	temp[9] = vec4(4, 22, 0, 1); temp[10] = vec4(20, 22, 0, 1); temp[11] = vec4(4, 26, 0, 1);
	// bottom line
	temp[12] = vec4(4, 4, 0, 1); temp[13] = vec4(20, 0, 0, 1); temp[14] = vec4(20, 4, 0, 1);
	temp[15] = vec4(4, 0, 0, 1); temp[16] = vec4(20, 0, 0, 1); temp[17] = vec4(4, 4, 0, 1);

	// top left line
	temp[18] = vec4(0, 44, 0, 1); temp[19] = vec4(4, 26, 0, 1); temp[20] = vec4(4, 44, 0, 1);
	temp[21] = vec4(0, 44, 0, 1); temp[22] = vec4(0, 26, 0, 1); temp[23] = vec4(4, 26, 0, 1);
	// top right line
	temp[24] = vec4(20, 44, 0, 1); temp[25] = vec4(24, 26, 0, 1); temp[26] = vec4(24, 44, 0, 1);
	temp[27] = vec4(20, 44, 0, 1); temp[28] = vec4(20, 26, 0, 1); temp[29] = vec4(24, 26, 0, 1);
	// bottom left line
	temp[30] = vec4(0, 22, 0, 1); temp[31] = vec4(4, 4, 0, 1); temp[32] = vec4(4, 22, 0, 1);
	temp[33] = vec4(0, 22, 0, 1); temp[34] = vec4(0, 4, 0, 1); temp[35] = vec4(4, 4, 0, 1);
	// bottom right line
	temp[36] = vec4(20, 22, 0, 1); temp[37] = vec4(24, 4, 0, 1); temp[38] = vec4(24, 22, 0, 1);
	temp[39] = vec4(20, 22, 0, 1); temp[40] = vec4(20, 4, 0, 1); temp[41] = vec4(24, 4, 0, 1);

	// background points
	points[0] = vec4(0, 0, 0, 1);
	points[1] = vec4(0, -58, 0, 1);
	points[2] = vec4(101, 0, 0, 1);
	points[3] = vec4(0, -58, 0, 1);
	points[4] = vec4(101, -58, 0, 1);
	points[5] = vec4(101, 0, 0, 1);

	// decimal points
	points[132] = vec4(63, -53, -0.1, 1);
	points[133] = vec4(67, -53, -0.1, 1);
	points[134] = vec4(63, -49, -0.1, 1);
	points[135] = vec4(63, -49, -0.1, 1);
	points[136] = vec4(67, -53, -0.1, 1);
	points[137] = vec4(67, -49, -0.1, 1);

	// number points
	for (int i = 0; i < 42; i++) {
		points[6 + i] = Translate(5, -53, -0.1) * temp[i];
		points[48 + i] = Translate(34, -53, -0.1) * temp[i];
		points[90 + i] = Translate(72, -53, -0.1) * temp[i];
	}

	// start off with dark grey
	for (int i = 0; i < 132; i++)
		colours[i] = darkgrey;

	// decimal is always red
	for (int i = 132; i < 138; i++)
		colours[i] = red;

	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[8]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec4)*138, points, GL_STATIC_DRAW);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vPosition);

	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[9]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec4)*138, colours, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vColor);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void init()
{
	srand(time(NULL));		// seed the rand() function

	// Load shaders and use the shader program
	GLuint program = InitShader("vshader.glsl", "fshader.glsl");
	glUseProgram(program);

	// Get the location of the attributes (for glVertexAttribPointer() calls)
	vPosition = glGetAttribLocation(program, "vPosition");
	vColor = glGetAttribLocation(program, "vColor");

	// Create 4 Vertex Array Objects, each representing one 'object'. Store the names in array vaoIDs
	glGenVertexArrays(4, &vaoIDs[0]);

	// Initialize the grid, board, and current tile
	initGrid();
	initBoard();
	initCurrentTile();
	initRobotArm();
	initTimer();

	// The location of the uniform variables in the shader program
	locxSize = glGetUniformLocation(program, "xSize"); 
	locySize = glGetUniformLocation(program, "ySize");
	mvp = glGetUniformLocation(program, "mvp");

	// Game initialization
	endGame = false;
	release = false;
	second_timer = 100;
	newtile(); // create new next tile

	// set to default
	glBindVertexArray(0);
	glClearColor(0, 0, 0, 0);

	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	glCullFace(GL_BACK);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable (GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GEQUAL, 0.2);
}

// Checks if the specified row (0 is the bottom 19 the top) is full
// If every cell in the row is occupied, it will clear that cell and everything above it will shift down one row
void checkfullrow(int row)
{
	int i;

	for (i = 0; i < 10; i++) {
		if ( !board[i][row] )
			break;
	}
	// if the row is not full, return
	if (i != 10)
		return;

	// shift down all rows above the specified row
	for (i = row; i < 20; i++) {
		// for each block in the row
		for (int j = 0; j < 10; j++) {
			// for every row except for the top
			if (i != 19) {
				board[j][i] = board[j][i+1];
				// replace tagged fruits with tags from above
				fruitTag[j][i] = fruitTag[j][i+1];

				// replace row with colours of row above
				colourBlock(j, i, boardcolours[j*6 + (i+1)*60]);
			}
			// for top row
			else {
				board[j][i] = false;
				fruitTag[j][i] = false;

				// replace top row with transparent
				colourBlock(j, i, transparent);
			}
		}
	}

	// refresh the colours in the buffer
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[5]);
	glBufferSubData(GL_ARRAY_BUFFER, 0, 7200*sizeof(vec4), boardcolours);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);
}

// Places the current tile - update the board vertex colour VBO and the array maintaining occupied cells
void settile()
{
	int x, y;

	// retrieve colours from current piece VBO
	vec4 colours[144] = {0};
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[3]);
	glGetBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(colours), colours);
	
	for (int i = 0; i < 4; i++) {
		x = tilepos[0] + tile[i][0];
		y = tilepos[1] + tile[i][1];

		// make sure the current block does not overflow off the top
		// if game has ended, allow overflow to prevent infinite loop
		if (y < 20) {
			board[x][y] = true;

			if (!endGame) {
				colourBlock(x, y, tilecol[i*36]);
			}
		}
		// if it does overflow, end the game
		else {
			endGame = true;
			break;
		}
	}

	// set current tile to grey if endGame was triggered (show losing block)
	if (endGame) {
		vec4 greycolour[144];
		for (int i = 0; i < 144; i++)
			greycolour[i] = grey;
		glBindBuffer(GL_ARRAY_BUFFER, vboIDs[3]);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(greycolour), greycolour);
	}

	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[5]);
	glBufferSubData(GL_ARRAY_BUFFER, 0, 7200*sizeof(vec4), boardcolours);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);
}

// Given (x,y), tries to move the tile x squares to the right and y squares down
// Returns true if the tile was successfully moved, or false if there was some issue
bool movetile(vec2 direction)
{
	vec2 newblockpos;
	for (int i = 0; i < 4; i++) {
		newblockpos = tilepos + tile[i] + direction;

		// check if it has hit a block
		if (board[(int)(newblockpos[0])][(int)(newblockpos[1])])
			return false;

		// check if it has hit the side of the board
		else if (newblockpos[0] < 0 || newblockpos[0] > 9)
			return false;

		// check if it has hit the bottom of the board
		else if (newblockpos[1] < 0)
			return false;
	}
	tilepos += direction;
	updatetile();
	return true;
}

// Compares input colour with board colour at coordinates (x,y)
bool colourcheck(vec4 colour, int x, int y) {
	return 	colour[0] == boardcolours[x*6 + y*60][0] &&
			colour[1] == boardcolours[x*6 + y*60][1] &&
			colour[2] == boardcolours[x*6 + y*60][2] &&
			colour[3] == boardcolours[x*6 + y*60][3];

}

// Tag consecutive fruits for deletion after checking rows
void tagfruits() {
	vec4 currentcol;
	int x, y;
	bool left, right, up, down;

	for (int i = 0; i < 200; i++) {
		x = i % 10;
		y = i / 10;

		// checks if the current block is on the board
		if ( !board[i%10][i/10] )
			continue;
		
		currentcol = boardcolours[x*6 + y*60];

		// check if near edges
		left = (x == 0);
		right = (x == 9);
		up = (y == 19);
		down = (y == 0);


		// check for threes where the current tile is the centre
		// corner to corner checks
		if ( !(up || down || left || right) ) {
			// NW to SE
			if (colourcheck(currentcol, x-1, y+1) &&
				colourcheck(currentcol, x+1, y-1)) {
				fruitTag[x][y] = true;
				fruitTag[x-1][y+1] = true;
				fruitTag[x+1][y-1] = true;
			}
			// SW to NE
			if (colourcheck(currentcol, x-1, y-1) &&
				colourcheck(currentcol, x+1, y+1)) {
				fruitTag[x][y] = true;
				fruitTag[x-1][y-1] = true;
				fruitTag[x+1][y+1] = true;
			}
		}
		// W to E check
		if (	!(left || right)  &&
				colourcheck(currentcol, x-1, y) &&
				colourcheck(currentcol, x+1, y)) {
			fruitTag[x][y] = true;
			fruitTag[x-1][y] = true;
			fruitTag[x+1][y] = true;
		}
		// N to S check
		if (	!(up || down)  &&
				colourcheck(currentcol, x, y-1) &&
				colourcheck(currentcol, x, y+1)) {
			fruitTag[x][y] = true;
			fruitTag[x][y-1] = true;
			fruitTag[x][y+1] = true;
		}

		// check for 3+ where the current tile is the end point
		// checking northbound
		if (y < 18 &&
			colourcheck(currentcol, x, y+1) &&
			colourcheck(currentcol, x, y+2)) {
			fruitTag[x][y] = true;
			fruitTag[x][y+1] = true;
			fruitTag[x][y+2] = true;

			// if 3+
			for (int k = y+3; k < 20; k++) {
				if (colourcheck(currentcol, x, k))
					fruitTag[x][k] = true;
				else
					break;
			}
		}
		// checking northeastbound
		if (y < 18 &&
			x < 8 &&
			colourcheck(currentcol, x+1, y+1) &&
			colourcheck(currentcol, x+2, y+2)) {
			fruitTag[x][y] = true;
			fruitTag[x+1][y+1] = true;
			fruitTag[x+2][y+2] = true;

			// if 3+
			int k = y+3;
			for (int j = x+3; j < 10; j++) {
				if (k < 20) {
					if (colourcheck(currentcol, j, k))
						fruitTag[j][k] = true;
					else
						break;
					k++;
				}
				else
					break;
			}
		}
		// checking eastbound
		if (x < 8 &&
			colourcheck(currentcol, x+1, y) &&
			colourcheck(currentcol, x+2, y)) {
			fruitTag[x][y] = true;
			fruitTag[x+1][y] = true;
			fruitTag[x+2][y] = true;

			// if 3+
			for (int j = x+3; j < 10; j++) {
				if (colourcheck(currentcol, j, y))
					fruitTag[j][y] = true;
				else
					break;
			}
		}
		// checking southeastbound
		if (y > 1 &&
			x < 8 &&
			colourcheck(currentcol, x+1, y-1) &&
			colourcheck(currentcol, x+2, y-2)) {
			fruitTag[x][y] = true;
			fruitTag[x+1][y-1] = true;
			fruitTag[x+2][y-2] = true;

			// if 3+
			int k = y-3;
			for (int j = x+3; j < 10; j++) {
				if (k >= 0) {
					if (colourcheck(currentcol, j, k))
						fruitTag[j][k] = true;
					else
						break;
					k--;
				}
				else
					break;
			}
		}
		// checking southbound
		if (y > 1 &&
			colourcheck(currentcol, x, y-1) &&
			colourcheck(currentcol, x, y-2)) {
			fruitTag[x][y] = true;
			fruitTag[x][y-1] = true;
			fruitTag[x][y-2] = true;

			// if 3+
			for (int k = y-3; k >= 0; k--) {
				if (colourcheck(currentcol, x, k))
					fruitTag[x][k] = true;
				else
					break;
			}
		}
		// checking southwestbound
		if (y > 1 &&
			x > 1 &&
			colourcheck(currentcol, x-1, y-1) &&
			colourcheck(currentcol, x-2, y-2)) {
			fruitTag[x][y] = true;
			fruitTag[x-1][y-1] = true;
			fruitTag[x-2][y-2] = true;

			// if 3+
			int k = y-3;
			for (int j = x-3; j >= 0; j--) {
				if (k >= 0) {
					if (colourcheck(currentcol, j, k))
						fruitTag[j][k] = true;
					else
						break;
					k--;
				}
				else
					break;
			}
		}
		// checking westbound
		if (x > 1 &&
			colourcheck(currentcol, x-1, y) &&
			colourcheck(currentcol, x-2, y)) {
			fruitTag[x][y] = true;
			fruitTag[x-1][y] = true;
			fruitTag[x-2][y] = true;

			// if 3+
			for (int j = x-3; j >= 0; j--) {
				if (colourcheck(currentcol, j, y))
					fruitTag[j][y] = true;
				else
					break;
			}
		}
		// checking northwestbound
		if (y < 18 &&
			x > 1 &&
			colourcheck(currentcol, x-1, y+1) &&
			colourcheck(currentcol, x-2, y+2)) {
			fruitTag[x][y] = true;
			fruitTag[x-1][y+1] = true;
			fruitTag[x-2][y+2] = true;

			// if 3+
			int k = y+3;
			for (int j = x-3; j >= 0; j--) {
				if (k < 20) {
					if (colourcheck(currentcol, j, k))
						fruitTag[j][k] = true;
					else
						break;
					k++;
				}
				else
					break;
			}
		}
	}
}

// Deletes the specified block and moves everything above it down
void deleteblock(int x, int y) {
	// shift down all blocks above the specified block
	for (int i = y; i < 20; i++) {
		// for every row except the top
		if (i != 19) {
			board[x][i] = board[x][i+1];
			// probably redundant, since deletetags() is called from top to bottom
			fruitTag[x][i] = fruitTag[x][i+1];

			// replace block with colours of block above
			colourBlock(x, i, boardcolours[x*6 + (i+1)*60]);
		}
		// for top row
		else {
			board[x][i] = false;
			fruitTag[x][i] = false;

			// replace top block with black
			colourBlock(x, i, transparent);
		}
	}

	// refresh the colours in the buffer
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[5]);
	glBufferSubData(GL_ARRAY_BUFFER, 0, 7200*sizeof(vec4), boardcolours);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);
}

// Remove the tagged fruits
void deletetags() {
	bool doublecheck = false;

	for (int x = 9; x >= 0; x--) {
		for (int y = 19; y >= 0; y--) {
			if (fruitTag[x][y]) {
				deleteblock(x,y);
				doublecheck = true;
			}
		}
	}

	// checks whether the moved blocks now create more consecutive fruits
	if (doublecheck) {
		tagfruits();
		deletetags();
	}
}

// Rotates the current tile, if there is room
void rotate()
{      
	int i, j;
	// check to see what block and orientation it currently is
	for (i = 0; i < 24; i++) {
		for (j = 0; j < 4; j++) {
			if (AllRotations[i][j][0] != tile[j][0]) break;
			else if (AllRotations[i][j][1] != tile[j][1]) break;
		}
		if (j == 4) break;
	}
	if (i == 24) return;

	// find the next orientation
	i++;
	if (i % 4 == 0)
		i -= 4;

	// copy arrays
	for (int j = 0; j < 4; j++)
		tile[j] = AllRotations[i][j];

	updatetile();
}

// changes the colour of the current tile depending on if there is a collision
void changetilecolour(bool collision) {
	vec4 colour[144];
	if (collision) {
		for (int i = 0; i < 144; i++)
			colour[i] = grey;
	}
	else {
		for (int i = 0; i < 144; i++)
			colour[i] = tilecol[i];
	}

	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[3]);
	glBufferSubData(GL_ARRAY_BUFFER, 0, 144*sizeof(vec4), colour);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

}

// checks if current tile is in a valid position, then releases it
void releasetile() {
	if ( collisioncheck() ) return;
	tilepos[0] = round(tilepos[0]);
	tilepos[1] = round(tilepos[1]);
	updatetile();

	release = true;
}

void refreshTimer() {
	vec4 colours[126];
	int digits[3];

	digits[0] = second_timer / 100;
	digits[1] = (second_timer / 10) % 10;
	digits[2] = (second_timer % 100) % 10;

	// set default colour
	for (int i = 0; i < 126; i++)
		colours[i] = darkgrey;

	// create numbers
	for (int i = 0; i < 3; i++) {
		// top line
		if (digits[i] != 1 && digits[i] != 4) {
			for (int j = 0; j < 6; j++)
				colours[42*i + j] = red;
		}

		// middle line
		if (digits[i] != 0 && digits[i] != 1 && digits[i] != 7) {
			for (int j = 0; j < 6; j++)
				colours[42*i + j + 6] = red;
		}

		// bottom line
		if (digits[i] != 1 && digits[i] != 4 && digits[i] != 7) {
			for (int j = 0; j < 6; j++)
				colours[42*i + j + 12] = red;
		}

		// top left line
		if (digits[i] != 1 && digits[i] != 2 && digits[i] != 3 && digits[i] != 7) {
			for (int j = 0; j < 6; j++)
				colours[42*i + j+ 18] = red;
		}

		// top right line
		if (digits[i] != 5 && digits[i] != 6) {
			for (int j = 0; j < 6; j++)
				colours[42*i + j + 24] = red;
		}

		// bottom left line
		if (digits[i] == 0 || digits[i] == 2 || digits[i] == 6 || digits[i] == 8) {
			for (int j = 0; j < 6; j++)
				colours[42*i + j + 30] = red;
		}

		// bottom right line
		if (digits[i] != 2) {
			for (int j = 0; j < 6; j++)
				colours[42*i + j + 36] = red;
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[9]);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(vec4)*6, sizeof(vec4)*126, colours);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void timer(int value) {
	refreshTimer();

	// if time has run out, release the tile if possible, otherwise end the game
	if ( second_timer <= 0 ) {
		if ( !collisioncheck() )
			releasetile();
		else {
			endGame = true;
			greyboard();
		}
	}
	
	// if the block has been released and 7 milliseconds have elapsed
	if ( release ) {
		second_timer = 100;

		// if no top out, continue the game
		if ( !endGame && !movetile(vec2(0,-1)) ) {
			settile();

			// if no partial lock out, check for completed rows, then create a new tile
			if (!endGame) {	
				// tag fruits for deletion after checkfullrow()
				tagfruits();

				// record which rows to check
				bool checkrow[4] = {false, false, false, false};
				for (int i = 0; i < 4; i++) {
					if (tile[i][1] == 1)
						checkrow[0] = true;
					else if (tile[i][1] == 0)
						checkrow[1] = true;
					else if (tile[i][1] == -1)
						checkrow[2] = true;
					else
						checkrow[3] = true;
				}
				// use checkfullrow(), starting with the topmost block filled
				for (int i = 0; i < 4; i++) {
					if (checkrow[i])
						checkfullrow(tilepos[1] + 1 - i);
				}
				// tag fruits again after blocks were moved by checkfullrow()
				tagfruits();
				deletetags();

				release = false;

				newtile();
			}
		}
	}

	if (!release && !endGame)
		second_timer--;

	glutPostRedisplay();
	glutTimerFunc(100, timer, 0);
}

// Starts the game over - empties the board, creates new tiles, resets line counters
void restart()
{
	initBoard();

	endGame = false;
	release = false;
	second_timer = 100;

	arm_theta = 30;
	arm_phi = 90;

	newtile();
}

// Draws the game
void display()
{

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUniform1i(locxSize, xSize); // x and y sizes are passed to the shader program to maintain shape of the vertices on screen
	glUniform1i(locySize, ySize);

	// projection matrix
	mat4 projection = Perspective(45.0, (float)xSize/(float)ySize, 0.5, 50.0);

	// camera/view matrix
	vec4 eye = vec4(30.0*sin(camera_angle), camera_height, 30.0*cos(camera_angle), 1.0);
	vec4 at = vec4(0.0, 0.0, 0.0, 1.0);
	vec4 up = vec4(-30.0*sin(camera_angle), camera_height, -30.0*cos(camera_angle), 0.0);
	mat4 view = LookAt(eye, at, up);

	// model matrix for current block, grid, and board blocks
	mat4 model = Translate(-5.0, -10.0, 0.0);

	// combine matrices
	mat4 mvp_mat = projection * view * model;
	glUniformMatrix4fv(mvp, 1, GL_TRUE, mvp_mat);

	glBindVertexArray(vaoIDs[1]); // Bind the VAO representing the current tile (to be drawn on top of the board)
	glDrawArrays(GL_TRIANGLES, 0, 144); // Draw the current tile (8 triangles)

	glBindVertexArray(vaoIDs[0]); // Bind the VAO representing the grid lines (to be drawn on top of everything else)
	glDrawArrays(GL_LINES, 0, 590); // Draw the grid lines (21+11 = 32 lines)

	// Bind the VAO representing the grid cells (to be drawn first)
	glBindVertexArray(vaoIDs[2]);
	glDrawArrays(GL_TRIANGLES, 0, 7200);


	// robot arm
	glBindVertexArray(vaoIDs[3]);

	// robot arm base
	mat4 base_model = Translate(-8.0, -10.0, -0.5);
	mvp_mat = projection * view * base_model;
	glUniformMatrix4fv(mvp, 1, GL_TRUE, mvp_mat);
	glDrawArrays(GL_TRIANGLES, 0, 36);

	// robot arm 1
	mat4 arm_model = Translate(-7.0, -9.0, 0.5) * RotateZ(90-arm_theta) * Translate(0.0, -0.5, -0.5);
	mvp_mat = projection * view * arm_model;
	glUniformMatrix4fv(mvp, 1, GL_TRUE, mvp_mat);
	glDrawArrays(GL_TRIANGLES, 36, 36);

	// robot arm 2
	arm_model = Translate(10.5, 0.5, 0.5) * RotateZ(arm_phi-90) * Translate(0.0, -0.25, -0.25);
	mvp_mat *= arm_model;
	glUniformMatrix4fv(mvp, 1, GL_TRUE, mvp_mat);
	glDrawArrays(GL_TRIANGLES, 72, 36);

	// release timer
	glBindVertexArray(vaoIDs[4]);
	mvp_mat = Translate(-0.95, 0.95, 0) * Scale(2.0/xSize, 2.0/ySize, 1);
	glUniformMatrix4fv(mvp, 1, GL_TRUE, mvp_mat);
	glDrawArrays(GL_TRIANGLES, 0, 138);

	glutSwapBuffers();
}

// Reshape callback will simply change xSize and ySize variables, which are passed to the vertex shader
// to keep the game the same from stretching if the window is stretched
void reshape(GLsizei w, GLsizei h)
{
	xSize = w;
	ySize = h;
	glViewport(0, 0, w, h);
}

// Handle arrow key keypresses
void special(int key, int x, int y)
{
	bool ctrl = glutGetModifiers() == GLUT_ACTIVE_CTRL;

	// only record these keys if game has not ended
	if (!endGame) {
		switch(key) {
			case GLUT_KEY_UP:
				if (ctrl) {
					camera_height++;
					if (camera_height > 20) camera_height = 20;
				}
				rotate();
				changetilecolour(collisioncheck());
				break;

			case GLUT_KEY_DOWN:
				if (ctrl) {
					camera_height--;
					if (camera_height < 1) camera_height = 1;
				}
				else if (release)
					movetile(vec2(0,-1));
				break;

			case GLUT_KEY_LEFT:
				if (ctrl) {
					camera_angle -= 0.1;
					if (camera_angle < 0) camera_angle += 2*M_PI;
				}
				/*else if (release){
					movetile(vec2(-1, 0));
					updatetile();
				}*/
				break;

			case GLUT_KEY_RIGHT:
				if (ctrl) {
					camera_angle += 0.1;
					if (camera_angle > 2*M_PI) camera_angle -= 2*M_PI;
				}
				/*else if (release){
					movetile(vec2(1,0));
					updatetile();
				}*/
				break;
		}
	}
	glutPostRedisplay();
}

// Handles standard keypresses
void keyboard(unsigned char key, int x, int y)
{
	switch(key) 
	{
		case 033: // Both escape key and 'q' cause the game to exit
		    exit(EXIT_SUCCESS);
		    break;
		case 'q':
			exit (EXIT_SUCCESS);
			break;
		case 'r': // 'r' key restarts the game
			restart();
			break;
		case 'a':
			if (endGame) break;
			arm_theta -= 3;
			if (arm_theta < -90) arm_theta = -90;
			if (!release) {
				updatetileposition();
				changetilecolour(collisioncheck());
			}
			break;
		case 'd':
			if (endGame) break;
			arm_theta += 3;
			if (arm_theta > 90) arm_theta = 90;
			if (!release) {
				updatetileposition();
				changetilecolour(collisioncheck());
			}
			break;
		case 'w':
			if (endGame) break;
			arm_phi += 3;
			if (arm_phi > 180) arm_phi = 180;
			if (!release) {
				updatetileposition();
				changetilecolour(collisioncheck());
			}
			break;
		case 's':
			if (endGame) break;
			arm_phi -= 3;
			if (arm_phi < -80) arm_phi = -80;
			if (!release) {
				updatetileposition();
				changetilecolour(collisioncheck());
			}
			break;
		case ' ':
			if (endGame || release) break;
			releasetile();
			break;
	}
	glutPostRedisplay();
}

void idle(void)
{
	glutPostRedisplay();
}

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(xSize, ySize);
	glutInitWindowPosition(680, 178); // Center the game window (well, on a 1920x1080 display)
	glutCreateWindow("Song's 3D Fruit Tetris");
	glewInit();
	init();

	// Callback functions
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutSpecialFunc(special);
	glutKeyboardFunc(keyboard);
	glutIdleFunc(idle);
	glutTimerFunc(100, timer, 0);

	glutMainLoop(); // Start main loop
	return 0;
}
