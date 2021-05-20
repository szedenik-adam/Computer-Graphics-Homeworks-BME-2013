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

#define tension       -0.5f

#define salami    1
#define nonsalami 2
#define golya_mode salami

const int screenWidth = 600;
const int screenHeight = 600;

typedef unsigned char UINT8;
typedef int INT32;
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
	Vector operator/(float a) {
		return Vector(x / a, y / a, z / a);
	}
	Vector operator+(const Vector& v) {
		return Vector(x + v.x, y + v.y, z + v.z);
	}
	Vector operator+=(const Vector& v) {
		*this = *this + v;
		return *this;
	}
	Vector operator-=(const Vector& v) {
		*this = *this - v;
		return *this;
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
	bool operator==(const Vector& v) {
		return (x == v.x && y == v.y && z == v.z);
	}
	float Length() { return sqrt(x * x + y * y + z * z); }

	float Distance(const Vector& v) {
		Vector delta = Vector(x - v.x, y - v.y, z - v.z);
		return sqrt(delta.x * delta.x + delta.y * delta.y + delta.z * delta.z);
	}
	Vector& normalize() {
		float len = this->Length();
		*this = (*this) * (1 / len);
		return *this;
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
	float x, y, z, w;
	PointW(float x, float y, float z, float weight) :x(x), y(y), z(z), w(weight) {}
	PointW(float x, float y, float z) :x(x), y(y), z(z), w(1) {}
	PointW() :x(0), y(0), z(0), w(1) {}
	operator Vector() const { return Vector(x, y, z); }
};

class MaterialBase {
public:
	virtual bool Setup() = 0;
};
class Material : public MaterialBase {
public:
	float kd[4], ks[4], ka[4];
	Material() { ka[3] = 1; kd[3] = 1; ks[3] = 1; }
	Material(Color a, Color d, Color s) {
		ka[0] = a.r; ka[1] = a.g; ka[2] = a.b; ka[3] = 1;
		kd[0] = d.r; kd[1] = d.g; kd[2] = d.b; kd[3] = 1;
		ks[0] = s.r; ks[1] = s.g; ks[2] = s.b; ks[3] = 1;
	}

	virtual bool Setup() {
		glDisable(GL_TEXTURE_2D);
		glMaterialfv(GL_FRONT, GL_AMBIENT, ka);
		glMaterialfv(GL_FRONT, GL_DIFFUSE, kd);
		glMaterialfv(GL_FRONT, GL_SPECULAR, ks);
		glMaterialf(GL_FRONT, GL_SHININESS, 5.0f);
		return false;
	}
};
class Texture : public MaterialBase {
public:
	GLuint texture;
	Material colors; bool colors_enabled;
	Texture() :texture(0), colors_enabled(false) {}
	Texture(GLuint texture) :texture(texture), colors_enabled(false) {}
	Texture(GLuint texture, Color a, Color d, Color s) :texture(texture), colors(a, d, s), colors_enabled(true) {}

	virtual bool Setup() {
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, texture);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		if (colors_enabled) {
			glMaterialfv(GL_FRONT, GL_AMBIENT, colors.ka);
			glMaterialfv(GL_FRONT, GL_DIFFUSE, colors.kd);
			glMaterialfv(GL_FRONT, GL_SPECULAR, colors.ks);
			glMaterialf(GL_FRONT, GL_SHININESS, 5.0f);
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		}
		else { glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE); }
		return true;
	}
};

class View
{
	static Vector light_pos[];
	static bool light_isDirectional[];
	static int lightN;
public:
	static Vector eye, lookat, up;
	static float zoom;
	static GLuint textures[];

	static inline void Init()
	{
		glViewport(0, 0, screenWidth, screenHeight);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(54, screenWidth / (float)screenHeight, 1, 100);
	}
	static void Clear()
	{
		glClearColor(0.01f, 0.05f, 0.35f, 0.0f);		// torlesi szin beallitasa
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // kepernyo torles

		View::ResetTransform();
	}
	static void Move(Vector delta) {
		eye += delta;
		lookat += delta;
	}

	static inline void ResetTransform()
	{
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		gluLookAt(View::eye.x, View::eye.y, View::eye.z, View::lookat.x, View::lookat.y, View::lookat.z, View::up.x, View::up.y, View::up.z); //VIEW
	}

	static void AddLight(Vector pos, bool isDirectional, Color IDiffuse, Color ISpecular, Color IAmbient = Color(0, 0, 0))
	{
		if (lightN == 8) lightN--;
		light_pos[lightN] = pos;
		light_isDirectional[lightN] = isDirectional;
		float array_[4] = { pos.x, pos.y, pos.z, (isDirectional) ? (0) : (1) };
		glLightfv(GL_LIGHT0 + lightN, GL_POSITION, array_);

		array_[0] = IAmbient.r; array_[1] = IAmbient.g; array_[2] = IAmbient.b; array_[3] = 1;
		glLightfv(GL_LIGHT0 + lightN, GL_AMBIENT, array_);

		array_[0] = IDiffuse.r; array_[1] = IDiffuse.g; array_[2] = IDiffuse.b;
		glLightfv(GL_LIGHT0 + lightN, GL_DIFFUSE, array_);

		array_[0] = ISpecular.r; array_[1] = ISpecular.g; array_[2] = ISpecular.b;
		glLightfv(GL_LIGHT0 + lightN, GL_SPECULAR, array_);

		glEnable(GL_LIGHT0 + lightN); lightN++;
	}
	static Texture grass;
	static Texture grass_frog;
	static Texture golya_;

	static void TextureInit()
	{
		glEnable(GL_TEXTURE_2D);			       // texturing is on
		glGenTextures(1, textures + 0);  		   // id generation
		glBindTexture(GL_TEXTURE_2D, textures[0]); // binding
		//static INT32 bitmap[256*256];            //GL_RGBA :8+8+8+8 
		INT32* bitmap = (INT32*)malloc(sizeof(INT32) * 1024 * 1024);

		UINT8 color[4]; int n = 256 * 256;
		int random = rand();
		color[1] = random & 127; random >>= 8; color[2] = random & 7; random >>= 3; color[0] = random & 7;  color[3] = 255;
		for (int i = 0; i < 256 * 256; i++) { bitmap[i] = *((INT32*)color); }
		for (int i = 0; i < 12; i++) {
			random = rand();
			color[1] = random & 127; random >>= 8; color[2] = random & 7; random >>= 3; color[0] = random & 7;  color[3] = 255;

			n = (int)(n / 1.2);
			for (int j = 0; j < n; j++) {
				int offset = ((rand() & 255) << 8) | (rand() & 255);
				bitmap[offset] = *((INT32*)color);
			}
		}
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 64, 64, 0,
			GL_RGBA, GL_UNSIGNED_BYTE, bitmap);  //Texture->OpenGL
		grass = Texture(textures[0]);
		grass_frog = Texture(textures[0], Color(0.1, 0.1, 0.1), Color(0.3, 0.8, 0.2), Color(0.1, 0.3, 0.1));

