// Raycaster_OpenGL.cpp : This file contains the 'main' function. Program execution begins and ends there.
#include "Dependencies/glew/glew.h"
#include "Dependencies/freeglut/freeglut.h"
#include <math.h>
#define PI 3.141592653589793238462643383279502884197169399375105820974944592307816406286

float px, py, pdx, pdy, pa;
//px, py vị trí của player
//pdx, pdy là delta x, delta y, một khoảng lệch với vị trí player
//pa là góc nhìn của player - player angle, ở đây mang giá trị radian

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
			//A(x0,y0)--------------B(x0+mapS, y0)
			//   |						|
			//   |						|
			//   |						|
			//   |						|
			//   |						|
			//   |						|
			//   |						|
			//C(x0,y0+mapS)---------D(x0+mapS,y0+mapS)
			//glVertex2i(x0, y0);
			//glVertex2i(x0, y0 + mapS);
			//glVertex2i(x0 + mapS, y0 + mapS);
			//glVertex2i(x0 + mapS, y0);
			glVertex2i(x0 + 1, y0 + 1); //điểm A ↘
			glVertex2i(x0 + 1, y0 + mapS - 1); //điểm C ↗
			glVertex2i(x0 + mapS - 1, y0 + mapS - 1); //điểm D ↖
			glVertex2i(x0 + mapS - 1, y0 + 1); //điểm B ↙
			// mục đích tạo đường biên cho mỗi hình vuông
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

	// vẽ thanh điều hướng khi có pa (khi nhấm A hoặc D)
	glLineWidth(3); // thanh điều hướng màu vang
	glBegin(GL_LINES);
	//vị trí player chính là điểm đầu của thanh điều hướng
	glVertex2i(px, py);
	// điểm còn lại được xác định theo góc pa, truy nhiên pdx và pdx các player 1 khoảng nhó, ta nhân đồng thời với 10 để tăng chiều dài của thanh điều hướng
	glVertex2i(px + pdx * 5, py + pdy * 5);
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
	/*
	if (key == 'a') { px -= 5; }; // 'a' thì di chuyển qua trái, vị trí bị dịch chuyển giảm trên trục hoành 1 khoảng 5 đơn vị
	if (key == 'd') { px += 5; }; // ngược lại với 'a'
	if (key == 'w') { py -= 5; } // 'w' thì di chuyển lên, vị trí dịch chuyển lại gần về 0 trên trục tung, vị trí bị dịch chuyển 1 khoảng 5 đơn vị trên trục tung
	if (key == 's') { py += 5; } // ngược lại với 'w'
	*/
	// Ta cần player di chuyển linh hoạt hơn, player cần có thể xoay 360 độ và di chuyển theo hướng đó
	//Hai phim A và D giờ đóng vai trò thay đổi hướng di chuyển của player, sẽ được biểu diễn bởi một thanh điều hướng ngắn
	if (key == 'a') 
	{ 
		pa -= 0.1;  // A hướng sang trái, đồng nghĩa việc xoay ngược chiều kim đồng hồ
		// đưa pa về 2PI khi pa < 0
		if (pa < 0) 
		{ 
			pa += 2 * PI; //360 độ tương đương 2*PI
		} 
		// tính tọa độ đỉnh của thanh điều hướng khi biết góc thông qua HỆ THỨC LƯỢNG TRONG TAM GIÁC VUÔNG
		// giá trị 5 ở công thức dưới chính là khoảng cách giữa vị trí player so với điểm cuối ( đầu còn lại của thanh điều hướng )
		pdx = cos(pa) * 5; 
		pdy = sin(pa) * 5; 
	};
	if (key == 'd') 
	{ 
		pa += 0.1; 
		if (pa > 2 * PI) 
		{ 
			pa -= 2 * PI; 
		} 
		pdx = cos(pa) * 5; 
		pdy = sin(pa) * 5; 
	};
	// Sau khi xác định được hướng, W và S đơn giản thay đổi vị trí tiến hoặc lùi của player
	if (key == 'w') { px += pdx; py += pdy; };
	if (key == 's') { px -= pdx; py -= pdy; };
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

