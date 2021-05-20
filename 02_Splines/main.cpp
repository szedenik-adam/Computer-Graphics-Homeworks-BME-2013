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
#define max(a,b)    (((a) > (b)) ? (a) : (b))
#define min(a,b)    (((a) < (b)) ? (a) : (b))

#define starttime 1
#define mousehold 2
#define weightmode mousehold

#define tension       -0.5f

#define Black        Color(0,0,0)
#define White        Color(1,1,1)
#define Blue		 Color(0,0,1)
#define Yellow       Color(1,1,0)

#define Red          Color(0.9,0,0)
#define Cyan		 Color(0,0.9,0.9)

const int screenWidth = 600;
const int screenHeight = 600;
//--------------------------------------------------------
// 3D Vektor
//--------------------------------------------------------
struct Vector {
	float x, y;

	Vector() {
		x = y = 0;
	}
	Vector(float x0, float y0) {
		x = x0; y = y0;
	}
	Vector operator*(float a) {
		return Vector(x * a, y * a);
	}
	Vector operator+(const Vector& v) {
		return Vector(x + v.x, y + v.y);
	}
	Vector operator-(const Vector& v) {
		return Vector(x - v.x, y - v.y);
	}
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
};
struct PointW
{
	float x, y, w;
	PointW(float x, float y, float weight) :x(x), y(y), w(weight) {}
	PointW(float x, float y) :x(x), y(y), w(1) {}
	PointW() :x(0), y(0), w(1) {}
};

class Circle
{
public:
	static void Draw(Vector pos, float radius, Color color)
	{
		float sector_angle = 0;
		glColor3f(color.r, color.g, color.b);
		glBegin(GL_TRIANGLE_FAN);
		glVertex2f(pos.x, pos.y);
		for (int i = 0; i <= 60; i++, sector_angle += (2 * PI) / ((float)60))
			glVertex2f((pos.x + cos(sector_angle) * radius), (pos.y + sin(sector_angle) * radius));
		glEnd();
	}
};

class Spline
{
	Color color;
	PointW  cpoints[1000];   int cp_count;
	Vector middle; int middle_count;
	float sumW; bool sumW_valid;
	void (*spline_logic)(Spline*);
public:
	Spline(Color color) :color(color), cp_count(0), spline_logic(0), middle_count(-1), sumW_valid(false) {}
	Spline() :color(1, 1, 1), cp_count(0), spline_logic(0), middle_count(-1), sumW_valid(false) {}

	void Clear() { cp_count = 0; }
	int CPCount() { return cp_count; }

	Vector Middle()
	{
		if (middle_count == cp_count) return middle;

		Vector m = Vector(0, 0);
		for (int i = 0; i < cp_count; i++) { m.x += cpoints[i].x; m.y += cpoints[i].y; }
		m.x /= cp_count; m.y /= cp_count;

		middle = m; middle_count = cp_count;
		return m;
	}
	float SumWeight()
	{
		if (sumW_valid) return sumW;
		float sum = 0;
		for (int i = 0; i < cp_count - 1; i++) sum += cpoints[i].w;
		sumW = sum; sumW_valid = true;
		return sum;
	}

	void DrawCPoints(Color color)
	{
		for (int i = 0; i < cp_count; i++) Circle::Draw(Vector(cpoints[i].x, cpoints[i].y), 1, color);
	}
	void DrawCPoints(Color color, Color negcolor, float* rads, float multiplier, int first, int n)
	{
		for (int i = first; i < cp_count && i < (n + first); i++)
			if (rads[i - first] == 0) continue;
			else if (rads[i - first] > 0) Circle::Draw(Vector(cpoints[i].x, cpoints[i].y), rads[i - first] * multiplier, color);
			else                      Circle::Draw(Vector(cpoints[i].x, cpoints[i].y), rads[i - first] * multiplier * -1, negcolor);
	}

	void Draw()
	{
		if (spline_logic != 0) { glColor3f(color.r, color.g, color.b); spline_logic(this); }
	}

	Color& LineColor() { return color; }

	void SetSplineType(void (*spline_function)(Spline*)) { spline_logic = spline_function; }