		glGenTextures(1, textures + 1);
		glBindTexture(GL_TEXTURE_2D, textures[1]);

		UINT8 white[4], black[4];
		white[0] = 255; white[1] = 255; white[2] = 255; white[3] = 255;
		black[0] = 10; black[1] = 10; black[2] = 10; black[3] = 255;

		for (int i = 0; i < 1024 * 1024; i++) bitmap[i] = *((INT32*)white);
		for (int y = 0; y < 280; y++)   for (int x = 0; x < 160; x++) bitmap[x + 1024 * y] = *((INT32*)black);
		for (int y = 280; y < 1024; y++) for (int x = 0; x < 64; x++) bitmap[x + 1024 * y] = *((INT32*)black);
		for (int y = 240; y < 280; y++)  for (int x = 64; x < (64 + (160 - 64) * (((y - 120) / (280 - 120.0f)))); x++) bitmap[x + 1024 * y] = *((INT32*)white);
		for (int y = 120; y < 640; y++)  for (int x = 64; x < 64 + (440 - 64) * pow((y - 120) / (760 - 120.0f), 4); x++) bitmap[x + 1024 * y] = *((INT32*)black);
		int endx = 64 + (440 - 64) * pow((640 - 120) / (760 - 120.0f), 2);
		for (int y = 640; y < 800; y++) for (int x = 64; x < endx - 120 * (pow((y - 640) / 160.0f, 2)); x++) bitmap[x + 1024 * y] = *((INT32*)black);
		for (int i = 0; i < 1000; i++) {
			int x = rand() & 1023; int y = rand() & 1023;
			if (!x) x = 1; if (x > 1022) x = 1022;
			if (!y) y = 1; if (y > 1022) y = 1022;
			INT32 pixel = bitmap[x + 1024 * y];
			if (pixel & 255 > 100) pixel -= 40 + (40 << 8) + (40 << 16);
			bitmap[x + 1024 * y] = pixel; bitmap[x + 1 + 1024 * y] = pixel; bitmap[x + 1024 * (y + 1)] = pixel;
			bitmap[x - 1 + 1024 * y] = pixel; bitmap[x + 1024 * (y - 1)] = pixel;
		}


		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1024, 1024, 0,
			GL_RGBA, GL_UNSIGNED_BYTE, bitmap);
		golya_ = Texture(textures[1], Color(0.05, 0.05, 0.05), Color(0.8, 0.8, 0.8), Color(0.2, 0.2, 0.2));


		free(bitmap);
	}
};
GLuint View::textures[16];
Texture View::grass;
Texture View::grass_frog;
Texture View::golya_;
Vector View::eye = Vector(0, 2, 0);
Vector View::lookat = Vector(0, 2, 3);
Vector View::up = Vector(0, 1, 0);
Vector View::light_pos[8];
bool   View::light_isDirectional[8];
int View::lightN = 0;

Material frog_body = Material(Color(0.05, 0.2, 0.01), Color(0.1, 0.5, 0.1), Color(0.1, 0.5, 0.1));
Material frog_leg = Material(Color(0.05, 0.2, 0.01), Color(0.1, 0.65, 0.1), Color(0.00, 0.00, 0.0));
Material frog_eye = Material(Color(0.05, 0.2, 0.01), Color(0.3, 0.5, 0.3), Color(0.10, 0.10, 0.10));

Material golya_beak = Material(Color(0.1, 0.1, 0.1), Color(0.95292, 0.247, 0.247), Color(0.4, 0.1, 0.1));
Material golya_eye = Material(Color(0.1, 0.1, 0.1), Color(0, 0, 0), Color(0, 0, 0));

Material firebug_body = Material(Color(3, 2.8, 1), Color(0.4, 0.34, 0.125), Color(0.3, 0.3, 0.1)); //ambient: emitting yellow light

Material texture_default = Material(Color(0.1, 0.1, 0.1), Color(1, 1, 1), Color(0.5, 0.5, 0.5));

class Object
{
protected:
	Object* parts[8]; int part_count;
public:
	Vector pos, rotation, scale;

	Object() :part_count(0), pos(0, 0, 0), rotation(0, 0, 0), scale(1, 1, 1) {}
	Object(Vector pos, Vector rotation, Vector scale) :part_count(0), pos(pos), rotation(rotation), scale(scale) {}

	void Draw() {
		View::ResetTransform();
		RecursiveDraw();
	}
	void RecursiveDraw() {
		glPushMatrix();

		glTranslatef(pos.x, pos.y, pos.z);   //MODEL
		glRotatef(rotation.x, 1, 0, 0);
		glRotatef(rotation.y, 0, 1, 0); //ha egyszerre több forgatás van nemjó  (változik a tengely)
		glRotatef(rotation.z, 0, 0, 1);
		glScalef(scale.x, scale.y, scale.z);

		LocalDraw();

		for (int i = 0; i < part_count; i++) parts[i]->RecursiveDraw();

		glPopMatrix();
	}
	virtual void LocalDraw() = 0;

	void AddPart(Object* part) {
		if (part_count < 8) { parts[part_count] = part; part_count++; }
	}
};

class Ellipsoid : public Object
{
public:
	Color color;
	MaterialBase* material;
	int resolution;

	Ellipsoid() :Object(), color(Color(0.1, 1, 1)), material(0), resolution(30) {}
	Ellipsoid(Vector pos, Vector rotation, MaterialBase* mat, float x_radius = 1, float y_radius = 1, float z_radius = 1, int resolution = 30)
		:Object(pos, rotation, Vector(x_radius, y_radius, z_radius)), material(mat), color(Color(0.1, 1, 1)), resolution(resolution) {}

	virtual void LocalDraw()
	{
		if (!material) return;
		bool isTexture = material->Setup();
		Ellipsoid::DrawSphere(resolution, isTexture, true);
	}

