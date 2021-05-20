
#include <math.h>
#include <stdlib.h>

#if defined(__APPLE__)                                                                                                                                                                                                            
#include <OpenGL/gl.h>                                                                                                                                                                                                            
#include <OpenGL/glu.h>                                                                                                                                                                                                           
#include <GLUT/glut.h>                                                                                                                                                                                                            
#else                                                                                                                                                                                                                             
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)                                                                                                                                                                       
#include <windows.h>                                                                                                                                                                                                              
#endif                                                                                                                                                                                                                            
#include <GL/gl.h>                                                                                                                                                                                                                
#include <GL/glu.h>                                                                                                                                                                                                               
#include <GL/glut.h>                                                                                                                                                                                                              
#endif          

#define PI 3.14159265358979323846
#define spring_const 0.0001
#define gravity_const 0.000009

#define Black        Color(0,0,0)
#define White        Color(1,1,1)
#define Yellow       Color(1,1,0)
#define LightBrown   Color(0.25,0.25,0)
#define Brown        Color(0.22,0.22,0)
#define DarkBrown    Color(0.2,0.2,0)
#define Red          Color(0.9,0,0)
#define Green        Color(0,0.5,0)
#define GoldenYellow Color(0.9,0.9,0)
#define LightGreen   Color(0.25,0.75,0.1)

#define max(x,y) (x>y)?(x):(y)
#define min(x,y) (x<y)?(x):(y)

const int screenWidth = 600;	// alkalmazás ablak felbontása
const int screenHeight = 600;

//--------------------------------------------------------
// 3D Vektor
//--------------------------------------------------------
struct Vector {
	float x, y, z;

	Vector() {
		x = y = z = 0;
	}
	Vector(float x0, float y0, float z0 = 0) {
		x = x0; y = y0; z = z0;
	}
	Vector operator*(float a) {
		return Vector(x * a, y * a, z * a);
	}
	Vector operator+(const Vector& v) {
		return Vector(x + v.x, y + v.y, z + v.z);
	}
	Vector operator-(const Vector& v) {
		return Vector(x - v.x, y - v.y, z - v.z);
	}
	float operator*(const Vector& v) { 	// dot product
		return (x * v.x + y * v.y + z * v.z);
	}
	Vector operator%(const Vector& v) { 	// cross product
		return Vector(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x);
	}
	float Length() { return sqrt(x * x + y * y + z * z); }
};

//--------------------------------------------------------
// Spektrum illetve szin
//--------------------------------------------------------
struct Color {
	float r, g, b;

	Color() {
		r = g = b = 0;
	}
	Color(float r0, float g0, float b0) {
		r = r0; g = g0; b = b0;
	}
	Color operator*(float a) {
		return Color(r * a, g * a, b * a);
	}
	Color operator*(const Color& c) {
		return Color(r * c.r, g * c.g, b * c.b);
	}
	Color operator+(const Color& c) {
		return Color(r + c.r, g + c.g, b + c.b);
	}
	bool operator==(const Color& c) {
		return r == c.r && b == c.b && g == c.g;
	}
};