	void AddCPoint(PointW pos)
	{
		if (pos.w == 0) pos.w = 0.01f;
		cpoints[cp_count] = pos;
		cp_count++;
		sumW_valid = false;
	}
	void SetAlmostLastCPointWeight(float w) { if (cp_count >= 2) { cpoints[cp_count - 2].w = w; sumW_valid = false; } }
	void SetLastCPointWeight(float w) { if (cp_count >= 1) { cpoints[cp_count - 1].w = w; sumW_valid = false; } }

	friend class SplineFunctions;
};

class SplineFunctions
{
public:
	static int ChooseN;
	static unsigned int ChooseArray[];

	static void BezCalculateChoose(int n)
	{
		ChooseN = n; int end = n / 2;
		ChooseArray[0] = 1;
		ChooseArray[n] = 1;
		for (int i = 0; i < end; i++) {
			unsigned long top = 1, bottom = 1; for (char j = 1, t = n; j <= i + 1; j++, t--) { bottom *= j; top *= t; if (top > (1 << 24)) { top /= bottom; bottom = 1; } }
			unsigned int result = top / bottom;
			ChooseArray[i + 1] = result;
			ChooseArray[n - (i + 1)] = result;
		}
	}

	static void Bezier(Spline* spline)
	{
		int n = min(spline->cp_count - 1, 30);
		if (ChooseN != n) { BezCalculateChoose(n); }

		glBegin(GL_LINE_STRIP);

		for (float t = 0; t <= 1.f; t += 0.01f)
		{
			Vector r(0, 0);
			for (int i = 0; i <= n; i++) {
				r.x += spline->cpoints[i].x * ChooseArray[i] * pow(t, i) * pow(1 - t, n - i);
				r.y += spline->cpoints[i].y * ChooseArray[i] * pow(t, i) * pow(1 - t, n - i);
			}
			glVertex2f(r.x, r.y);
		}
		glEnd();
	}
	static float BezierVals[];

	static Vector Bezier(Spline* spline, float t)
	{
		int n = min(spline->cp_count - 1, 30);
		if (ChooseN != n) { BezCalculateChoose(n); }

		Vector r(0, 0);
		for (int i = 0; i <= n; i++) {
			float B = ChooseArray[i] * pow(t, i) * pow(1 - t, n - i);
			BezierVals[i] = B;
			r.x += spline->cpoints[i].x * B;
			r.y += spline->cpoints[i].y * B;
		}
		return Vector(r.x, r.y);
	}

	static inline Vector TCatmullRomV(PointW& cp1, PointW& cp2, PointW& cp0)
	{
		Vector v = Vector(0, 0);
		v.x = (((cp2.x - cp1.x) / (cp1.w)) + ((cp1.x - cp0.x) / (cp0.w))) * (1 - tension) * 0.5f;
		v.y = (((cp2.y - cp1.y) / (cp1.w)) + ((cp1.y - cp0.y) / (cp0.w))) * (1 - tension) * 0.5f;
		return v;
	}

	static inline Vector TCatmullRomR(float t, PointW& cp1, PointW& cp2, Vector& v, Vector& v2)
	{
		Vector r;      float w = cp1.w;
		float t2 = t * t;  float w2 = w * w;
		float t3 = t2 * t; float w3 = w2 * w;
		r.x = cp1.x + v.x * t + t2 * (3 * (cp2.x - cp1.x) / w2 - ((v2.x + 2 * v.x) / w))
			+ t3 * (2 * (cp1.x - cp2.x) / w3 + ((v2.x + v.x) / w2));
		r.y = cp1.y + v.y * t + t2 * (3 * (cp2.y - cp1.y) / w2 - ((v2.y + 2 * v.y) / w))
			+ t3 * (2 * (cp1.y - cp2.y) / w3 + ((v2.y + v.y) / w2));
		return r;
	}

	static void TCatmullRom(Spline* spline)
	{
		Vector v, v2, r;
		glBegin(GL_LINE_STRIP);

		for (int i = 0; i < spline->cp_count - 1; i++)
		{
			PointW& cp1 = spline->cpoints[i];
			PointW& cp2 = spline->cpoints[i + 1];
			PointW& cp0 = spline->cpoints[i - 1];
			PointW& cp3 = spline->cpoints[i + 2];

			if (i == 0) { v = Vector(0, 0); }
			else { v = TCatmullRomV(cp1, cp2, cp0); }
			if (i == spline->cp_count - 2) { v2 = Vector(0, 0); }
			else { v2 = TCatmullRomV(cp2, cp3, cp1); }

			for (float f = 0; f < cp1.w; f += 0.05f) {
				r = TCatmullRomR(f, cp1, cp2, v, v2);
				glVertex2f(r.x, r.y);
			}
			r = TCatmullRomR(cp1.w, cp1, cp2, v, v2);
			glVertex2f(r.x, r.y);
		}
		glEnd();
	}