	static void DrawSphere(int res = 30, bool AddTexCoord = false, bool AddNormals = true)
	{
		float y_angle = 0, old_y = 0;

		glBegin(GL_TRIANGLES);
		for (int i = 0; i < res / 2; i++)
		{
			float xz_angle = 0, y_offset = cos(y_angle), radius = sin(y_angle), old_xz = 0;
			old_y = y_angle;  y_angle += (2 * PI) / ((float)res); //stepping to next y_angle
			float next_y_offset = cos(y_angle), next_radius = sin(y_angle);

			for (int j = 0; j < res; j++)
			{
				Vector current_point = Vector(cos(xz_angle) * radius, y_offset, sin(xz_angle) * radius);
				Vector below_point = Vector(cos(xz_angle) * next_radius, next_y_offset, sin(xz_angle) * next_radius);

				old_xz = xz_angle;  xz_angle += (2 * PI) / ((float)res);

				Vector current_plus1 = Vector(cos(xz_angle) * radius, y_offset, sin(xz_angle) * radius);
				Vector below_plus1 = Vector(cos(xz_angle) * next_radius, next_y_offset, sin(xz_angle) * next_radius);
				if (AddTexCoord) {
					glTexCoord2f(old_y / PI, old_xz / (2 * PI));    glNormal3fv((GLfloat*)&current_point);   glVertex3f(current_point.x, current_point.y, current_point.z); //current point
					glTexCoord2f(old_y / PI, xz_angle / (2 * PI));  glNormal3fv((GLfloat*)&current_plus1);   glVertex3f(current_plus1.x, current_plus1.y, current_plus1.z); //current point+1
					glTexCoord2f(y_angle / PI, old_xz / (2 * PI));  glNormal3fv((GLfloat*)&below_point);     glVertex3f(below_point.x, below_point.y, below_point.z); //below point

					glTexCoord2f(old_y / PI, xz_angle / (2 * PI));    glNormal3fv((GLfloat*)&current_plus1);   glVertex3f(current_plus1.x, current_plus1.y, current_plus1.z); //current point+1
					glTexCoord2f(y_angle / PI, xz_angle / (2 * PI));  glNormal3fv((GLfloat*)&below_plus1);     glVertex3f(below_plus1.x, below_plus1.y, below_plus1.z); //below point+1
					glTexCoord2f(y_angle / PI, old_xz / (2 * PI));    glNormal3fv((GLfloat*)&below_point);     glVertex3f(below_point.x, below_point.y, below_point.z); //below point
				}
				else
					if (AddNormals) {
						glNormal3fv((GLfloat*)&current_point);   glVertex3f(current_point.x, current_point.y, current_point.z); //current point
						glNormal3fv((GLfloat*)&current_plus1);   glVertex3f(current_plus1.x, current_plus1.y, current_plus1.z); //current point+1
						glNormal3fv((GLfloat*)&below_point);     glVertex3f(below_point.x, below_point.y, below_point.z); //below point

						glNormal3fv((GLfloat*)&current_plus1);   glVertex3f(current_plus1.x, current_plus1.y, current_plus1.z); //current point+1
						glNormal3fv((GLfloat*)&below_plus1);     glVertex3f(below_plus1.x, below_plus1.y, below_plus1.z); //below point+1
						glNormal3fv((GLfloat*)&below_point);     glVertex3f(below_point.x, below_point.y, below_point.z); //below point
					}
					else {
						glVertex3f(current_point.x, current_point.y, current_point.z); //current point
						glVertex3f(current_plus1.x, current_plus1.y, current_plus1.z); //current point+1
						glVertex3f(below_point.x, below_point.y, below_point.z); //below point

						glVertex3f(current_plus1.x, current_plus1.y, current_plus1.z); //current point+1
						glVertex3f(below_plus1.x, below_plus1.y, below_plus1.z); //below point+1
						glVertex3f(below_point.x, below_point.y, below_point.z); //below point	
					}
			}
		}
		glEnd();
	}
};

class MultiCylinder
{
	static const int max_resolution;
	static Vector buffer[];
	static float sincosbuffer[];
public:
	static void Draw(Vector pbottom, Vector ptop, float subcylinder_count, float* subcylinder_rads, int resolution = 10)
	{
		Vector* old_point = buffer, * current_point = buffer + resolution, * next_point = buffer + 4 * resolution;
		Vector* old_normal = buffer + 2 * resolution, * current_normal = buffer + 3 * resolution;
		float* _sin = sincosbuffer; float* _cos = sincosbuffer + resolution;
		float angle = 0;
		Vector dir = (ptop - pbottom).normalize();
		Vector d1(dir.z * (-1), dir.z * (-1), dir.x + dir.y); d1.normalize();
		Vector d2(dir % d1);							  d2.normalize();
		dir = (ptop - pbottom) / (subcylinder_count - 1);

		for (int i = 0; i < resolution; i++, angle += 2 * PI / resolution) {
			_sin[i] = sin(angle); _cos[i] = cos(angle);
			old_normal[i] = (d1 * _cos[i] + d2 * _sin[i]) * subcylinder_rads[0];
			old_point[i] = pbottom + old_normal[i];
			current_point[i] = pbottom + dir + (d1 * _cos[i] + d2 * _sin[i]) * subcylinder_rads[1];
		}
		glBegin(GL_TRIANGLES);
		for (int i = 1; i < subcylinder_count; i++) {
			float rad = *(++subcylinder_rads);
			pbottom += dir;

			if (i == subcylinder_count - 1) { current_normal[0] = (d1 * _cos[0] + d2 * _sin[0]) * rad; current_point[0] = pbottom + current_normal[0]; }
			else {
				next_point[0] = pbottom + dir + (d1 * _cos[0] + d2 * _sin[0]) * (*(subcylinder_rads + 1));
				current_normal[0] = ((next_point[0] - old_point[0]) % (current_point[0] - pbottom)) % (next_point[0] - old_point[0]);
			}

			for (int j = 1; j < resolution; j++) {
				if (i == subcylinder_count - 1) { current_normal[j] = (d1 * _cos[j] + d2 * _sin[j]) * rad; current_point[j] = pbottom + current_normal[j]; }
				else {
					next_point[j] = pbottom + dir + (d1 * _cos[j] + d2 * _sin[j]) * (*(subcylinder_rads + 1));
					current_normal[j] = ((next_point[j] - old_point[j]) % (current_point[j] - pbottom)) % (next_point[j] - old_point[j]);
				}

				glNormal3fv((GLfloat*)(current_normal + j - 1));  glVertex3f(current_point[j - 1].x, current_point[j - 1].y, current_point[j - 1].z);
				glNormal3fv((GLfloat*)(current_normal + j));  glVertex3f(current_point[j].x, current_point[j].y, current_point[j].z);
				glNormal3fv((GLfloat*)(old_normal + j - 1));  glVertex3f(old_point[j - 1].x, old_point[j - 1].y, old_point[j - 1].z);

				glNormal3fv((GLfloat*)(current_normal + j));  glVertex3f(current_point[j].x, current_point[j].y, current_point[j].z);
				glNormal3fv((GLfloat*)(old_normal + j));  glVertex3f(old_point[j].x, old_point[j].y, old_point[j].z);
				glNormal3fv((GLfloat*)(old_normal + j - 1));  glVertex3f(old_point[j - 1].x, old_point[j - 1].y, old_point[j - 1].z);
			}
			glNormal3fv((GLfloat*)(current_normal + resolution - 1));  glVertex3f(current_point[resolution - 1].x, current_point[resolution - 1].y, current_point[resolution - 1].z);
			glNormal3fv((GLfloat*)(current_normal + 0));           glVertex3f(current_point[0].x, current_point[0].y, current_point[0].z);
			glNormal3fv((GLfloat*)(old_normal + resolution - 1));  glVertex3f(old_point[resolution - 1].x, old_point[resolution - 1].y, old_point[resolution - 1].z);

			glNormal3fv((GLfloat*)(current_normal + 0));           glVertex3f(current_point[0].x, current_point[0].y, current_point[0].z);
			glNormal3fv((GLfloat*)(old_normal + 0));           glVertex3f(old_point[0].x, old_point[0].y, old_point[0].z);
			glNormal3fv((GLfloat*)(old_normal + resolution - 1));  glVertex3f(old_point[resolution - 1].x, old_point[resolution - 1].y, old_point[resolution - 1].z);

			Vector* tmp = current_normal; current_normal = old_normal; old_normal = tmp;
			tmp = old_point;      old_point = current_point; current_point = next_point; next_point = tmp;
		}
		glEnd();


	}
};
const int MultiCylinder::max_resolution = 20;
Vector MultiCylinder::buffer[max_resolution * 5];
float MultiCylinder::sincosbuffer[max_resolution * 2];

