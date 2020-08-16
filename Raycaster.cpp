﻿// Raycaster_OpenGL.cpp : This file contains the 'main' function. Program execution begins and ends there.
#include "Dependencies/glew/glew.h"
#include "Dependencies/freeglut/freeglut.h"
#include <math.h>

float px, py; // vị trí của player

//khởi tạo hàm vẽ người chơi trên bản đồ
void drawPlayer() {
	glColor3f(1, 1, 0); // màu vàng
	glPointSize(8); // player là 1 điểm màu vàng có kích cỡ 8 pixels
	glBegin(GL_POINTS);
	glVertex2i(px, py);
	glEnd();
}

void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//gọi hàm vẽ Player vào display
	drawPlayer();
	glutSwapBuffers();
}

// di chuyển player khi nhấn ADWS, hiện tại chỉ di chuyển ngang và dọc, chưa thể di chuyển chéo
void buttons(unsigned char key, int x, int y)
{
	// di chuyển vị trí 1 khoảng 5 đơn vị tùy hướng di chuyển
	if (key == 'a') { px -= 5; }; // 'a' thì di chuyển qua trái, vị trí bị dịch chuyển giảm trên trục hoành 1 khoảng 5 đơn vị
	if (key == 'd') { px += 5; }; // ngược lại với 'a'
	if (key == 'w') { py -= 5; } // 'w' thì di chuyển lên, vị trí dịch chuyển lại gần về 0 trên trục tung, vị trí bị dịch chuyển 1 khoảng 5 đơn vị trên trục tung
	if (key == 's') { py += 5; } // ngược lại với 'w'
	glutPostRedisplay(); // tiến hành vẽ lại trên màn hình mỗi khi nhấn phím
}

void init()
{
	//set màu nền về màu đen xám
	glClearColor(0.3, 0.3, 0.3, 0);
	//set window dimension 1024 x 512 pixels
	gluOrtho2D(0, 1024, 512, 0);
	// khởi tạo vị trí ban đầu cho player bằng tay 
	px = 300;
	py = 300;
};

int main(int argc, char** argv)
{
	glutInit(&argc, argv); //Initialize GLUT
	glutInitDisplayMode(GLUT_SINGLE || GLUT_RGBA); //Enable double bufferd mode
	glutInitWindowSize(1024, 512); //Khởi tạo cửa sổ 1024x512
	glutInitWindowPosition(100, 100); // vị trí cửa sổ khởi đầu 100x100
	glutCreateWindow("Raycaster");
	init();
	glutDisplayFunc(display);
	// set Keyboard Func là hàm buttons, di chuyển player
	glutKeyboardFunc(buttons);
	glutMainLoop();
	return 0;
}

