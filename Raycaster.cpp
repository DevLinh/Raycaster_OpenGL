﻿// Raycaster_OpenGL.cpp : This file contains the 'main' function. Program execution begins and ends there.
#include "Dependencies/glew/glew.h"
#include "Dependencies/freeglut/freeglut.h"
#include <math.h>
#define PI 3.141592653589793238462643383279502884197169399375105820974944592307816406286
#define P2 PI/2
#define P3 3*PI/2
#define DR PI/180 //một độ bằng PI/180 radian
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
	1,0,1,1,0,0,0,1,
	1,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,1,
	1,0,1,0,1,1,0,1,
	1,0,1,0,0,0,0,1,
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
	// điểm còn lại được xác định theo góc pa, truy nhiên pdx và pdx các player 1 khoảng nhó, ta nhân đồng thời với 5 để tăng chiều dài của thanh điều hướng
	glVertex2i(px + pdx * 5, py + pdy * 5);
	glEnd();
}
// tính khoảng cách 2 điểm A(ax,ay) và B(bx,by) trong mặt phẳng Oxy
float dist(float ax, float ay, float bx, float by, float ang)
{
	return  (sqrt((bx - ax) * (bx - ax) + (by - ay) * (by - ay))) ;
}

// tạo Ray - Tia
void drawRays2D() 
{
	int r, mx, my, mp, dof;
	float rx, ry, ra, x0, y0; 
	// ra: rays angle (góc của các tia)
	// gán góc của tia bằng góc của người chơi
	// rx, ry là tọa độ vị trí giao điểm của tia với hàng ngang gần điểm gần player nhất theo hướng trên lưới
	// x0, y0 là các khoảng cố định để tìm ra tọa độ giao với tường theo chiều ngang, sẽ được giải thích cụ thế trong bài báo cáo
	
	//tạo một biến lưu trữ khoảng cách sau khi quyết định chọn tia nào
	float disF;
	//ta thử làm tia lệch 30 độ so với hướng nhìn của player hay lệch so với thanh điều hướng
	ra = pa - DR * 30; if (ra < 0) { ra += 2 * PI; } if (ra > 2 * PI) { ra -= 2 * PI; }
	// ta bắt đầu với 1 tia (r < 1) :D
	for (r = 0; r < 60; r++)
	{

		// Ta bắt đầu kiểm tra theo chiều NGANG
		// cụ thể là kiểm tra theo từng hàng ngang trên lưới, nếu tia gặp chướng ngại vật thì ngắt tia tại đó
		dof = 0;
		// Khởi tạo khoảng cách ban đầu của tia thật lớn, đồng thơi lưu trữ vị trí chiều ngang của tia
		float disH = 1000000, hx = px, hy = py;
		float aTan = -1 / tan(ra);
		//Kiểm tra tia hướng lên hay xuống dựa vào giá trị ra
		if (ra > PI) // hướng lên
		{
			ry = (((int)py >> 6) << 6) - 0.0001;
			// trước hết ép py sang kiểu số nguyên để sử dụng phép dịch
			// phép dịch với tham số thứ 6 là số 6 tương đương với phép nhân với 2^6 đối với dịch trái và chia cho 2^6 đối với dịch phải
			// 2^6 = 64, vị trí mỗi hàng ngang trên lưới trên trục y lần lượt là 0, 64, 128, 192, ...
			// ví dụ py = 150, py >> 6 <=> 150 / 64 = 2, 2 << 6 <=> 2 * 64 = 128, ta trừ thêm  0.0001 làm tỷ lệ sai số do ép py sang int, có thể có hoặt có thể không trừ
			// vậy ta tìm ra hàng ngang giao với tia gần nhất theo hướng đi lên có ry gần bằng 128 (ry = 127.9999)
			// các tham số dưới được GIẢI THÍCH cụ thể hơn trong BÁO CÁO
			rx = (py - ry) * aTan + px;
			y0 = -64;
			x0 = -y0 * aTan;
		}
		if (ra < PI) // hướng xuống
		{
			ry = (((int)py >> 6) << 6) + 64;
			// player ở giữa 2 hàng ngang, hàng phía trên gần nhất đã được xác định nhờ các giải thích bên trên
			// vì vậy để tìm được hàng gần nhất hướng xuống dưới, ta đơn giản lấy hàng cách hàng gần nhất ở trên một khoảng bằng 64
			rx = (py - ry) * aTan + px;
			y0 = 64;
			x0 = -y0 * aTan;
		}
		if (ra == 0 || ra == PI) // trường hợp tia năm ngang thì tạm thời nhìn thẳng
		{
			rx = px;
			ry = py;
			dof = 8;
		}
		//chúng ta ko thê kiễm tra mãi được
		while (dof < 8)
		{
			//my và my là tọa độ
			mx = (int)(rx) >> 6;
			my = (int)(ry) >> 6;
			mp = my * mapX + mx;
			// bây giờ mỗi khi thỏa mãn điều kiện, ta câp nhật tọa độ cho hx và hy, tính lại chiều dài - khoảng cách từ player tới vị trí tia giao nhau với tường ở hàng ngang
			if (mp > 0 && mp < mapX * mapY && map[mp] == 1) { hx = rx; hy = ry; disH = dist(px, py, hx, hy, ra); dof = 8; }// tại vị trí đó là tường, thì dừng lại vòng lặp
			//ngược lại tiếp tục vẽ dài tia cho đến khi gặp tượng theo hàng ngang
			else
			{
				rx += x0;
				ry += y0;
				dof += 1;
			}
		}



		// Ta bắt đầu kiểm tra theo chiều DỌC
		// cụ thể là kiểm tra theo từng hàng ngang trên lưới, nếu tia gặp chướng ngại vật thì ngắt tia tại đó
		dof = 0;
		// Khởi tạo khoảng cách ban đầu của tia thật lớn, đồng thơi lưu trữ vị trí chiều dọc của tia
		float disV = 1000000, vx = px, vy = py;
		float nTan = -tan(ra);
		//Kiểm tra tia hướng lên hay xuống dựa vào giá trị ra
		if (ra > P2 && ra < P3) // hướng trái
		{
			rx = (((int)px >> 6) << 6) - 0.0001;
			// trước hết ép py sang kiểu số nguyên để sử dụng phép dịch
			// phép dịch với tham số thứ 6 là số 6 tương đương với phép nhân với 2^6 đối với dịch trái và chia cho 2^6 đối với dịch phải
			// 2^6 = 64, vị trí mỗi hàng ngang trên lưới trên trục y lần lượt là 0, 64, 128, 192, ...
			// ví dụ py = 150, py >> 6 <=> 150 / 64 = 2, 2 << 6 <=> 2 * 64 = 128, ta trừ thêm  0.0001 làm tỷ lệ sai số do ép py sang int, có thể có hoặt có thể không trừ
			// vậy ta tìm ra hàng ngang giao với tia gần nhất theo hướng đi lên có ry gần bằng 128 (ry = 127.9999)
			// các tham số dưới được GIẢI THÍCH cụ thể hơn trong BÁO CÁO
			ry = (px - rx) * nTan + py;
			x0 = -64;
			y0 = -x0 * nTan;
		}
		if (ra < P2 || ra > P3) // hướng phải
		{
			rx = (((int)px >> 6) << 6) + 64;
			// player ở giữa 2 hàng ngang, hàng phía trên gần nhất đã được xác định nhờ các giải thích bên trên
			// vì vậy để tìm được hàng gần nhất hướng xuống dưới, ta đơn giản lấy hàng cách hàng gần nhất ở trên một khoảng bằng 64
			ry = (px - rx) * nTan + py;
			x0 = 64;
			y0 = -x0 * nTan;
		}
		if (ra == 0 || ra == PI) // trường hợp tia năm dọc thì tạm thời nhìn thẳng
		{
			rx = px;
			ry = py;
			dof = 8;
		}
		//chúng ta ko thê kiễm tra mãi được
		while (dof < 8)
		{
			//my và my là tọa độ
			mx = (int)(rx) >> 6;
			my = (int)(ry) >> 6;
			mp = my * mapX + mx;
			if (mp > 0 && mp < mapX * mapY && map[mp] == 1) { vx = rx; vy = ry; disV = dist(px, py, vx, vy, ra); dof = 8; }// tại vị trí đó là tường, thì dừng lại vòng lặp
			//ngược lại tiếp tục vẽ dài tia cho đến khi gặp tượng theo hàng ngang
			else
			{
				rx += x0;
				ry += y0;
				dof += 1;
			}
		}
		// sau đó ta so sánh khoảng cách giữa vị trí player tới giao điểm của 2 tia tới vật cản, tìm được tia phù hợp
		if (disH > disV) { rx = vx; ry = vy; disF = disV; }
		if (disH < disV) { rx = hx; ry = hy; disF = disH; }
		// vẽ tia
		glColor3f(1, 0, 0); //tia đỏ
		glLineWidth(1); // độ dộng tia chỉ bằng 1
		glBegin(GL_LINES);//vẽ đường
		glVertex2i(px, py);
		glVertex2i(rx, ry);
		glEnd();

		//3D WALL
		// tiến hành vẽ các bức tường 3D bên màn hình phải
		// ta sẽ biễu diễn cảnh 3D trên màn hình kích thước 320x160px, khoảng cách càng xa thì vât càng nhỏ
		// mapS = 64 chính là kích thước của hình vuông trên 2D, cũng chính là kích thước khối lập phương trên không gian 3D, chính vì thế mỗi đường vẽ lên cảnh 3d sẽ nghịch với khoảng cách
		float ca = pa - ra; if (ca < 0) { ca += 2 * PI; } if (ca > 2 * PI) { ca -= 2 * PI; } disF = disF * cos(ca);
		float lineH = (320 * mapS) / disF; if (lineH > 320) { lineH = 320; }
		float lineO = 256 - lineH / 2;

		// vẽ từng đường có chiều cao lineH rộng 8 lên màn hình, vì trong vòng lặp nên nó sẽ được vẽ liên tục nhau
		glColor3f(0, 1, 0); //tia đỏ
		glLineWidth(8); 
		glBegin(GL_LINES);//vẽ đường
		glVertex2i(r*8 + 530, lineO); //lý do + 530 vì do màn hình ta sử dụng độ phân giải 1024x512,nửa kia màn hình là 512x512 đã sử dụng cho bản đồ 2D, vậy còn lại phần sau sử dụng cho 3D
		glVertex2i(r*8 + 530, lineH + lineO);
		glEnd();


		//sau khi vẽ thì tăng ra thêm 1 độ
		ra += DR; if (ra < 0) { ra += 2 * PI; } if (ra > 2 * PI) { ra -= 2 * PI; }
	}
}