class Plane : public Object
{
public:
	Vector pos, up; float size_x, size_z, quad_size_x, quad_size_z;
	MaterialBase* material;

	Plane() :material(0) {}
	Plane(Vector pos, Vector up, float size_x, float size_z, float quad_size_x = 1, float quad_size_z = 1)
		:Object(pos, Vector(0, 0, 0), Vector(1, 1, 1)), up(up), size_x(size_x), size_z(size_z), quad_size_x(quad_size_x), quad_size_z(quad_size_z), material(0) {}

	virtual void LocalDraw() {
		if (!material) { return; }
		material->Setup();

		glBegin(GL_TRIANGLES);
		glNormal3fv((GLfloat*)&up); float x = 0, newx;
		for (int i = 0; i < size_x / quad_size_x; i++, x = newx)
		{
			float z = 0, newz; newx = x + quad_size_x;
			for (int j = 0; j < size_z / quad_size_z; j++, z = newz)
			{
				newz = z + quad_size_z;
				glTexCoord2f(0, 0);  glVertex3f(x, 0, z);
				glTexCoord2f(1, 0);  glVertex3f(newx, 0, z);
				glTexCoord2f(1, 1);  glVertex3f(newx, 0, newz);

				glTexCoord2f(0, 0);  glVertex3f(x, 0, z);
				glTexCoord2f(1, 1);  glVertex3f(newx, 0, newz);
				glTexCoord2f(0, 1);  glVertex3f(x, 0, newz);
			}
		}
		glEnd();
	}
};


class TCRSpline
{
public:
	Color color;
	PointW  cpoints[10];   int cp_count; //control points
	float last_t; int last_t_i; bool points_valid;
	float sumW; bool sumW_valid; int begin, end;
public:
	Vector   points[120];  int  p_count; //calculated real points
	TCRSpline() :cp_count(0), begin(0), sumW_valid(false), points_valid(false), last_t(-10) {}

	void Clear() { cp_count = 0; }
	int CPCount() { return cp_count; }

	float SumWeight()
	{
		if (sumW_valid) return sumW;
		float sum = 0;
		for (int i = begin; i < end - 1; i++) sum += cpoints[i].w;
		sumW = sum; sumW_valid = true;
		return sum;
	}

	void AddCPoint(PointW pos)
	{
		if (pos.w == 0) pos.w = 0.01f;
		cpoints[cp_count] = pos;
		cp_count++; end++;
		sumW_valid = false;
	}
	void CPointSkip(int first, int last) { begin = first; end = cp_count - last; } // first,last: margin
	void SetAlmostLastCPointWeight(float w) { if (cp_count >= 2) { cpoints[cp_count - 2].w = w; sumW_valid = false; } }
	void SetLastCPointWeight(float w) { if (cp_count >= 1) { cpoints[cp_count - 1].w = w; sumW_valid = false; } }

	static inline Vector TCatmullRomV(PointW& cp1, PointW& cp2, PointW& cp0)
	{
		Vector v = Vector(0, 0);
		v.x = (((cp2.x - cp1.x) / (cp1.w)) + ((cp1.x - cp0.x) / (cp0.w))) * (1 - tension) * 0.5f;
		v.y = (((cp2.y - cp1.y) / (cp1.w)) + ((cp1.y - cp0.y) / (cp0.w))) * (1 - tension) * 0.5f;
		v.z = (((cp2.z - cp1.z) / (cp1.w)) + ((cp1.z - cp0.z) / (cp0.w))) * (1 - tension) * 0.5f;
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
		r.z = cp1.z + v.z * t + t2 * (3 * (cp2.z - cp1.z) / w2 - ((v2.z + 2 * v.z) / w))
			+ t3 * (2 * (cp1.z - cp2.z) / w3 + ((v2.z + v.z) / w2));
		return r;
	}

	void TCatmullRom()
	{
		if (points_valid) return;
		Vector v, v2, r;
		float step = this->SumWeight() / 100;
		int j = 0;
		for (int i = begin; i < end - 1; i++)
		{
			PointW& cp1 = this->cpoints[i];
			PointW& cp2 = this->cpoints[i + 1];
			PointW& cp0 = this->cpoints[i - 1];
			PointW& cp3 = this->cpoints[i + 2];

			if (i == 0) { v = Vector(0, 0); }
			else { v = TCatmullRomV(cp1, cp2, cp0); }
			if (i == this->cp_count - 2) { v2 = Vector(0, 0); }
			else { v2 = TCatmullRomV(cp2, cp3, cp1); }

			for (float f = 0; f < cp1.w; f += step) {
				points[j] = TCatmullRomR(f, cp1, cp2, v, v2); j++;
			}
		}
		points[j] = cpoints[end - 1]; j++;
		p_count = j;
		points_valid = true;
	}

	Vector TCatmullRomVect(float t)
	{
		Vector v, v2, r;

		int i;
		if (t < last_t) i = 0; else i = last_t_i; //cache match
		for (; i < this->cp_count - 1; i++)
		{
			PointW& cp1 = this->cpoints[i];
			PointW& cp2 = this->cpoints[i + 1];
			PointW& cp0 = this->cpoints[i - 1];
			PointW& cp3 = this->cpoints[i + 2];

			if (t > cp1.w) { t -= cp1.w; continue; }

			if (i == 0) { v = Vector(0, 0); }
			else { v = TCatmullRomV(cp1, cp2, cp0); }
			if (i == this->cp_count - 2) { v2 = Vector(0, 0); }
			else { v2 = TCatmullRomV(cp2, cp3, cp1); }

			r = TCatmullRomR(t, cp1, cp2, v, v2);

			return Vector(r.x, r.y);
		}
		return Vector(0, 0);
	}
};