class Shape
{
protected:
	Color color;
	Vector center;
	float angle;
	bool points_valid;
public:
	Shape(Color color) :color(color), center(0, 0, 0), angle(0) {}
	Shape(Color color, Vector center) :color(color), center(center), angle(0), points_valid(false) {}
	virtual ~Shape() {}
	Color& _Color() { return color; }
	virtual void Move(Vector center) { this->center = center;  points_valid = false; }
	virtual void Rotate(float angle) { this->angle = angle;   points_valid = false; }
	virtual void Transform(float newx_oldx, float newx_oldy, float newx_oldz, float newy_oldx, float newy_oldy, float newy_oldz, float newz_oldx, float newz_oldy, float newz_oldz)
	{ }
	virtual bool isCollide(Vector& vect) { return false; }
	virtual bool isCollide(Shape* shape) { return false; }
	virtual Vector* GetPoints() { return 0; }
	virtual int GetPointsLength() { return 0; }
	Vector& Position() { return center; }
	float& Angle() { return angle; }
	virtual void Draw() = 0;
};
class Triangle : public Shape
{
	Vector p[3];
public:
	Triangle() :Shape(Black) {}
	Triangle(Vector p1, Vector p2, Vector p3, Color color) : Shape(color)
	{
		p[0] = p1; p[1] = p2; p[2] = p3;
	}
	virtual void Transform(float newx_oldx, float newx_oldy, float newx_oldz, float newy_oldx, float newy_oldy, float newy_oldz, float newz_oldx, float newz_oldy, float newz_oldz)
	{
		Vector old;
		for (int i = 0; i < 3; i++) {
			old = p[i];
			p[i].x = old.x * newx_oldx + old.y * newx_oldy + old.z * newx_oldz;
			p[i].y = old.x * newy_oldx + old.y * newy_oldy + old.z * newy_oldz;
			p[i].z = old.x * newz_oldx + old.y * newz_oldy + old.z * newz_oldz;
		}
	}
	virtual void Draw()
	{
		glColor3f(color.r, color.g, color.b); float _cos = cos(angle), _sin = sin(angle);
		glBegin(GL_TRIANGLES);
		glVertex2f(center.x + p[0].x * _cos + p[0].y * _sin, center.y + p[0].y * _cos - p[0].x * _sin);
		glVertex2f(center.x + p[1].x * _cos + p[1].y * _sin, center.y + p[1].y * _cos - p[1].x * _sin);
		glVertex2f(center.x + p[2].x * _cos + p[2].y * _sin, center.y + p[2].y * _cos - p[2].x * _sin);
		glEnd();
	}
	friend void CreateRubber();
	friend void UpdateRubber();
};
class Quad : public Shape
{
	Vector p[4];
public:
	Quad(Vector p1, Vector p2, Vector p3, Vector p4, Color color) : Shape(color)
	{
		p[0] = p1; p[1] = p2; p[2] = p3; p[3] = p4;
	}
	static Quad* CreateRectangle(float left, float top, float width, float height, Color color)
	{
		Quad* rect = new Quad(Vector(left, top), Vector(left + width, top), Vector(left + width, top - height), Vector(left, top - height), color);
		return rect;
	}
	virtual void Transform(float newx_oldx, float newx_oldy, float newx_oldz, float newy_oldx, float newy_oldy, float newy_oldz, float newz_oldx, float newz_oldy, float newz_oldz)
	{
		Vector old;
		for (int i = 0; i < 4; i++) {
			old = p[i];
			p[i].x = old.x * newx_oldx + old.y * newx_oldy + old.z * newx_oldz;
			p[i].y = old.x * newy_oldx + old.y * newy_oldy + old.z * newy_oldz;
			p[i].z = old.x * newz_oldx + old.y * newz_oldy + old.z * newz_oldz;
		}
	}
	virtual void Draw()
	{
		glColor3f(color.r, color.g, color.b); float _cos = cos(angle), _sin = sin(angle);
		glBegin(GL_QUADS);
		glVertex2f(center.x + p[0].x * _cos + p[0].y * _sin, center.y + p[0].y * _cos - p[0].x * _sin);
		glVertex2f(center.x + p[1].x * _cos + p[1].y * _sin, center.y + p[1].y * _cos - p[1].x * _sin);
		glVertex2f(center.x + p[2].x * _cos + p[2].y * _sin, center.y + p[2].y * _cos - p[2].x * _sin);
		glVertex2f(center.x + p[3].x * _cos + p[3].y * _sin, center.y + p[3].y * _cos - p[3].x * _sin);
		glEnd();
	}
};