	static int TCRFirstWeightedPoint;
	static float TCRVals[];

	static Vector TCatmullRomVect(Spline* spline, float t)
	{
		Vector v, v2, r; float w_total = spline->SumWeight();

		t *= w_total;

		for (int i = 0; i < spline->cp_count - 1; i++)
		{
			PointW& cp1 = spline->cpoints[i];
			PointW& cp2 = spline->cpoints[i + 1];
			PointW& cp0 = spline->cpoints[i - 1];
			PointW& cp3 = spline->cpoints[i + 2];

			if (t > cp1.w) { t -= cp1.w; continue; }

			if (i == 0) { v = Vector(0, 0); }
			else { v = TCatmullRomV(cp1, cp2, cp0); }
			if (i == spline->cp_count - 2) { v2 = Vector(0, 0); }
			else { v2 = TCatmullRomV(cp2, cp3, cp1); }

			r = TCatmullRomR(t, cp1, cp2, v, v2);

			PointW cp[4], cporig[] = { cp0,cp1,cp2,cp3 };
			if (i == 0) cporig[0] = PointW(0, 0, 1);
			if (i == spline->cp_count - 2) cporig[3] = PointW(0, 0, 1);

			TCRFirstWeightedPoint = i - 1;
			for (int j = 0; j < 4; j++) {
				cporig[j].x -= r.x; cporig[j].y -= r.y;
				cp[j].w = cporig[j].w; cp[j].x = 0; cp[j].y = 0;
			}
			Vector parts[4]; Vector sum = Vector(0, 0);
			for (int j = 0; j < 4; j++)
			{
				cp[j] = cporig[j];
				if (i > 0)                  v = TCatmullRomV(cp[1], cp[2], cp[0]); else v = Vector(0, 0);
				if (i < spline->cp_count - 2) v2 = TCatmullRomV(cp[2], cp[3], cp[1]); else v2 = Vector(0, 0);
				parts[j] = TCatmullRomR(t, cp[1], cp[2], v, v2);
				sum = sum + parts[j];
				cp[j].x = 0; cp[j].y = 0;
				TCRVals[j] = parts[j].x / cporig[j].x;
				float yval = parts[j].y / cporig[j].y;
				if (cporig[j].x == 0 || (parts[j].x < parts[j].y && cporig[j].y != 0)) TCRVals[j] = yval;
			}
			return Vector(r.x, r.y);
		}
		return Vector(0, 0);
	}
};
int SplineFunctions::ChooseN = 0;
unsigned int SplineFunctions::ChooseArray[31];
float SplineFunctions::BezierVals[31];
int SplineFunctions::TCRFirstWeightedPoint;
float SplineFunctions::TCRVals[4];

class View
{
	static Vector camerabottompos;
	static float zoom;
public:
	static const float Distance;

	static const float& Zoom() { return zoom; }
	static const Vector& Camera() { return View::camerabottompos; }

	static void Camera(Vector camerabottompos)
	{
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		View::camerabottompos = camerabottompos;
		gluOrtho2D(camerabottompos.x, camerabottompos.x + (2 * Distance) / zoom, camerabottompos.y, camerabottompos.y + (2 * Distance) / zoom);
	}
	static inline void CameraMiddle(Vector middle)
	{
		Camera(middle + (Vector(-1, -1) * (Distance / zoom)));
	}

	static void ChangeZoom(float delta)
	{
		Vector middle = camerabottompos + (Vector(1, 1) * (Distance / zoom));
		zoom *= delta;
		Vector bottom = middle + (Vector(-1, -1) * (Distance / zoom));
		Camera(bottom);
	}