class Frog
{
public:
	Ellipsoid ellipsoids[10];
	Vector pos, rotation;
	float size;

	Frog() :pos(0, 0, 0), rotation(0, 0, 0), size(1) {
		InitFrogBody();
	}
	Frog(Vector pos, Vector rotation, float size) :pos(pos), rotation(rotation), size(size) {
		InitFrogBody();
	}
	void InitFrogBody() {
		ellipsoids[0] = Ellipsoid(Vector(0, 0, 0), Vector(20, 0, 0), &frog_body, 0.7, 0.6, 1.1);
		ellipsoids[1] = Ellipsoid(Vector(0, 0.2, -0.5), Vector(-20, 0, 0), &frog_body, 0.7, 0.6, 1.0);
		ellipsoids[2] = Ellipsoid(Vector(-0.5, 0.6, -0.5), Vector(0, 0, 0), &frog_eye, 0.2, 0.17, 0.1);
		ellipsoids[3] = Ellipsoid(Vector(+0.5, 0.6, -0.5), Vector(0, 0, 0), &frog_eye, 0.2, 0.17, 0.1);

		ellipsoids[4] = Ellipsoid(Vector(-0.7, -0.7, -0.6), Vector(0, 0, 0), &frog_leg, 0.2, 0.7, 0.13);
		ellipsoids[5] = Ellipsoid(Vector(+0.7, -0.7, -0.6), Vector(0, 0, 0), &frog_leg, 0.2, 0.7, 0.13);

		ellipsoids[6] = Ellipsoid(Vector(-0.65, -0.2, +0.6), Vector(0, 40, 0), &frog_leg, 0.4, 0.45, 0.6);
		ellipsoids[7] = Ellipsoid(Vector(+0.65, -0.2, +0.6), Vector(0, -40, 0), &frog_leg, 0.4, 0.45, 0.6);

		for (int i = 0; i < 10; i++) { ellipsoids[i].resolution = 20; }
		ellipsoids[0].resolution = 30;

		ellipsoids[0].AddPart(ellipsoids + 1);
		ellipsoids[1].AddPart(ellipsoids + 2);
		ellipsoids[1].AddPart(ellipsoids + 3);
		ellipsoids[0].AddPart(ellipsoids + 4);
		ellipsoids[0].AddPart(ellipsoids + 5);
		ellipsoids[0].AddPart(ellipsoids + 6);
		ellipsoids[0].AddPart(ellipsoids + 7);
	}
	void Draw() {
		View::ResetTransform();
		glTranslatef(pos.x, pos.y, pos.z);
		glRotatef(rotation.x, 1, 0, 0);
		glRotatef(rotation.y, 0, 1, 0);
		glRotatef(rotation.z, 0, 0, 1);
		glScalef(size, size, size);

		ellipsoids[0].RecursiveDraw();
	}
};
class FireBug {
public:
	int gl_light; Vector pos, rotation; float size;
	FireBug() :gl_light(0), pos(1, 3.3, 4), rotation(0, 0, 0), size(0.1) {
	}
	void SetLight(int gl_light) {
		this->gl_light = gl_light;
		if (gl_light == 0) { glDisable(gl_light); return; }

		float array_[4] = { pos.x, pos.y, pos.z - 0.1, 1 };

		array_[0] = 0; array_[1] = 0; array_[2] = 0; array_[3] = 1;
		glLightfv(gl_light, GL_AMBIENT, array_);

		array_[0] = 0.4; array_[1] = 0.35; array_[2] = 0.01;
		glLightfv(gl_light, GL_DIFFUSE, array_);

		array_[0] = 0.14; array_[1] = 0.10; array_[2] = 0.01;
		glLightfv(gl_light, GL_SPECULAR, array_);

		glLightf(gl_light, GL_CONSTANT_ATTENUATION, 0);
		glLightf(gl_light, GL_LINEAR_ATTENUATION, 0);
		glLightf(gl_light, GL_QUADRATIC_ATTENUATION, 0.02);

		glEnable(gl_light);
	}
	void DisableLight() { SetLight(0); }
	void Draw() {

		float lightpos[4] = { pos.x,pos.y,pos.z - size / 2,1 };
		glLightfv(gl_light, GL_POSITION, lightpos);

		View::ResetTransform();
		glTranslatef(pos.x, pos.y, pos.z);
		glRotatef(rotation.x, 1, 0, 0);
		glRotatef(rotation.y, 0, 1, 0);
		glRotatef(rotation.z, 0, 0, 1);
		glScalef(size / 2, size / 2, size * 2);

		firebug_body.Setup();

		glNormal3f(0, 0, 1);

		Ellipsoid::DrawSphere(8, false, false);
	}
};
class SalamiGolya
{
	static float leg_rads[];
	const static int leg_rad_count;
	static bool leg_rads_inited;
	Ellipsoid eye[2];
public:
	TCRSpline spine; //x,y,z: position from middle
	TCRSpline body;  //x,y,z: scaling from middle
	float beak_angle;
	Vector beak_end, beak_start;

	float leg_angles[6];
public:
	Vector pos, rotation;