class Circle : public Shape
{
	Vector middle;
	float radius;
	int resolution;
public:
	Circle(Vector center, float radius, Color color, int resolution = 60) :
		middle(center), radius(radius), resolution(resolution), Shape(color) {}
	virtual void Transform(float newx_oldx, float newx_oldy, float newx_oldz, float newy_oldx, float newy_oldy, float newy_oldz, float newz_oldx, float newz_oldy, float newz_oldz)
	{
		Vector old = middle;
		middle.x = old.x * newx_oldx + old.y * newx_oldy + old.z * newx_oldz;
		middle.y = old.x * newy_oldx + old.y * newy_oldy + old.z * newy_oldz;
		middle.z = old.x * newz_oldx + old.y * newz_oldy + old.z * newz_oldz;
	}
	virtual void Draw()
	{
		float sector_angle = 0; Vector rotated_middle = Vector(middle.x * cos(angle) + middle.y * sin(angle), middle.y * cos(angle) - middle.x * sin(angle));
		glColor3f(color.r, color.g, color.b);
		glBegin(GL_TRIANGLE_FAN);
		glVertex2f(center.x + rotated_middle.x, center.y + rotated_middle.y);
		for (int i = 0; i <= resolution; i++, sector_angle += (2 * PI) / ((float)resolution))
			glVertex2f(center.x + rotated_middle.x + cos(sector_angle) * radius, center.y + rotated_middle.y + sin(sector_angle) * radius);
		glEnd();
	}
};
class EllIpse : public Shape
{
	Vector middle;
	float rX, rY;
	int resolution;
	Vector* points;
public:
	EllIpse(Vector center, float radiusX, float radiusY, Color color, int resolution = 60) :
		middle(center), rX(radiusX), rY(radiusY), resolution(resolution), Shape(color), points(0) {}
	virtual void Transform(float newx_oldx, float newx_oldy, float newx_oldz, float newy_oldx, float newy_oldy, float newy_oldz, float newz_oldx, float newz_oldy, float newz_oldz)
	{
		Vector old = middle;
		middle.x = old.x * newx_oldx + old.y * newx_oldy + old.z * newx_oldz;
		middle.y = old.x * newy_oldx + old.y * newy_oldy + old.z * newy_oldz;
		middle.z = old.x * newz_oldx + old.y * newz_oldy + old.z * newz_oldz;
		rX = fabs(rX * newx_oldx + rY * newx_oldy);
		rY = fabs(rX * newy_oldx + rY * newy_oldy);       points_valid = false;
	}
	virtual bool isCollide(Vector& vect)
	{
		Vector focus1, focus2; float _cos, _sin;
		float focal_dist, rr;
		if (rX > rY) {
			focal_dist = sqrt((rX * rX) - (rY * rY)); //http://stackoverflow.com/questions/8187996/counting-points-inside-an-ellipse
			focus1 = Vector(middle.x - focal_dist, middle.y);
			focus2 = Vector(middle.x + focal_dist, middle.y); rr = rX + rX;
		}
		else {
			focal_dist = sqrt((rY * rY) - (rX * rX));
			focus1 = Vector(middle.x, middle.y - focal_dist);
			focus2 = Vector(middle.x, middle.y + focal_dist); rr = rY + rY;
		}
		_cos = cos(angle); _sin = sin(angle);
		focus1 = Vector(focus1.x * _cos + focus1.y * _sin, focus1.y * _cos - focus1.x * _sin) + center;
		focus2 = Vector(focus2.x * _cos + focus2.y * _sin, focus2.y * _cos - focus2.x * _sin) + center;

		if ((sqrt((vect.x - focus1.x) * (vect.x - focus1.x) + (vect.y - focus1.y) * (vect.y - focus1.y)) +
			sqrt((vect.x - focus2.x) * (vect.x - focus2.x) + (vect.y - focus2.y) * (vect.y - focus2.y))) <= rr) return true; return false;
	}
	virtual Vector* GetPoints() {
		if (points_valid) return points; int res = 10; float iangle = 0, _cos = cos(angle), _sin = sin(angle);
		if (points == 0) points = new Vector[res + 1];
		for (int i = 0; i < res + 1; i++) {
			iangle += (2 * PI) / ((float)res);
			points[i].x = center.x + (middle.x + cos(iangle) * rX) * _cos + (middle.y + sin(iangle) * rY) * _sin;
			points[i].y = center.y + (middle.y + sin(iangle) * rY) * _cos - (middle.x + cos(iangle) * rX) * _sin;
		}
		return points;
	}
	virtual int GetPointsLength() {
		return 11;
	}
	virtual void Draw()
	{
		float sector_angle = 0, _cos = cos(angle), _sin = sin(angle);
		glColor3f(color.r, color.g, color.b);
		glBegin(GL_TRIANGLE_FAN);
		glVertex2f(center.x + middle.x * _cos + middle.y * _sin, center.y + middle.y * _cos - middle.x * _sin);
		for (int i = 0; i <= resolution; i++, sector_angle += (2 * PI) / ((float)resolution))
			glVertex2f(center.x + (middle.x + cos(sector_angle) * rX) * _cos + (middle.y + sin(sector_angle) * rY) * _sin, center.y + (middle.y + sin(sector_angle) * rY) * _cos - (middle.x + cos(sector_angle) * rX) * _sin);
		glEnd();
	}
};
class CompositeShape : public Shape
{
	struct CShape {
		Shape* shape;
		CShape* next;
		CShape() { shape = 0; next = 0; }
		CShape(Shape* shape, CShape* next) { this->shape = shape; this->next = next; }
	};
	CShape firstitem;
public:
	CompositeShape(Shape* shape) :Shape(Color()), firstitem(shape, 0) {}
	virtual ~CompositeShape()
	{
		CShape* runner = firstitem.next;
		delete firstitem.shape;
		while (runner) { delete runner->shape; CShape* oldptr = runner; runner = runner->next; delete oldptr; }
	}
	void Add(Shape* shape)
	{
		CShape* runner = &firstitem;
		while (runner->next) runner = runner->next;
		CShape* last = new CShape(shape, 0);
		runner->next = last;
	}
	virtual void Move(Vector center)
	{
		CShape* runner = &firstitem;
		while (runner) { runner->shape->Move(center);  runner = runner->next; }
		this->center = center;
	}
	virtual void Rotate(float angle)
	{
		CShape* runner = &firstitem;
		while (runner) { runner->shape->Angle() = angle;  runner = runner->next; }
		this->angle = angle;
	}
	virtual void Transform(float newx_oldx, float newx_oldy, float newx_oldz, float newy_oldx, float newy_oldy, float newy_oldz, float newz_oldx, float newz_oldy, float newz_oldz)
	{
		CShape* runner = &firstitem;
		while (runner) { runner->shape->Transform(newx_oldx, newx_oldy, newx_oldz, newy_oldx, newy_oldy, newy_oldz, newz_oldx, newz_oldy, newz_oldz);  runner = runner->next; }
	}
	virtual bool isCollide(Vector& vect)
	{
		CShape* runner = &firstitem; bool result = false;
		while (runner && !result) { result = runner->shape->isCollide(vect);  runner = runner->next; }
		return result;
	}
	bool isCollide(CompositeShape* othershape)
	{
		CShape* runner = &firstitem; bool result = false;
		while (runner && !result)
		{
			Shape* localshape = runner->shape;
			runner = runner->next;
			if (localshape->GetPoints() == 0) continue;
			CShape* otherrunner = &(othershape->firstitem);
			while (otherrunner && !result)
			{
				Shape* othershape = otherrunner->shape;
				Vector* points = othershape->GetPoints();
				for (int i = 0; i < othershape->GetPointsLength() && !result; i++) result = localshape->isCollide(points[i]);
				otherrunner = otherrunner->next;
			}
		}
		return result;
	}
	void ReplaceColor(Color original, Color newcolor)
	{
		CShape* runner = &firstitem;
		while (runner) { if (runner->shape->_Color() == original) runner->shape->_Color() = newcolor;  runner = runner->next; }
	}
	virtual void Draw()
	{
		CShape* runner = &firstitem;
		while (runner) { runner->shape->Draw();  runner = runner->next; }
	}

};