	static inline void Init()
	{
		glViewport(0, 0, screenWidth, screenHeight);

		Camera(Vector(0, 0));
	}
	static void Clear()
	{
		glClearColor(0.05f, 0.1f, 0.15f, 0.0f);		// torlesi szin beallitasa
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // kepernyo torles

		View::ResetTransform();
	}
	static Vector TranslateScreenCoordinates(int x, int y)
	{
		Vector result;
		result.x = (((float)x) / (screenWidth)) * Distance * 2 / zoom + camerabottompos.x;
		result.y = ((screenHeight - (float)y) / screenHeight) * Distance * 2 / zoom + camerabottompos.y;
		return result;
	}
	static void RotationTransform(float rotation, Vector middle)
	{
		glTranslatef(middle.x, middle.y, 0);
		glRotatef(rotation, 0, 0, 1);
		glTranslatef(-1 * middle.x, -1 * middle.y, 0);
	}
	static Vector RotationTransform(float rotation, Vector middle, Vector point)
	{
		rotation *= PI / 180;
		float _sin = sin(rotation); float _cos = cos(rotation);
		point = point - middle;
		Vector result;
		result.x = point.x * _cos - point.y * _sin;
		result.y = point.x * _sin + point.y * _cos;
		result = result + middle;
		return result;
	}
	static inline void ResetTransform()
	{
		glMatrixMode(GL_MODELVIEW);				// A MODELVIEW transzformaciot egysegmatrixra inicializaljuk
		glLoadIdentity();
	}

};
const float View::Distance = 25.0f;
Vector View::camerabottompos;
float View::zoom = 1.0f;

Spline splines[2];

Vector mousepos, mousedownpos;
bool isAnimating = false; float animationPhase = 0; long lastAnimation = 0;
long mousedowntime = 0, time = 0; bool isMouseDown = false; char tracking = 0;

// Inicializacio, a program futasanak kezdeten, az OpenGL kontextus letrehozasa utan hivodik meg (ld. main() fv.)
void onInitialization() {
	View::Init();

	splines[0].LineColor() = White;
	splines[0].SetSplineType(&SplineFunctions::TCatmullRom);
	splines[1].LineColor() = Blue;
	splines[1].SetSplineType(&SplineFunctions::Bezier);
}
Vector bPos, cPos;
// Rajzolas, ha az alkalmazas ablak ervenytelenne valik, akkor ez a fuggveny hivodik meg
void onDisplay()
{
	View::Clear();

	if (!isAnimating) { splines[0].DrawCPoints(Red); splines[0].Draw(); }

	if (isAnimating)
	{
		bPos = SplineFunctions::Bezier(splines + 1, animationPhase);
		cPos = SplineFunctions::TCatmullRomVect(splines + 0, animationPhase);

		splines[0].DrawCPoints(Red, Cyan, SplineFunctions::TCRVals, 2, SplineFunctions::TCRFirstWeightedPoint, 4);

		View::RotationTransform(60, splines[1].Middle());
		splines[1].DrawCPoints(Red, Cyan, SplineFunctions::BezierVals, 2, 0, min(splines[1].CPCount(), 31));
		splines[1].Draw();

		View::ResetTransform();
		splines[0].Draw();
		Circle::Draw(cPos, 1, Yellow);

		//View::RotationTransform(60,splines[1].Middle()); //így is lehetne de ezzel nem jó a pont követõ
		bPos = View::RotationTransform(60, splines[1].Middle(), bPos);
		Circle::Draw(bPos, 1, Yellow);

		if (tracking == 1) View::CameraMiddle(bPos); else if (tracking == 2) View::CameraMiddle(cPos);
	}

	glutSwapBuffers();     				// Buffercsere: rajzolas vege
}
char navigation = 0; int navtime = 0;
// Billentyuzet esemenyeket lekezelo fuggveny (lenyomas)
void onKeyboard(unsigned char key, int x, int y)
{
	if (key == 's') {
		View::Camera(Vector(10.f * View::Zoom(), 20.f * View::Zoom()) + View::Camera());
		if (View::Camera().x > 100.0f - ((View::Distance * 2) / View::Zoom()) || View::Camera().y > 100.0f - ((View::Distance * 2) / View::Zoom())) View::Camera(Vector(0, 0));
		glutPostRedisplay();
	}
	if (key == ' ') {
		isAnimating = true; lastAnimation = time;
	}
	if (key == 'c') { for (int i = 0; i < (sizeof(splines) / sizeof(Spline)); i++) splines[i].Clear(); isAnimating = false; animationPhase = 0; glutPostRedisplay(); }
	if (key == 'r') { navigation |= 1; navtime = time; }
	if (key == 'f') { navigation |= 2; navtime = time; }
	if (key == 'd') { navigation |= 4; navtime = time; }
	if (key == 'g') { navigation |= 8; navtime = time; }
	if (key == 'z') { View::ChangeZoom(1.1111f); glutPostRedisplay(); }
	if (key == 'h') { View::ChangeZoom(0.9f);    glutPostRedisplay(); }
	if (key == 't') { tracking++; if (tracking > 2) tracking = 0; }
}

