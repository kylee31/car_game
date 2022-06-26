#include <iostream>
#include "GL/glew.h"
#include "GL/freeglut.h"
#include "glm/glm.hpp"
#include "glm/ext.hpp"

#include "CarGame0App.h"


GLuint g_window_w = 800;
GLuint g_window_h = 600;




int main(int argc, char** argv)
{

	glutInit(&argc, argv);
	
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_MULTISAMPLE);
	glutInitWindowSize(g_window_w, g_window_h);
	glutSetOption(GLUT_MULTISAMPLE, 8);
	glEnable(GL_MULTISAMPLE);
	glutCreateWindow("Computer Graphics");
	
	glutReshapeFunc(Reshape);
	glutDisplayFunc(Display);

	//키보드 눌렸을때
	glutKeyboardFunc(Keyboard);

	//키보드에서 손 뗐을때
	glutKeyboardUpFunc(KeyboardUp);
	glutMouseFunc(Mouse);
	glutMotionFunc(MouseMotion);
	glutTimerFunc((unsigned int)(1000 /60), Timer, (1000 / 60));



	glewExperimental = GL_TRUE;
	if ( glewInit() != GLEW_OK )
	{
		std::cerr << "Unable to initialize GLEW ... exiting" << std::endl;
		exit(EXIT_FAILURE);
	}
	else
	{
		std::cout << "GLEW OK\n";
	}


	InitOpenGL();

	glutMainLoop();


	ClearOpenGLResource();

	return 0;
}