CompositeShape* player, * enemy;
Shape* background;
Triangle rubber[4];
double enemyphase = 0;
Vector mousepos, grabdelta, catapultpos = Vector((200 / (float)screenWidth) * 2 - 1, ((screenHeight - 400) / (float)screenHeight) * 2 - 1, 0);
Vector playerA, playerV;
bool isMouseDown = false;

typedef enum _GameState { Idle, BirdGrabbed, BirdLaunching, BirdFlying, BirdHit } GameState;
GameState gamestate;

char image[screenWidth * screenHeight];	// egy alkalmazás ablaknyi kép

CompositeShape* CreateBird(Color main_color)
{
	CompositeShape* bird = new CompositeShape(new Triangle(Vector(0.05, -0.01), Vector(0.11, -0.00), Vector(0.10, 0.03), Black));
	bird->Add(new Triangle(Vector(0.04, -0.02), Vector(0.11, -0.01), Vector(0.11, -0.03), Black));
	bird->Add(new Triangle(Vector(0.03, -0.03), Vector(0.11, -0.04), Vector(0.10, -0.07), Black));
	bird->Add(new EllIpse(Vector(0, 0), 0.07, 0.10, main_color, 60));
	bird->Add(new Triangle(Vector(-0.025, -0.050), Vector(-0.018, 0.017), Vector(-0.098, 0.005), Yellow));
	bird->Add(new EllIpse(Vector(-0.04, 0.04), 0.013, 0.025, White, 60));
	bird->Add(new EllIpse(Vector(-0.01, 0.04), 0.013, 0.025, White, 60));
	bird->Add(new Circle(Vector(-0.04, 0.03), 0.008, Black, 60));
	bird->Add(new Circle(Vector(-0.01, 0.03), 0.008, Black, 60));
	bird->Add(new Triangle(Vector(-0.06, 0.06), Vector(-0.055, 0.077), Vector(-0.024, 0.05), Black));
	bird->Add(new Triangle(Vector(0.010, 0.06), Vector(0.005, 0.077), Vector(-0.026, 0.05), Black));
	return bird;
}
CompositeShape* CreateBackground()
{
	CompositeShape* back = new CompositeShape(new Triangle(Vector(-0.75, -1, 0), Vector(-0.55, -1, 0), Vector(-0.7, -0.78, 0), DarkBrown));
	back->Add(new Triangle(Vector(-0.55, -1, 0), Vector(-0.7, -0.78, 0), Vector(-0.6, -0.78, 0), LightBrown));
	back->Add(new Quad(Vector(-0.8, -0.63, 0), Vector(-0.6, -0.78, 0), Vector(-0.7, -0.78, 0), Vector(-0.8, -0.58, 0), Brown));
	back->Add(new Quad(Vector(-0.5, -0.63, 0), Vector(-0.67, -0.78, 0), Vector(-0.6, -0.78, 0), Vector(-0.5, -0.58, 0), LightBrown));
	back->Add(Quad::CreateRectangle(-2, -0.96, 4, 0.4, LightGreen));

	back->Transform(1, 0, 0, 0, 1.8, 0, 0, 0, 1);
	back->Move(Vector((100 / (float)screenWidth) * 2, 1.8 - 1));

	return back;
}
void CreateRubber()
{
	rubber[0].p[0] = Vector(-0.8, -0.6, 0); rubber[0].p[1] = Vector(-0.8, -0.62, 0); rubber[0].p[2] = catapultpos;
	rubber[1].p[0] = Vector(-0.5, -0.6, 0); rubber[1].p[1] = Vector(-0.5, -0.62, 0); rubber[1].p[2] = catapultpos;

	for (int i = 0; i < 2; i++) {
		rubber[i].p[0].x += (100 / (float)screenWidth) * 2;   rubber[i].p[0].y = rubber[i].p[0].y * 1.8 + 1.8 - 1;
		rubber[i].p[1].x += (100 / (float)screenWidth) * 2;   rubber[i].p[1].y = rubber[i].p[1].y * 1.8 + 1.8 - 1;
	}
}
void UpdateRubber()
{
	if (gamestate != BirdGrabbed && gamestate != BirdLaunching) {
		rubber[0].p[2] = catapultpos;         rubber[1].p[2] = catapultpos;
	}
	else {
		rubber[0].p[2] = player->Position();  rubber[1].p[2] = player->Position();
	}
}

