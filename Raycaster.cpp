// Raycaster_OpenGL.cpp : This file contains the 'main' function. Program execution begins and ends there.
#include "Dependencies/glew/glew.h"
#include "Dependencies/freeglut/freeglut.h"
#include <math.h>
#include <stdio.h>
#include <sys/timeb.h>
#include <sys/utime.h>
#include <string>
static int point;  // Điểm của người chơi
#define fps 25
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
//Định nghĩa food
typedef struct Food {
	int xf, yf;//Tọa độ tâm khối hình vuông
	int size = 10+rand()%20;//Size thức ăn = 1/2 độ dài cạnh
};

//tạo một biến lưu trữ food cho toàn chương trình
Food food;
//taọ biến lưu trữ điểm
int points;
//Hàm kiểm tra food có chạm với wall
bool checkFood(int x, int y) {
	int pfx, pfy;
	pfx = (int)x >> 6;
	pfy = (int)y >> 6;
	return (map[pfy * mapX + pfx]!=0);
}

//kiem tra xem food bi an chua (player cham vào hay vào bên trong mồi được coi là ăn mồi)
bool isEated(Food f)
{
	return (px > f.xf - f.size / 2  && px  < f.xf + f.size / 2) && (py > f.yf - f.size / 2 && py < f.yf + f.size/2);
}
//Vẽ food- bây giờ ta sẽ chỉnh sửa lại, sẽ random vị trí mới cho mồi nếu kiểm tra là mồi random lúc khởi tạo bị ăn, hàm draw bây giờ ko chịu trách nhiệm tạo random mồi nữa, việc này sẽ do một hàm riêng biệt đảm nhiệm
//tạo hàm tạo mồi ngẫu nhiên phù hợp điều kiện
Food randomFood()
{
	Food f;
	int i = 0;
	do {
		f.xf = (rand() % (480 - 64 + 1)) + 64;
		f.yf = (rand() % (480 - 64 + 1)) + 64;
		i++;
	} while (checkFood(f.xf - f.size / 2, f.yf - f.size / 2) || checkFood(f.xf + f.size / 2, f.yf - f.size / 2) || checkFood(f.xf + f.size / 2, f.yf + f.size / 2) || checkFood(f.xf - f.size / 2, f.yf + f.size / 2));
	return f;
}
void drawFood(Food f) {
	glColor3f(0.5, 0, 1);//Màu sắc food
	glBegin(GL_QUADS);
	//Vẽ tọa độ các đỉnh của food
	glVertex2i(f.xf - f.size / 2, f.yf - f.size / 2);
	glVertex2i(f.xf + f.size / 2, f.yf - f.size / 2);
	glVertex2i(f.xf + f.size / 2, f.yf + f.size / 2);
	glVertex2i(f.xf - f.size / 2, f.yf + f.size / 2);
	glEnd();
}
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
	glColor3f(1, 0, 0); // màu đỏ
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
// để tìm giao điểm giữa mồi với tia, ta quy về bài toán tìm giao điểm giữa 2 đường thẳng, đồng thời kiểm tra giá trị của giao điểm đó có năm trong khoảng của food không
// cho nên bước đầu tiên ta cần tìm ra giao điểm của phương trình đường thẳng tia và mặt tiếp tia
// bước đầu tiên ta cần xây dựng được phương trình đường thẳng của tia và phương trình đường thẳng của mặt tiếp tia
struct crossPoint
{
	float cx;
	float cy;
	float discp = 100000;
};

