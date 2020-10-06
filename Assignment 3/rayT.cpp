#include "include/Angel.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include "trace.h"
#include "global.h"
#include "sphere.h"
#include "image_util.h"
#include "scene.h"

using namespace std;

int win_width = WIN_WIDTH;
int win_height = WIN_HEIGHT;

GLfloat frame[WIN_HEIGHT][WIN_WIDTH][3];   

float image_width = IMAGE_WIDTH;
float image_height = (float(WIN_HEIGHT) / float(WIN_WIDTH)) * IMAGE_WIDTH;

// some colors
RGB_float background_clr; // background color
RGB_float null_clr = {0.0, 0.0, 0.0};   // NULL color

//
// these view parameters should be fixed
//
Point eye_pos = {0.0, 0.0, 0.0};  // eye position
float image_plane = -1.5;           // image plane position

// list of spheres in the scene
Spheres *scene = NULL;

// light 1 position and color
Point light1;
float light1_ambient[3];
float light1_diffuse[3];
float light1_specular[3];

// global ambient term
float global_ambient[3];

// light decay parameters
float decay_a;
float decay_b;
float decay_c;

// maximum level of recursions; you can use to control whether reflection
// is implemented and for how many levels
int step_max = 1;

int shadow_on = 0;
int reflections_on = 0;
int chessboard_on = 0;
int refractions_on = 0;
int stochastic_on = 0;
int supersampling_on = 0;

//Chessboard plane definition using a point and a normal vector

Point chess_point = {0, 3, 0};
Vector chess_board_normal = {0, 15, 4}; //MUST BE NORMALIZED BEFORE USE

float cb_ambient_black[3] = {1, 1, 1};    // material property used in Phong model
float cb_ambient_white[3] = {1, 1, 1};    // material property used in Phong model
float cb_diffuse_black[3] = {1,1,1};
float cb_diffuse_white[3] = {1,1,1};
float cb_specular_black[3] = {1,1,1};
float cb_specular_white[3] = {1,1,1};		
float cb_shineness = 1;
float cb_reflectance = 0.5;


// OpenGL
const int NumPoints = 6;

void init()
{
	// Vertices of a square
	double ext = 1.0;
	vec4 points[NumPoints] = {
		vec4( -ext, -ext,  0, 1.0 ), //v1
		vec4(  ext, -ext,  0, 1.0 ), //v2
		vec4( -ext,  ext,  0, 1.0 ), //v3	
		vec4( -ext,  ext,  0, 1.0 ), //v3	
		vec4(  ext, -ext,  0, 1.0 ), //v2
		vec4(  ext,  ext,  0, 1.0 )  //v4
	};

	// Texture coordinates
	vec2 tex_coords[NumPoints] = {
		vec2( 0.0, 0.0 ),
		vec2( 1.0, 0.0 ),
		vec2( 0.0, 1.0 ),
		vec2( 0.0, 1.0 ),
		vec2( 1.0, 0.0 ),
		vec2( 1.0, 1.0 )
	};

	// Initialize texture objects
	GLuint texture;
	glGenTextures( 1, &texture );

	glBindTexture( GL_TEXTURE_2D, texture );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, WIN_WIDTH, WIN_HEIGHT, 0,
		GL_RGB, GL_FLOAT, frame );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	glActiveTexture( GL_TEXTURE0 );

	// Create and initialize a buffer object
	GLuint buffer;
	glGenBuffers( 1, &buffer );
	glBindBuffer( GL_ARRAY_BUFFER, buffer );
	glBufferData( GL_ARRAY_BUFFER, sizeof(points) + sizeof(tex_coords), NULL, GL_STATIC_DRAW );
	GLintptr offset = 0;
	glBufferSubData( GL_ARRAY_BUFFER, offset, sizeof(points), points );
	offset += sizeof(points);
	glBufferSubData( GL_ARRAY_BUFFER, offset, sizeof(tex_coords), tex_coords );

	// Load shaders and use the resulting shader program
	GLuint program = InitShader( "vshader.glsl", "fshader.glsl" );
	glUseProgram( program );

	// set up vertex arrays
	offset = 0;
	GLuint vPosition = glGetAttribLocation( program, "vPosition" );
	glEnableVertexAttribArray( vPosition );
	glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0,
		BUFFER_OFFSET(offset) );

	offset += sizeof(points);
	GLuint vTexCoord = glGetAttribLocation( program, "vTexCoord" ); 
	glEnableVertexAttribArray( vTexCoord );
	glVertexAttribPointer( vTexCoord, 2, GL_FLOAT, GL_FALSE, 0,
		BUFFER_OFFSET(offset) );

	glUniform1i( glGetUniformLocation(program, "texture"), 0 );

	glClearColor( 1.0, 1.0, 1.0, 1.0 );
}

void display( void )
{
	glClear( GL_COLOR_BUFFER_BIT );
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_CULL_FACE);

	glDrawArrays( GL_TRIANGLES, 0, NumPoints );

	glutSwapBuffers();
}

void keyboard(unsigned char key, int x, int y)
{
	switch (key) {
	case 'q':case 'Q':
		free(scene);
		exit(0);
		break;
	case 's':case 'S':
		save_image();
		glutPostRedisplay();
		break;
	default:
		break;
	}
}


int main( int argc, char **argv )
{
	if (argc < 2) {
		printf("Missing arguments ... use:\n");
		printf("./rayT <scene#>\n");
		return -1;
	}

	set_up_default_scene();
	step_max = 5;

	if (!strcmp(argv[1], "1")){
		chessboard_on = 1;
		//cout << "in" << endl;
	}
	else if (!strcmp(argv[1], "2")){
		shadow_on = 1;
		reflections_on = 1;
		chessboard_on = 1;
	}
	else if (!strcmp(argv[1], "3")){
		refractions_on = 1;
		chessboard_on = 1;
	}
	else{
		cb_ambient_black[0] = 0;
		cb_ambient_black[1] = 0;
		cb_ambient_black[2] = 0;    
		cb_diffuse_black[0] = 0;
		cb_diffuse_black[1] = 0;
		cb_diffuse_black[2] = 0;
		cb_specular_black[0] = 0;
		cb_specular_black[1] = 0;
		cb_specular_black[2] = 0;
		shadow_on = 1;
		reflections_on = 1;
		chessboard_on = 1;
		refractions_on = 1;
		stochastic_on = 1;
		supersampling_on = 1;
	}


	printf("Rendering scene using Song's ray tracer ...\n");
	ray_trace();

	histogram_normalization();

	glutInit( &argc, argv );
	glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE );
	glutInitWindowSize( WIN_WIDTH, WIN_HEIGHT );
	glutCreateWindow( "Assignment 3" );
	glewInit();
	init();

	glutDisplayFunc( display );
	glutKeyboardFunc( keyboard );
	glutMainLoop();
	return 0;
}