bool flipped_player = false;
// Inicializacio, a program futasanak kezdeten, az OpenGL kontextus letrehozasa utan hivodik meg (ld. main() fv.)
void onInitialization() {
	Vector enemypos = Vector(0.84, 0);
	glViewport(0, 0, screenWidth, screenHeight);

	gamestate = Idle;

	if (player != 0) delete player; if (background != 0) delete background;
	if (enemy != 0) { enemypos = enemy->Position();   delete enemy; }

	enemy = CreateBird(Green);
	enemy->Move(enemypos);

	player = CreateBird(Red);
	player->Move(catapultpos);
	player->Transform(-1, 0, 0, 0, (flipped_player) ? (-1) : (1), 0, 0, 0, 1);  flipped_player = !flipped_player;

	background = CreateBackground();
	CreateRubber();
}

bool drawimg = false;
// Rajzolas, ha az alkalmazas ablak ervenytelenne valik, akkor ez a fuggveny hivodik meg
void onDisplay() {
	glClearColor(0.1f, 0.2f, 0.3f, 1.0f);		// torlesi szin beallitasa
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // kepernyo torles

	background->Draw();
	enemy->Draw();

	player->Draw();

	for (int i = 0; i < 2; i++) rubber[i].Draw();

	if (drawimg)
		glDrawPixels(screenWidth, screenHeight, GL_GREEN, GL_BYTE, image);

	glMatrixMode(GL_MODELVIEW);				// A MODELVIEW transzformaciot egysegmatrixra inicializaljuk
	glLoadIdentity();

	glutSwapBuffers();     				// Buffercsere: rajzolas vege

}
bool gravitybird = false;
char enemymoving = 0;
// Billentyuzet esemenyeket lekezelo fuggveny (lenyomas)
void onKeyboard(unsigned char key, int x, int y) {
	Vector v;
	if (key == 'a') {

		for (int Y = 400; Y < 600; Y++)
			for (int X = 0; X < 200; X++) {
				v.x = (((float)X) / (600 / 2)) - 1;
				v.y = (((600 - (float)Y)) / (600 / 2)) - 1;
				if (player->isCollide(v)) image[(600 - Y) * screenWidth + X] = 127; else image[(600 - Y) * screenWidth + X] = 0;
			}
		drawimg = !drawimg;
	}
	if (key == 's') { gravitybird = !gravitybird; }
	if (key == 'd') { enemymoving |= 1; }
	if (key == 'g') { enemymoving |= 2; }
	if (key == 'r') { enemymoving |= 4; }
	if (key == 'f') { enemymoving |= 8; }
	if (key == 'e') { enemymoving ^= 16; }

}