// Billentyuzet esemenyeket lekezelo fuggveny (felengedes)
void onKeyboardUp(unsigned char key, int x, int y)
{
	if (key == 'r') { navigation &= ~1; }
	if (key == 'f') { navigation &= ~2; }
	if (key == 'd') { navigation &= ~4; }
	if (key == 'g') { navigation &= ~8; }
}

// Eger esemenyeket lekezelo fuggveny
void onMouse(int button, int state, int x, int y)
{
	if (isAnimating) { return; }

	mousepos = View::TranslateScreenCoordinates(x, y);

#if weightmode == mousehold
	if (button == GLUT_LEFT && state == GLUT_DOWN) {

		mousedowntime = time;
		isMouseDown = true;
		splines[0].AddCPoint(PointW(mousepos.x, mousepos.y, (1) / 20.f));
		if (splines[1].CPCount() < 31) splines[1].AddCPoint(PointW(mousepos.x, mousepos.y, (1) / 20.f));

	}
	if (button == GLUT_LEFT && state == GLUT_UP) {
		isMouseDown = false;
		splines[0].SetAlmostLastCPointWeight((time - mousedowntime) / 20.f);
		splines[1].SetAlmostLastCPointWeight((time - mousedowntime) / 20.f);
	}
#endif

#if weightmode == starttime
	if (button == GLUT_LEFT && state == GLUT_DOWN) {

		splines[0].AddCPoint(PointW(mousepos.x, mousepos.y, (time - mousedowntime)));
		splines[1].AddCPoint(PointW(mousepos.x, mousepos.y, (time - mousedowntime)));
		splines[0].SetAlmostLastCPointWeight((time - mousedowntime));
		splines[1].SetAlmostLastCPointWeight((time - mousedowntime));
		mousedowntime = time;
	}
#endif

	glutPostRedisplay();
}

// Eger mozgast lekezelo fuggveny
void onMouseMotion(int x, int y)
{

}

// `Idle' esemenykezelo, jelzi, hogy az ido telik, az Idle esemenyek frekvenciajara csak a 0 a garantalt minimalis ertek
void onIdle() {
	time = glutGet(GLUT_ELAPSED_TIME);		// program inditasa ota eltelt ido

	if (isAnimating) {
#if weightmode == starttime
		animationPhase += (1 / splines[0].SumWeight() * (time - lastAnimation));
#endif
#if weightmode == mousehold
		animationPhase += (0.05 / splines[0].SumWeight() * (time - lastAnimation));
#endif
		//if(time&4) printf("%d\n",time-lastAnimation);
		lastAnimation = time;
		if (animationPhase > 1.0f) { animationPhase = 0; }
		glutPostRedisplay();
	}
#if weightmode == mousehold
	if (isMouseDown) {
		splines[0].SetAlmostLastCPointWeight((time - mousedowntime) / 20.f);
		splines[1].SetAlmostLastCPointWeight((time - mousedowntime) / 20.f);
		glutPostRedisplay();
	}
#endif
	if (navigation) {
		int delta = time - navtime;
		if (navigation & 1) View::Camera(Vector(0, 0.02f * delta) + View::Camera());
		if (navigation & 2) View::Camera(Vector(0, -0.02f * delta) + View::Camera());
		if (navigation & 4) View::Camera(Vector(-0.02f * delta, 0) + View::Camera());
		if (navigation & 8) View::Camera(Vector(0.02f * delta, 0) + View::Camera());
		navtime = time;
		glutPostRedisplay();
	}
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
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