	SalamiGolya() :pos(-2.95, 2, 10), rotation(0, 10, 0), beak_angle(0) {
		spine.AddCPoint(PointW(-2.0, -1.7, 0, 1));      body.AddCPoint(PointW(0, 0, 0, 1));  // invisible tail
		spine.AddCPoint(PointW(-1.0, -0.3, 0, 0.4));    body.AddCPoint(PointW(0.0, 0.1, 0.02, 0.4)); // tail
		spine.AddCPoint(PointW(0, -0.0, 0, 1));         body.AddCPoint(PointW(0.5, 0.8, 0.6, 1)); // base point
		spine.AddCPoint(PointW(2.2, 0.6, 0, 1));        body.AddCPoint(PointW(0.20, 0.25, 0.25, 1)); // neck bottom
		spine.AddCPoint(PointW(2, 2.4, 0, 0.2543));     body.AddCPoint(PointW(0.20, 0.20, 0.20, 0.2543)); // neck end

		spine.AddCPoint(PointW(2.4, 2.2, 0, 0.116));    body.AddCPoint(PointW(0.30, 0.26, 0.20, 0.116)); // head
		spine.AddCPoint(PointW(2.7, 2.2, 0, 4.1));      body.AddCPoint(PointW(0.1, 0.1, 0.1, 4.1)); // beak start

		spine.AddCPoint(PointW(4, 2.2, 0, 0.2));        body.AddCPoint(PointW(0, 0, 0, 0.2)); // invisible beak end

		eye[0].material = &golya_eye;     eye[1].material = &golya_eye;
		eye[0].scale = Vector(0.1, 0.1, 0.1); eye[1].scale = Vector(0.1, 0.1, 0.1);

		beak_end = Vector(spine.cpoints[7].x, spine.cpoints[7].y, spine.cpoints[7].z);
		beak_start = Vector(spine.cpoints[6].x, spine.cpoints[6].y, spine.cpoints[6].z);

#if golya_mode == salami
		for (int i = 0; i < 10; i++) { float avg = (body.cpoints[i].x + body.cpoints[i].y + body.cpoints[i].z) / 3;  body.cpoints[i] = PointW(avg, avg, avg, body.cpoints[i].w); }
#endif

		spine.CPointSkip(1, 1); // cuts invisible tail + beak part
		body.CPointSkip(1, 1);

		if (!SalamiGolya::leg_rads_inited) {
			float a = 0;               SalamiGolya::leg_rads_inited = true;
			for (int i = 0; i < leg_rad_count; i++, a += PI / (leg_rad_count - 1)) SalamiGolya::leg_rads[i] = 0.1f - 0.055f * sin(a);
		}
		for (int i = 0; i < 6; i++) leg_angles[i] = 0;
	}
	inline void DrawOG(Vector* normal1, Vector* normal2, Vector* next_normal1, Vector* next_normal2, Vector* point1, Vector* point2, Vector* next_point1, Vector* next_point2, float texX, float texY, float texX2, float texY2)
	{
		glTexCoord2f(texX, texY);	glNormal3fv((GLfloat*)normal1);		  glVertex3f(point1->x, point1->y, point1->z);
		glTexCoord2f(texX2, texY);	glNormal3fv((GLfloat*)next_normal1);  glVertex3f(next_point1->x, next_point1->y, next_point1->z);
		glTexCoord2f(texX, texY2);	glNormal3fv((GLfloat*)normal2);		  glVertex3f(point2->x, point2->y, point2->z);

		glTexCoord2f(texX, texY2);	glNormal3fv((GLfloat*)normal2);       glVertex3f(point2->x, point2->y, point2->z);
		glTexCoord2f(texX2, texY);	glNormal3fv((GLfloat*)next_normal1);  glVertex3f(next_point1->x, next_point1->y, next_point1->z);
		glTexCoord2f(texX2, texY2);	glNormal3fv((GLfloat*)next_normal2);  glVertex3f(next_point2->x, next_point2->y, next_point2->z);
	}
	void DrawFeet(Vector position, float rotation) {
		const Vector middletoeend(6, 0, 0); const Vector middletoenear(1.5, 0, 1.0);
		const Vector lefttoeend(4.3, 0, 1.7); const Vector lefttoenear(0, 0, 1.6);
		const Vector backtoeend(-2.5, 0, 0);

		glPushMatrix();
		glTranslatef(position.x, position.y, position.z);
		glScalef(0.1, 0.1, 0.1);
		glBegin(GL_TRIANGLES); //TALP-lábujjak
		glNormal3f(0, -1, 0);
		glVertex3f(middletoeend.x, 0, 0);   glVertex3f(middletoenear.x, 0, middletoenear.z);   glVertex3f(middletoenear.x, 0, -1 * middletoenear.z); //középsõ
		glVertex3f(middletoenear.x, 0, middletoenear.z);   glVertex3f(lefttoeend.x, 0, lefttoeend.z);    glVertex3f(lefttoenear.x, 0, lefttoenear.z); //bal
		glVertex3f(middletoenear.x, 0, middletoenear.z * -1); glVertex3f(lefttoeend.x, 0, lefttoeend.z * -1); glVertex3f(lefttoenear.x, 0, lefttoenear.z * -1); //jobb
		glEnd();
		glBegin(GL_TRIANGLE_FAN); //TALP-0,0,0 közeli részek
		glVertex3f(0, 0, 0); glVertex3f(middletoenear.x, 0, -1 * middletoenear.z);             glVertex3f(middletoenear.x, 0, middletoenear.z);
		glVertex3f(lefttoenear.x, 0, lefttoenear.z); glVertex3f(backtoeend.x, 0, 0); glVertex3f(lefttoenear.x, 0, -1 * lefttoenear.z);
		glVertex3f(middletoenear.x, 0, -1 * middletoenear.z);
		glEnd();
		glBegin(GL_TRIANGLE_FAN); //LÁBFEJ felsõ része
		glNormal3f(0, 1, 0); glVertex3f(0, 1.0, 0);
		glNormal3f(middletoenear.x, 0.2, -1 * middletoenear.z);   glVertex3f(middletoenear.x, 0, -1 * middletoenear.z);
		glNormal3f(middletoeend.x, 0.2, 0);                     glVertex3f(middletoeend.x, 0, 0);
		glNormal3f(middletoenear.x, 0.2, middletoenear.z);      glVertex3f(middletoenear.x, 0, middletoenear.z);
		glNormal3f(lefttoeend.x, 0.2, lefttoeend.z);            glVertex3f(lefttoeend.x, 0, lefttoeend.z);
		glNormal3f(lefttoenear.x, 0.2, lefttoenear.z);          glVertex3f(lefttoenear.x, 0, lefttoenear.z);
		glNormal3f(backtoeend.x, 0.2, 0);						glVertex3f(backtoeend.x, 0, 0);
		glNormal3f(lefttoenear.x, 0.2, lefttoenear.z * -1);		glVertex3f(lefttoenear.x, 0, lefttoenear.z * -1);
		glNormal3f(lefttoeend.x, 0.2, lefttoeend.z * -1);			glVertex3f(lefttoeend.x, 0, lefttoeend.z * -1);
		glNormal3f(middletoenear.x, 0.2, -1 * middletoenear.z);	glVertex3f(middletoenear.x, 0, -1 * middletoenear.z);
		glEnd();
		glPopMatrix();
	}