// Billentyuzet esemenyeket lekezelo fuggveny (felengedes)
void onKeyboardUp(unsigned char key, int x, int y) {
	if (key == 'd') { enemymoving &= ~1; }
	if (key == 'g') { enemymoving &= ~2; }
	if (key == 'r') { enemymoving &= ~4; }
	if (key == 'f') { enemymoving &= ~8; }
}

// Eger esemenyeket lekezelo fuggveny
void onMouse(int button, int state, int x, int y)
{
	mousepos.x = (((float)x) / (screenWidth / 2)) - 1;
	mousepos.y = (((screenHeight - (float)y)) / (screenHeight / 2)) - 1;

	if (button == GLUT_LEFT && state == GLUT_DOWN) {

		if (!isMouseDown && gamestate == Idle && player->isCollide(mousepos)) {
			gamestate = BirdGrabbed;
			grabdelta = player->Position() - mousepos;
		}
		isMouseDown = true;
	}
	if (button == GLUT_LEFT && state == GLUT_UP) {
		isMouseDown = false;
		if (gamestate == BirdHit) {
			onInitialization();
			gamestate = Idle;
		}
		if (gamestate == BirdGrabbed) {
			playerA = Vector(0, 0); playerV = Vector(0, 0);
			gamestate = BirdLaunching;
		}
		else if (gamestate != BirdFlying) gamestate = Idle;
	}

}

// Eger mozgast lekezelo fuggveny
void onMouseMotion(int x, int y)
{
	mousepos.x = (((float)x) / (600 / 2)) - 1;
	mousepos.y = (((600 - (float)y)) / (600 / 2)) - 1;

	if (isMouseDown && gamestate == BirdGrabbed) {
		Vector pos = mousepos + grabdelta;
		if (pos.x > catapultpos.x - 0.001f) pos.x = catapultpos.x - 0.001f;
		Vector delta = catapultpos - player->Position();
		float angle = atan2(delta.y, delta.x);
		player->Move(pos);
		player->Rotate(-1 * atan2(delta.y, delta.x));
		UpdateRubber();
	}
}