void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//gọi hàm vẽ Map
	drawMap2D();
	//gọi hàm vẽ Player vào display
	drawPlayer();
	//gọi hàm vẽ tia
	drawRays2D();
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
	// kiểm tra xem tại vị trí hiện tại, nếu player tiếp tục di chuyển tới hoặc lùi và ô của tường thì không được di chuyển tiếp
	// px,py là vị trí hiện tại của player, ta chia lấy phần nguyên px,py với 64 (>> 6), tìm được 2 giá trị tương ứng x0, y0 ở trên phần vẽ bản đồ
	// sau đó ta kiểm tra giá trị tại ô đó có phải vật cản không
	int x0, y0;
	// tiếp theo phải kiểm tra thanh điều hướng, hay hướng di chuyển của player có hướng tới vật cản không px + pdx * 5, py + pdy * 5
	int pdx0, pdy0;
	pdx0 = (int)(px + pdx * 2) >> 6;
	pdy0 = (int)(py + pdy * 2) >> 6;
	if (map[pdy0 * mapX + pdx0] != 1)
	{
		//nếu ta hướng tới và chưa gặp vật cản, ta có thể chuyển tới và lùi
		// ta có thể di chuyển cho tới khi hướng di chuyển ta cách vật cản một khoảng
		if (key == 'w') { px += pdx; py += pdy; };
		//đối với đi lui, ta phải kiểm tra điều kiện khi lui
		x0 = (int)(px - pdx*2) >> 6;
		y0 = (int)(py - pdy*2) >> 6;
		if (key == 's' && map[y0 * mapX + x0] != 1) { px -= pdx; py -= pdy; };
	}
	else 
	{
		// thanh điều hướng hướng vào vật cản, ta vẫn có thể di chuyển lui, nhưng di chuyển lui vẫn phải kiểm tra nếu tiếp tục lui có gặp vật cản không
		x0 = (int)(px - pdx) >> 6;
		y0 = (int)(py - pdy) >> 6;
		if (key == 's' && map[y0 * mapX + x0] != 1) { px -= pdx; py -= pdy; };
	}
	/*
	if (key == 'w' && map[y0 * mapX + x0] != 1 && map[pdy0 * mapX + pdx0] != 1) { px += pdx; py += pdy; };
	if (key == 's' && map[y0 * mapX + x0] != 1 && map[pdy0 * mapX + pdy0] != 1) { px -= pdx; py -= pdy; };
	*/
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