	void Draw() {
		View::ResetTransform();
		glTranslatef(pos.x, pos.y, pos.z);
		glRotatef(rotation.x, 1, 0, 0);
		glRotatef(rotation.y, 0, 1, 0);
		glRotatef(rotation.z, 0, 0, 1);

		View::golya_.Setup();
		spine.TCatmullRom();
		body.TCatmullRom();

		static Vector buffer[160];
		Vector* circle = buffer, * nextcircle = buffer + 32, * nextnextcircle = buffer + 128;
		Vector* normals = buffer + 64, * nextnormals = buffer + 96;
		float angle = 0, sin_[32], cos_[32], texY[32], texX, texX2;
		Vector left(0, 0, -1), up = (left % ((spine.points[1] - spine.points[0]).normalize())).normalize();

		for (int j = 0; j < 32; j++, angle += (2 * PI) / ((float)32)) {
			sin_[j] = sin(angle);  // loading the sin+cos values
			cos_[j] = cos(angle);   texY[(j - 8 >= 0) ? (j - 8) : (j - 8 + 32)] = (j < 16) ? (j / 16.0) : (1 - ((j - 16) / 16.0));
#if golya_mode == nonsalami
			circle[j] = spine.points[0] + (up * (sin_[j] * ((j >= 16) ? (body.points[0].x) : (body.points[0].z)))) + left * (cos_[j] * body.points[0].y);
			normals[j] = circle[j] - spine.points[0];
			nextcircle[j] = spine.points[1] + (up * (sin_[j] * ((j >= 16) ? (body.points[1].x) : (body.points[1].z)))) + left * (cos_[j] * body.points[1].y);
#endif
#if golya_mode == salami
			circle[j] = spine.points[0] + (up * sin_[j] * body.points[0].x) + left * (cos_[j] * body.points[0].x);
			normals[j] = circle[j] - spine.points[0];
			nextcircle[j] = spine.points[1] + (up * sin_[j] * body.points[1].x) + left * (cos_[j] * body.points[1].x);
#endif
		}
		texX = 0; texX2 = 1 / (spine.p_count - 1.0f);

		glBegin(GL_TRIANGLES);
		for (int i = 1; i < spine.p_count; i++)
		{
			if (i == spine.p_count - 1) nextnormals[0] = nextcircle[0] - spine.points[i];
			else {
				up = (left % ((spine.points[i + 1] - spine.points[i]).normalize())).normalize();
				nextnextcircle[0] = spine.points[i + 1] + up * (sin_[0] * body.points[i + 1].x) + left * (cos_[0] * body.points[i + 1].y);
				nextnormals[0] = ((nextnextcircle[0] - circle[0]) % (nextcircle[0] - spine.points[i])) % (nextnextcircle[0] - circle[0]);
			}
			angle += (2 * PI) / ((float)32);
			for (int j = 1; j < 32; j++)
			{
				if (i == spine.p_count - 1) nextnormals[j] = nextcircle[j] - spine.points[i];
				else {
#if golya_mode == non_salami
					nextnextcircle[j] = spine.points[i + 1] + up * (sin_[j] * ((j >= 16) ? (body.points[i + 1].x) : (body.points[i + 1].z))) + left * (cos_[j] * body.points[i + 1].y);
#endif
#if golya_mode == salami
					nextnextcircle[j] = spine.points[i + 1] + up * (sin_[j] * body.points[i + 1].x) + left * (cos_[j] * body.points[i + 1].y);
#endif
					nextnormals[j] = ((nextnextcircle[j] - circle[j]) % (nextcircle[j] - spine.points[i])) % (nextnextcircle[j] - circle[j]);
				}

				DrawOG(normals + j - 1, normals + j, nextnormals + j - 1, nextnormals + j, circle + j - 1, circle + j, nextcircle + j - 1, nextcircle + j, texX, texY[j - 1], texX2, texY[j]);
			}
			//végén lezáró téglalap: circle 31->0
			DrawOG(normals + 31, normals + 0, nextnormals + 31, nextnormals + 0, circle + 31, circle + 0, nextcircle + 31, nextcircle + 0, texX, texY[31], texX2, texY[0]);

			Vector* tmp = circle;   circle = nextcircle;   nextcircle = nextnextcircle;  nextnextcircle = tmp;
			tmp = normals; normals = nextnormals; nextnormals = tmp;

			texX = texX2; texX2 += 1 / (spine.p_count - 1.0f);
		}
		glEnd();

		// SZEM
		eye[0].pos = spine.points[98] + up * (0.7 * sin_[29] * ((27 >= 16) ? (body.points[98].x) : (body.points[98].z))) + left * (0.7 * cos_[29] * body.points[98].y);
		eye[0].RecursiveDraw();
		eye[1].pos = spine.points[98] + up * (0.7 * sin_[19] * ((18 >= 16) ? (body.points[98].x) : (body.points[98].z))) + left * (0.7 * cos_[19] * body.points[98].y);
		eye[1].RecursiveDraw();

		// CSÕR
		float radius = body.cpoints[6].x;
		up = (left % (((Vector)(spine.cpoints[7]) - (Vector)(spine.cpoints[6])).normalize())).normalize();

		golya_beak.Setup();
		glBegin(GL_TRIANGLES);

		Vector dir[2]; dir[1] = left * cos_[16] * radius + up * sin_[16] * radius;
		Vector b[2];   b[1] = beak_start + dir[1];       Vector end_normal;

		for (int j = 0; j <= 16; j++) {
			dir[j & 1] = left * cos_[j] * radius + up * sin_[j] * radius;
			b[j & 1] = beak_start + dir[j & 1];

			glNormal3f(dir[0].x, dir[0].y, dir[0].z);
			glVertex3f(b[0].x, b[0].y, b[0].z);        end_normal = (dir[0] + dir[1]) / 2; // ez kivehetõ hogy gyorsabb legyen
			glNormal3f(dir[1].x, dir[1].y, dir[1].z);
			glVertex3f(b[1].x, b[1].y, b[1].z);
			glNormal3f(end_normal.x, end_normal.y, end_normal.z); // persze akkor ezt is ki kell szedni
			glVertex3f(beak_end.x, beak_end.y - 0.1, beak_end.z);
		}												dir[1] = left * cos_[0] * radius + up * sin_[0] * radius;
		b[1] = beak_start + dir[1];
		for (int j = 16; j < 32; j++) {
			dir[j & 1] = left * cos_[j] * radius + up * sin_[j] * radius;
			b[j & 1] = beak_start + dir[j & 1];

			glNormal3f(dir[0].x, dir[0].y, dir[0].z);
			glVertex3f(b[0].x, b[0].y, b[0].z);        end_normal = (dir[0] + dir[1]) / 2; // kivehetõ
			glNormal3f(dir[1].x, dir[1].y, dir[1].z);
			glVertex3f(b[1].x, b[1].y, b[1].z);
			glNormal3f(end_normal.x, end_normal.y, end_normal.z); // ezzel együtt
			glVertex3f(beak_end.x, beak_end.y + 0.1, beak_end.z);
		}
		dir[0] = left * cos_[0] * radius + up * sin_[0] * radius;
		b[0] = beak_start + dir[0];

		glNormal3f(dir[0].x, dir[0].y, dir[0].z);
		glVertex3f(b[0].x, b[0].y, b[0].z);        end_normal = (dir[0] + dir[1]) / 2; // kivehetõ
		glNormal3f(dir[1].x, dir[1].y, dir[1].z);
		glVertex3f(b[1].x, b[1].y, b[1].z);
		glNormal3f(end_normal.x, end_normal.y, end_normal.z); // ezzel együtt
		glVertex3f(beak_end.x, beak_end.y + 0.1, beak_end.z);

		glEnd();

		// LÁB
		MultiCylinder::Draw(Vector(0.7, -1.1, -0.25), Vector(0.7, -0.3, -0.25), leg_rad_count, leg_rads);
		MultiCylinder::Draw(Vector(0.7, -1.97, -0.25), Vector(0.7, -1.1, -0.25), leg_rad_count, leg_rads);

		MultiCylinder::Draw(Vector(0.7, -1.1, +0.25), Vector(0.7, -0.3, +0.25), leg_rad_count, leg_rads);
		MultiCylinder::Draw(Vector(0.7, -1.97, +0.25), Vector(0.7, -1.1, +0.25), leg_rad_count, leg_rads);

		glPushMatrix();
		glTranslatef(0.7, -1.1, -0.25);
		glScalef(0.115, 0.115, 0.115);
		Ellipsoid::DrawSphere(20, false, true);
		glPopMatrix();

		glPushMatrix();
		glTranslatef(0.7, -1.1, +0.25);
		glScalef(0.115, 0.115, 0.115);
		Ellipsoid::DrawSphere(20, false, true);
		glPopMatrix();

		// LÁBFEJ
		DrawFeet(Vector(0.7, -1.98, -0.25), 0);
		DrawFeet(Vector(0.7, -1.98, +0.25), 0);
	}
};
const int SalamiGolya::leg_rad_count = 6;
bool SalamiGolya::leg_rads_inited = false;
float SalamiGolya::leg_rads[SalamiGolya::leg_rad_count];

