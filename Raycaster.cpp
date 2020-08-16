﻿// Raycaster_OpenGL.cpp : This file contains the 'main' function. Program execution begins and ends there.
#include "Dependencies/glew/glew.h"
#include "Dependencies/freeglut/freeglut.h"
#include <math.h>

float px, py; // vị trí của player

int mapX = 8, mapY = 8, mapS = 64; // bản đồ là một lưới vuông kích thước 8x8, mỗi ô có chiều dài cạnh là 64
// ta lưu trữ thông tin của bản đồ trong 1 mảng 1 chiều 64 phần tử, giá trị 1 đại diện cho tường, chướng ngại vật
// 0 đại diện cho khoảng không
int map[] =
{
	1,1,1,1,1,1,1,1,
	1,0,0,0,0,0,0,1,
	1,0,0,1,0,0,0,1,
	1,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,1,
	1,0,0,0,1,1,0,1,
	1,0,0,0,0,0,0,1,
	1,1,1,1,1,1,1,1
};

// khởi tạo hàm vẽ bản đồ lên màn hình
void drawMap2D()
{
	int x, y, x0, y0;
	for (y = 0;y < mapY;y++) {
		for (x = 0;x < mapX;x++) {
			if (map[y * mapX + x] == 1) {
				glColor3f(1, 1, 1); // tường màu trắng
			}
			else {
				glColor3f(0, 0, 0); // khoảng không thì màu đen
			}
			x0 = x * mapS;
			y0 = y * mapS;
			glBegin(GL_QUADS); // gọi vẽ hình thang trong GL, ta dùng nó vẽ hình vuông
			//(x0,y0)--------------(x0+mapS, y0)
			//   |						|
			//   |						|
			//   |						|
			//   |						|
			//   |						|
			//   |						|
			//   |						|
			//(x0,y0+mapS)---------(x0+mapS,y0+mapS)
			glVertex2i(x0, y0);
			glVertex2i(x0, y0 + mapS);
			glVertex2i(x0 + mapS, y0 + mapS);
			glVertex2i(x0 + mapS, y0);
			glEnd();
		}
	}
}

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
	//gọi hàm vẽ Map
	drawMap2D();
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