long oldtime = 0;
// `Idle' esemenykezelo, jelzi, hogy az ido telik, az Idle esemenyek frekvenciajara csak a 0 a garantalt minimalis ertek
void onIdle() {
	long time = glutGet(GLUT_ELAPSED_TIME);		// program inditasa ota eltelt ido
	for (; oldtime < time; oldtime++)
	{
		if (enemymoving & 1)   enemy->Move(Vector(enemy->Position().x - 0.001, enemy->Position().y));
		if (enemymoving & 2)   enemy->Move(Vector(enemy->Position().x + 0.001, enemy->Position().y));
		if (enemymoving & (4)) enemy->Move(Vector(enemy->Position().x, enemy->Position().y + 0.001));
		if (enemymoving & (8)) enemy->Move(Vector(enemy->Position().x, enemy->Position().y - 0.001));

		if (gamestate != BirdHit && !(enemymoving & 16)) {
			enemyphase = sin((2 * PI / 5000) * (time % 5000));
			enemy->Move(Vector(enemy->Position().x, (enemyphase) / 1.1, 0));
		}

		if (gamestate != BirdLaunching && gamestate != BirdFlying) continue;

		if (gamestate == BirdLaunching) {
			Vector delta = catapultpos - player->Position();
			float angle = atan2(delta.y, delta.x); //atan2: http://stackoverflow.com/questions/13458992/angle-between-two-vectors-2d
			if (delta.x <= 0.001f) delta.x = 0.0048f;

			playerV.x += delta.x * spring_const;
			playerV.y += delta.y * spring_const;

			if (player->Position().x >= catapultpos.x + 0.01) gamestate = BirdFlying;
		}
		if (gamestate == BirdFlying) {
			if (!gravitybird) playerV.y -= gravity_const;

			if (gravitybird) {
				Vector delta = enemy->Position() - player->Position();
				float distance = sqrt((delta.x * delta.x) + (delta.y * delta.y));
				float angle = atan2(delta.y, delta.x);
				playerV.x += (1 / (distance * distance)) / 100000 * cos(angle);
				playerV.y += (1 / (distance * distance)) / 100000 * sin(angle);
			}
		}

		player->Move(player->Position() + playerV);
		player->Rotate(-1 * atan2(playerV.y, playerV.x));

		if ((!gravitybird && (player->Position().x >= 1.8 || player->Position().y < -1.4)) || player->Position().y < -3) onInitialization();

		UpdateRubber();

		if (player->isCollide(enemy))
		{
			gamestate = BirdHit;
			player->ReplaceColor(Red, GoldenYellow);
		}
	}


	glutPostRedisplay();
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// A C++ program belepesi pontja, a main fuggvenyt mar nem szabad bantani
int main(int argc, char** argv) {
	glutInit(&argc, argv); 				// GLUT inicializalasa
	glutInitWindowSize(600, 600);			// Alkalmazas ablak kezdeti merete 600x600 pixel 
	glutInitWindowPosition(100, 100);			// Az elozo alkalmazas ablakhoz kepest hol tunik fel
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);	// 8 bites R,G,B,A + dupla buffer + melyseg buffer

	glutCreateWindow("Grafika hazi feladat");		// Alkalmazas ablak megszuletik es megjelenik a kepernyon

	glMatrixMode(GL_MODELVIEW);				// A MODELVIEW transzformaciot egysegmatrixra inicializaljuk
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);			// A PROJECTION transzformaciot egysegmatrixra inicializaljuk
	glLoadIdentity();

	onInitialization();					// Az altalad irt inicializalast lefuttatjuk

	glutDisplayFunc(onDisplay);				// Esemenykezelok regisztralasa
	glutMouseFunc(onMouse);
	glutIdleFunc(onIdle);
	glutKeyboardFunc(onKeyboard);
	glutKeyboardUpFunc(onKeyboardUp);
	glutMotionFunc(onMouseMotion);

	glutMainLoop();					// Esemenykezelo hurok

	return 0;
}