Ellipsoid ellipsoids[10]; Plane ground; SalamiGolya golya; Frog frog; FireBug firebug;

Vector mousepos, mousedownpos;
bool isAnimating = false; float animationPhase = 0; long lastAnimation = 0;
long mousedowntime = 0, time = 0; bool isMouseDown = false; char tracking = 0;

// Inicializacio, a program futasanak kezdeten, az OpenGL kontextus letrehozasa utan hivodik meg (ld. main() fv.)
void onInitialization() {
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glShadeModel(GL_SMOOTH);
	glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);

	glEnable(GL_NORMALIZE);

	View::Init();
	View::TextureInit();

	View::AddLight(Vector(0, 1, 0), true, Color(1, 1, 1), Color(2, 2, 2), Color(0.2, 0.2, 0.2));

	glEnable(GL_LIGHTING);
	//glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);

	frog.size = 0.3;
	frog.pos = Vector(1, 0.27, 5);
	frog.rotation = Vector(0, 40, 0);

	firebug.SetLight(GL_LIGHT4);

	ground = Plane(Vector(-20, 0, -20), Vector(0, 1, 0), 40, 40, 2, 2);
	ground.material = &(View::grass);
}

// Rajzolas, ha az alkalmazas ablak ervenytelenne valik, akkor ez a fuggveny hivodik meg
void onDisplay()
{
	View::Clear();

	firebug.Draw(); // Elsõként rajzolás a fényforrás miatt
	ground.Draw();
	golya.Draw();
	frog.Draw();


	glutSwapBuffers();     				// Buffercsere: rajzolas vege
}
char navigation = 0; int navtime = 0; bool rotate_enabled = false; float navspeed = 0.02f;
// Billentyuzet esemenyeket lekezelo fuggveny (lenyomas)
void onKeyboard(unsigned char key, int x, int y)
{
	if (key == 'c') { rotate_enabled = !rotate_enabled; }
	if (key == 'r') { navigation |= 1; navtime = time; }
	if (key == 'f') { navigation |= 2; navtime = time; }
	if (key == 'd') { navigation |= 4; navtime = time; }
	if (key == 'g') { navigation |= 8; navtime = time; }
	if (key == 'z') { navigation |= 16; navtime = time; }
	if (key == 'h') { navigation |= 32; navtime = time; }
	if (key == '0') { golya.spine.cpoints[4].w *= 0.9; golya.body.cpoints[4].w *= 0.9; golya.spine.points_valid = false; golya.body.points_valid = false; glutPostRedisplay(); }
	if (key == '1') { golya.spine.cpoints[5].w *= 0.9; golya.body.cpoints[5].w *= 0.9; golya.spine.points_valid = false; golya.body.points_valid = false; glutPostRedisplay(); }
	if (key == '2') { golya.spine.cpoints[6].w *= 0.9; golya.body.cpoints[6].w *= 0.9; golya.spine.points_valid = false; golya.body.points_valid = false; glutPostRedisplay(); }
	if (key == '3')navspeed *= 0.9;
	if (key == '4')navspeed *= 1.1;
}

// Billentyuzet esemenyeket lekezelo fuggveny (felengedes)
void onKeyboardUp(unsigned char key, int x, int y)
{
	if (key == 'r') { navigation &= ~1; }
	if (key == 'f') { navigation &= ~2; }
	if (key == 'd') { navigation &= ~4; }
	if (key == 'g') { navigation &= ~8; }
	if (key == 'z') { navigation &= ~16; }
	if (key == 'h') { navigation &= ~32; }
}

// Eger esemenyeket lekezelo fuggveny
void onMouse(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT && state == GLUT_DOWN) {
		mousedowntime = time;
		isMouseDown = true;

	}
	if (button == GLUT_LEFT && state == GLUT_UP) {
		isMouseDown = false;
	}

	glutPostRedisplay();
}

// Eger mozgast lekezelo fuggveny
void onMouseMotion(int x, int y)
{

}

// `Idle' esemenykezelo, jelzi, hogy az ido telik, az Idle esemenyek frekvenciajara csak a 0 a garantalt minimalis ertek
void onIdle() {
	time = glutGet(GLUT_ELAPSED_TIME);		// program inditasa ota eltelt ido

	if (rotate_enabled) {
		golya.rotation.y = time / (float)60;
		frog.rotation.y = time / (float)60;
		glutPostRedisplay();
	}
	if (navigation) {
		int delta = time - navtime;
		if (navigation & 1)  View::Move(Vector(0, 0, navspeed * delta));
		if (navigation & 2)  View::Move(Vector(0, 0, -navspeed * delta));
		if (navigation & 4)  View::Move(Vector(navspeed * delta, 0, 0));
		if (navigation & 8)  View::Move(Vector(-navspeed * delta, 0, 0));
		if (navigation & 16) View::Move(Vector(0, navspeed * delta, 0));
		if (navigation & 32) View::Move(Vector(0, -navspeed * delta, 0));
		navtime = time;
		glutPostRedisplay();
	}
}

// ...Idaig modosithatod
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