crossPoint minDistCP(crossPoint cp[])
{
	crossPoint minDistP = cp[0];
	for (int i = 1; i < 4; i++)
	{
		if (cp[i].discp < minDistP.discp)
		{
			minDistP = cp[i];
		}
	}
	if (minDistP.discp != 100000)
	{
		return minDistP;
	}
	else
	{
		minDistP.cx = minDistP.cy = 0;
		return minDistP;
	}
	
}
// một phương trình đường thẳng dạng y = ax + b; với 2 điểm cho trước (gồm (px,py) và (rx, ry) sau khi tìm ra giao điểm của tia với tường, ta giải bài toán hệ phương trình 2 ẩn tìm ra cặp hệ số a và b, từ đó tìm ra phương trình đường thẳng của tia
// sau đó truyền và phương trình đường thẳng của mặt tiếp tia, ta tìm ra giao điểm
crossPoint findCrossPoint(float px, float py, float rx, float ry, float ra)
{
	// py = a.px + b
	// ry = a.rx + b
	float a = (ry - py) / (rx - px);
	float b = py - a * px;
	// vì bài toán xét giao điểm đường thẳng nên không xét theo hướng lên hay xuống, ta xét theo hướng là dọc hay ngang
	// xét khối thức ăn có 4 mặt, 2 mặt trên dưới tương ứng với hai đường thẳng y và hai mặt trái phải với 2 đường thẳng x
	// x = food.xf + food.size/2 (1)
	// x = food.xf - food.size/2 (2)
	// y = food.yf + food.size/2 (3)
	// y = food.yf - food.size/2 (4)
	crossPoint cpH1, cpH2; // lần lượt là giao điểm với đường thẳng mặt ngang (3) và (4)
	crossPoint cpV1, cpV2; // lần lượt là giao điểm với đường thẳng 2 mặt trái phải (4) và (3)
	cpH1.cx = cpH2.cx = cpH1.cy = cpH2.cy = cpV1.cx = cpV2.cx = cpV1.cy = cpV2.cy = 0;
	if (ra != 0 && ra != PI)
	{
		cpH1.cy = food.yf + food.size / 2;
		cpH2.cy = food.yf - food.size / 2;

		cpH1.cx = (cpH1.cy - b) / a;
		cpH2.cx = (cpH2.cy - b) / a;
	}
	if (ra != P2 && ra != P3)
	{
		cpV1.cx = food.xf - food.size / 2;
		cpV2.cx = food.xf + food.size / 2;

		cpV1.cy = a * cpV1.cx + b;
		cpV2.cy = a * cpV2.cx + b;
	}
	//float discpH1, discpH2, discpV1, discpV2 = 100000;
	if ( (cpH1.cx >= (food.xf - food.size / 2)) && (cpH1.cx <= (food.xf + food.size / 2) ))
	{
		cpH1.discp = dist(px, py, cpH1.cx, cpH1.cy, ra);
	}
	if ((cpH2.cx >= (food.xf - food.size / 2)) && (cpH2.cx <= (food.xf + food.size / 2)))
	{
		cpH2.discp = dist(px, py, cpH2.cx, cpH2.cy, ra);
	}
	if ((cpV1.cy >= (food.yf - food.size / 2)) && (cpV1.cy <= (food.yf + food.size / 2)))
	{
		cpV1.discp = dist(px, py, cpV1.cx, cpV1.cy, ra);
	}
	if ((cpV2.cy >= (food.yf - food.size / 2)) && (cpV2.cy <= (food.yf + food.size / 2)))
	{
		cpV2.discp = dist(px, py, cpV2.cx, cpV2.cy, ra);
	}
	crossPoint cp[] = { cpH1, cpH2, cpV1, cpV2 };
	return minDistCP(cp);
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
	float fx, fy;
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
			// giao điểm giữa tia với khối thức ăn - trường hợp player và khối food ở trong cùng 1 ô
			if ( ((int)px >> 6 == food.xf >> 6) && ((int)py >> 6 == food.yf) )
			{
				fy = food.yf - food.size / 2;
				fx = rx = (py - fy) * aTan + px;
			}
		}
		if (ra < PI) // hướng xuống
		{
			ry = (((int)py >> 6) << 6) + 64;
			// player ở giữa 2 hàng ngang, hàng phía trên gần nhất đã được xác định nhờ các giải thích bên trên
			// vì vậy để tìm được hàng gần nhất hướng xuống dưới, ta đơn giản lấy hàng cách hàng gần nhất ở trên một khoảng bằng 64
			rx = (py - ry) * aTan + px;
			y0 = 64;
			x0 = -y0 * aTan;
			if ((int)px >> 6 == food.xf >> 6 && (int)py >> 6 == food.yf)
			{
				fy = food.yf + food.size / 2;
				fx = rx = (py - fy) * aTan + px;
			}
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

		// ta kiểm tra nếu trả thanh điều hướng (đỉnh thanh) và food cũng trong góc phần tư thì mới tính toán giao điểm
		// nếu kết quả tìm điểm giao là một điểm có tọa độ (0,0) thì chính là tia đó ko giao với food
		float disCP = 100000;
		if ((food.xf - px) * (rx - px) > 0 && (food.yf - py) * (ry - py) > 0)
		{
			crossPoint cp = findCrossPoint(px, py, rx, ry, ra);
			if (cp.cx != 0 && cp.cy != 0 && cp.discp < disF)
			{
				printf("crossPoint (%f, %f)\n", cp.cx, cp.cy);
				printf("dis = %f\n", cp.discp);
				// vẽ tia chiếu tới mồi
				glColor3f(0, 1, 0); //tia xanh
				glLineWidth(1); // độ dộng tia chỉ bằng 1
				glBegin(GL_LINES);//vẽ đường
				glVertex2i(px, py);
				glVertex2i(cp.cx, cp.cy);
				glEnd();
				disCP = cp.discp;
			}
		}

	
		// vẽ tia chiếu tới tường
		if (disF < disCP) {
			glColor3f(1, 1, 1); //tia trắng
			glLineWidth(1); // độ dộng tia chỉ bằng 1
			glBegin(GL_LINES);//vẽ đường
			glVertex2i(px, py);
			glVertex2i(rx, ry);
			glEnd();
		}
		
		

		//3D WALL
		// tiến hành vẽ các bức tường 3D bên màn hình phải
		// ta sẽ biễu diễn cảnh 3D trên màn hình kích thước 320x160px, khoảng cách càng xa thì vât càng nhỏ
		// mapS = 64 chính là kích thước của hình vuông trên 2D, cũng chính là kích thước khối lập phương trên không gian 3D, chính vì thế mỗi đường vẽ lên cảnh 3d sẽ nghịch với khoảng cách
		float ca = pa - ra; if (ca < 0) { ca += 2 * PI; } if (ca > 2 * PI) { ca -= 2 * PI; } disF = disF * cos(ca);
		float lineH = (320 * mapS) / disF; if (lineH > 320) { lineH = 320; } // chiều cao của mỗi đường tường
		float lineO = 256 - lineH / 2;

		// vẽ từng đường có chiều cao lineH rộng 8 lên màn hình, vì trong vòng lặp nên nó sẽ được vẽ liên tục nhau
		glColor3f(0, 1, 0); //các dòng màu xanh
		glLineWidth(8); 
		glBegin(GL_LINES);//vẽ đường
		glVertex2i(r*8 + 530, lineO); //lý do + 530 vì do màn hình ta sử dụng độ phân giải 1024x512,nửa kia màn hình là 512x512 đã sử dụng cho bản đồ 2D, vậy còn lại phần sau sử dụng cho 3D
		glVertex2i(r*8 + 530, lineH + lineO);
		glEnd();

		if (disF > disCP)
		{
			disCP = disCP * cos(ca);
			float foodH = (320 * food.size) / disCP; if (foodH > 320) { foodH = 320; }// chiều cao của khối thức ăn
			float foodO = 256 - foodH / 2;
			glColor3f(0.5, 0, 1); //các dòng màu xanh
			glLineWidth(8);
			glBegin(GL_LINES);//vẽ đường
			glVertex2i(r * 8 + 530, foodO); //lý do + 530 vì do màn hình ta sử dụng độ phân giải 1024x512,nửa kia màn hình là 512x512 đã sử dụng cho bản đồ 2D, vậy còn lại phần sau sử dụng cho 3D
			glVertex2i(r * 8 + 530, foodH + foodO);
			glEnd();
		}

		//sau khi vẽ thì tăng ra thêm 1 độ
		ra += DR; if (ra < 0) { ra += 2 * PI; } if (ra > 2 * PI) { ra -= 2 * PI; }
	}
}
//Hàm  đếm milli giây
int getMilliCount() {
	timeb tb;
	ftime(&tb);
	int nCount = tb.millitm + (tb.time & 0xfffff) * 1000;
	return nCount;
}
//Hàm cho máy nghĩ đến sleeptime milli giây
void sleep(int sleeptime)
{
	int count = 0;
	int beginsleep = getMilliCount();
	while (getMilliCount() - beginsleep < sleeptime)
	{
		count++;
	}
}
//Vẽ chữ 
void drawText(const std::string& text, const unsigned int x, const unsigned int y, const float r, const float g, const float b)
{
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, 200, 0, 200, 0, 1);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glColor3f(r, g, b); //Chỉnh màu chữ
	glRasterPos2i(x, y); //Di chuyển tới vị trí x,y. tính theo pixel
	for (const char c : text)
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, (int)c); //thực hiện vẽ từng kí tự của chuỗi kí tự
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
}
void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//gọi hàm vẽ Map
	int beginFrame = getMilliCount();

	//gọi hàm vẽ Map
	drawMap2D();
	//gọi hàm vẽ Player vào display
	drawPlayer();
	if (isEated(food))
	{
		points += food.size;
		food = randomFood();
		printf("new %d, %d\n", food.xf, food.yf);
		printf("points %d\n", points);
	}
	else
	{
		drawFood(food);
	}
	std::string s = "Points: " + std::to_string(point);
	drawText(s, 100, 190, 1, 0.5, 0);

	std::string s1 = "Exist time: ";
	drawText(s1, 170, 190, 1, 0.5, 0);
	//gọi hàm vẽ tia
	drawRays2D();

	int timeDiff = getMilliCount() - beginFrame;//Thời gian thự hiện xong chu trình lặp
	int sleepTime = (int)((1000 / fps) - timeDiff);
	int framesSkipped = 0;//Số khung hình đã bỏ qua


	if (sleepTime > 0) {
		//Máy dựng được nhiều hơn 25fps thì cho máy sleep
		sleep(sleepTime);
	}
	//Skip tối đa 5 frame (nếu máy không dựng đủ 25fps )
	while (sleepTime < 0 && framesSkipped <= 5) {
		display();
		sleepTime + (1000 / fps);
		framesSkipped++;
	}
	if (framesSkipped >= 5) {
		printf("Xin lỗi, game quá đẳng cấp.");
		exit(2);

	}
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
	//khởi tạo một giá trị ban đầu phù hợp cho food
	food = randomFood();
	printf("%d, %d\n", food.xf, food.yf);
	printf("an roi: %d\n", isEated(food));
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

