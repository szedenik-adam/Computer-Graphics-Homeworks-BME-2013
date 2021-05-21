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

#define displaymode_normal 1
#define displaymode_high   2
#define displaymode_vhigh  4
#define displaymode_both   3

const int screenWidth = 600;
const int screenHeight = 600;

#define display_progress   1
#define displaymode      displaymode_both
#define photon_map_limit 200


#if display_progress
#include <stdio.h>
#endif

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
	Vector operator*(float a) const {
		return Vector(x * a, y * a, z * a);
	}
	Vector operator/(float a) const {
		return Vector(x / a, y / a, z / a);
	}
	Vector operator+(const Vector& v) const {
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
	Vector operator-(const Vector& v) const {
		return Vector(x - v.x, y - v.y, z - v.z);
	}
	float operator*(const Vector& v) const { 	// dot product
		return (x * v.x + y * v.y + z * v.z);
	}
	Vector operator%(const Vector& v) const { 	// cross product
		return Vector(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x);
	}
	bool operator==(const Vector& v) const {
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
	static Vector UpVectorDeg(float xyAngle, float yzAngle)//0,0: (0,1,0), 90,0:(1,0,0), 0,90:(0,0,1)
	{
		Vector v = Vector(0, 0, 0);
		v.y = cos(yzAngle * PI / 180); v.z = sin(yzAngle * PI / 180);
		v.x = v.y * sin(xyAngle * PI / 180); v.y = v.y * cos(xyAngle * PI / 180);
		return v;
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
	Color operator*(float a) const {
		return Color(r * a, g * a, b * a);
	}
	Color operator*(const Color& c) const {
		return Color(r * c.r, g * c.g, b * c.b);
	}
	Color operator/(const Color& c) const {
		return Color(r / c.r, g / c.g, b / c.b);
	}
	Color operator+(const Color& c) const {
		return Color(r + c.r, g + c.g, b + c.b);
	}
	Color operator-(const Color& c) const {
		return Color(r - c.r, g - c.g, b - c.b);
	}
	Color operator+=(const Color& v) {
		*this = *this + v;
		return *this;
	}
	bool operator==(const Color& c) const {
		return r == c.r && b == c.b && g == c.g;
	}
};

class Surface
{
public:
	Color ka, kd, ks;//ambiens, diffúz, spekuláris
	Surface(Color ka, Color kd, Color ks) :ka(ka), kd(kd), ks(ks) {}
	virtual bool IsSmooth() { return false; }
	virtual bool IsDiffuse() { return false; }

	Color ReflectedRadiance(const Vector& reflection_dir, const Vector& normal, const Vector& dir, const Color& Lin) {
		float costheta = normal * reflection_dir;
		if (costheta < 0) return Color(0, 0, 0);
		Color Lref = Lin * kd * costheta;		// diffuse reflection
		Vector H = (reflection_dir + dir).normalize();
		float cosdelta = normal * H;
		if (cosdelta < 0) return Lref;
		Lref = Lref + Lin * ks * pow(cosdelta, 0.5f);	// glossy reflection
		return Lref;
	}
};

class SmoothSurface : public Surface
{
public:
	Color F0;//freshnel számok n-k
	float n;//anyagjellemzõ
	bool isReflective;
	bool isRefractive;


	virtual bool IsSmooth() { return true; }

	SmoothSurface(Color n, Color k, Color kd, Color ks, bool isReflective = true, bool isRefractive = false) :
		n((n.r + n.g + n.b) / 3), Surface(k, kd, ks), isReflective(isReflective), isRefractive(isRefractive)
	{
		F0 = ((n - Color(1, 1, 1)) * (n - Color(1, 1, 1)) + (k * k)) / ((n + Color(1, 1, 1)) * (n + Color(1, 1, 1)) + (k * k));
	}

	static Vector Reflection(Vector dir, Vector normal) {
		return dir + normal * ((normal * dir) * (-5));//-2 alapból, -1-nél szaros lesz a fém sík, -5-nél minden hülyeség eltûnik :D
	}
	bool Refraction(Vector& dir, Vector& normal, Vector& refraction_out) {
		float cosa = normal * (-1) * dir, cn;                    cn = n;
		if (cosa < 0) { cosa = cosa * (-1); normal = normal * (-1); }//cn=1/n; }  else cn=n;     
		float disc = 1 - (1 - cosa * cosa) / cn / cn;
		if (disc < 0) return 0;
		refraction_out = dir / cn + normal * (cosa / cn - sqrt(disc));
		return 1;
	}
	Color Fresnel(Vector& dir, Vector& normal) {
		float cosa = fabs(normal.normalize() * dir.normalize());
		return F0 + (Color(1, 1, 1) - F0) * pow(1 - cosa, 5/*<-!!*/);//a hatvány adja meg a visszaverõdés élességét
	}
	static SmoothSurface Glass;     static SmoothSurface Gold;
	static SmoothSurface Diamond;   static SmoothSurface Silver;
	static SmoothSurface Copper;
};
SmoothSurface SmoothSurface::Glass = SmoothSurface(Color(1.5, 1.5, 1.5), Color(0, 0, 0), Color(0, 0, 0), Color(0, 0, 0), false, true);// n,ka,kd,ks
SmoothSurface SmoothSurface::Diamond = SmoothSurface(Color(2.4, 2.4, 2.4), Color(0, 0, 0), Color(0, 0, 0), Color(0, 0, 0), false, true);// <- isReflective:false, isRefractive:true
SmoothSurface SmoothSurface::Gold = SmoothSurface(Color(0.17, 0.35, 1.5), Color(3.1, 2.7, 1.9), Color(0.75164f, 0.60648f, 0.22648f), Color(0.628281f, 0.555802f, 0.366065f));
SmoothSurface SmoothSurface::Copper = SmoothSurface(Color(0.2, 1.1, 1.2), Color(3.6, 2.6, 2.3), Color(0.7038f, 0.27048f, 0.0828f), Color(0.256777f, 0.137622f, 0.086014f));
SmoothSurface SmoothSurface::Silver = SmoothSurface(Color(0.14, 0.16, 0.13), Color(4.1, 2.3, 3.1), Color(0.50754f, 0.50754f, 0.50754f), Color(0.508273f, 0.508273f, 0.508273f));

class DiffuseSurface : public Surface
{
public:
	float shine;//0.1

	Vector phmappos[photon_map_limit];
	Color  phmapcol[photon_map_limit];
	int    phmapN;

	virtual bool IsDiffuse() { return true; }

	DiffuseSurface(Color ka, Color kd, Color ks, float shine) :Surface(ka, kd, ks), shine(shine), phmapN(0) {}

	void AddPhoton(Vector& pos, const Color& color) {
		if (phmapN < photon_map_limit) {
			phmappos[phmapN] = pos;
			phmapcol[phmapN] = color; phmapN++;
		}
	}
	Color PhotonMap(Vector& pos) {
		Color result(0, 0, 0);
		for (int i = 0; i < phmapN; i++) {
			float dist = pos.Distance(phmappos[i]);
			if (dist < 5) {
				dist += 0.5;//  ne legyen dist<1 => fényerõsítés
				result += phmapcol[i] * (1 / (dist * dist));
			}
		}
		return result;
	}

	static DiffuseSurface plastic, plastic2;
};
DiffuseSurface DiffuseSurface::plastic = DiffuseSurface(Color(0.4, 0.05, 0.01), Color(0.4, 0.1, 0.2), Color(0.5, 0.5, 0.5), 0.3);
DiffuseSurface DiffuseSurface::plastic2 = DiffuseSurface(Color(0.15, 0.22, 1.25), Color(0.22, 0.66, 3.2), Color(0.31, 0.61, 1.0), 0.3);


struct Hit {
	float t; Vector pos, normal;
	Surface* mat;
	Hit() :t(-1), pos(Vector(0, 0)), normal(Vector(0, 0)) {}
	Hit(float t, Vector pos, Vector normal, Surface* mat) :t(t), pos(pos), normal(normal), mat(mat) {}
};
#define NO_HIT Hit(-1,Vector(0,0),Vector(0,0),0)

class Object
{
public:
	Surface* material;
	Object(Surface* material) :material(material) {}
	virtual Hit Intersect(Vector offset, Vector dir) = 0;
};
class Barrel : public Object
{
public:
	Vector bottom_circle_middle, up, approx_circle_middle;
	float radius, height, approx_circle_radius;
	bool isTopEnabled;
	Surface* bottom_mat;
public:
	Barrel() :Object((Surface*)&SmoothSurface::Gold) {}
	Barrel(Vector bottomCircleMiddle, Vector upDirection, float radius, float height, Surface* material)
		:radius(radius), height(height), bottom_circle_middle(bottomCircleMiddle), Object(material), bottom_mat(material), isTopEnabled(true)
	{
		up = upDirection.normalize();
		approx_circle_radius = sqrt(radius * radius + (height / 2) * (height / 2)) + 0.001f;
		approx_circle_middle = bottom_circle_middle + (up * (height / 2));
	}

	void SetBottom(Vector bottomCircleMiddle) {
		bottom_circle_middle = bottomCircleMiddle;
		approx_circle_middle = bottom_circle_middle + (up * (height / 2));
	}

	void SetDirection(Vector up) {
		this->up = up.normalize();
		approx_circle_middle = bottom_circle_middle + (up * (height / 2));
	}

	Hit Intersect(Vector offset, Vector dir)
	{
		Vector circle_middle_shifted_offset = offset - approx_circle_middle;
		float a = dir * dir, b = 2 * (dir * circle_middle_shifted_offset), c = (circle_middle_shifted_offset * circle_middle_shifted_offset) - (approx_circle_radius * approx_circle_radius);
		float innerpart = b * b - 4 * a * c;
		if (innerpart < 0) return NO_HIT;//nem metszi a határolókört

		float t[4]; Vector ipoint[4];

		Vector bottom_circle_normal = up * -1;//alsó körlap, felsõt ennek a mintájára és távolságvizsgálat a palásttal
		t[0] = ((bottom_circle_middle - offset) * bottom_circle_normal) / (dir * bottom_circle_normal);//alsó körlap
		t[1] = (isTopEnabled) ? (((bottom_circle_middle + up * height - offset) * up) / (dir * up)) : (-1);//felsõ körlap
		ipoint[0] = offset + dir * t[0];
		ipoint[1] = offset + dir * t[1];
		if (ipoint[0].Distance(bottom_circle_middle) > radius)  t[0] = -1;
		if (ipoint[1].Distance(bottom_circle_middle + up * height) > radius) t[1] = -1;

		Vector middle_shifted_offset = offset - bottom_circle_middle; // bottom_middle:0,0,0 from now

		Vector roota = (dir - (up * (dir * up))); a = roota * roota;
		b = 2 * ((dir - (up * (dir * up))) * (middle_shifted_offset - (up * (middle_shifted_offset * up))));
		Vector rootc = middle_shifted_offset - (up * (middle_shifted_offset * up));
		c = rootc * rootc; c -= radius * radius;//gyökszámítás együtthatói a henger metszéspontjához

		innerpart = b * b - 4 * a * c;
		/*if(true){*/if (innerpart < 0) {//nincs ütközés a palásttal
			if (t[0] < 0.001) {
				if (t[1] < 0.001) return NO_HIT; else return Hit(t[1], ipoint[1], up, material);
			}
			if (t[1] < 0.001) {
				return Hit(t[0], ipoint[0], bottom_circle_normal, bottom_mat);
			}
			if (t[0] < t[1])
				return Hit(t[0], ipoint[0], bottom_circle_normal, bottom_mat);
			else
				return Hit(t[1], ipoint[1], up, material);
			return NO_HIT;
		}

		t[2] = ((-b) - sqrt(innerpart)) / (2 * a);
		t[3] = ((-b) + sqrt(innerpart)) / (2 * a);
		ipoint[2] = offset + (dir * t[2]);
		ipoint[3] = offset + (dir * t[3]); //printf("%lf\t%lf\t%lf\n",ipoint.x,ipoint.y,ipoint.z);

		int mini = -1;// (-) miatt kisebb

		if (!((up * (ipoint[2] - bottom_circle_middle)) < 0 || (up * (ipoint[2] - (bottom_circle_middle + (up * height)))) > 0) && t[2] > 0.001)
		{
			mini = 2;
		}
		else //t[2] mindig kisebb mint t[3]
			if (!((up * (ipoint[3] - bottom_circle_middle)) < 0 || (up * (ipoint[3] - (bottom_circle_middle + (up * height)))) > 0) && t[3] > 0.001)
			{
				mini = 3;
			}
			else {
				if (t[0] < 0.001) {
					if (t[1] < 0.001) return NO_HIT; else { return Hit(t[1], ipoint[1], up, material); }
				}
				if (t[1] < 0.001) {
					return Hit(t[0], ipoint[0], bottom_circle_normal, bottom_mat);
				}
				if (t[0] < t[1]) { return Hit(t[0], ipoint[0], bottom_circle_normal, bottom_mat); }
				else { return Hit(t[1], ipoint[1], up, material); }
				return NO_HIT;
			}

		//bonyolult ciklusból kifejtett if-else szerkezet az elágazásbecslõ és a pipeline miatt
		if (t[0] < t[mini] && t[0]> 0.001) mini = 0;
		if (t[1] < t[mini] && t[1]> 0.001) mini = 1;

		if (mini >= 2) {
			//printf("henger: %lf %lf %lf\n",ipoint.x,ipoint.y,ipoint.z);
			Vector ip = ipoint[mini];
			Vector normal = (ip - bottom_circle_middle) - up * ((ip - bottom_circle_middle) * up);
			return Hit(t[mini], ip, normal, material);// t[0] (-) miatt mindig kisebb t[1]-nél
		}
		else {
			if (mini == -1) return NO_HIT;
			return Hit(t[mini], ipoint[mini], (mini == 0) ? (up * -1) : (up), (mini == 0) ? (bottom_mat) : (material));
		}

	}

};


class Paraboloid : public Object
{
public:
	Vector bottom_pos, direction;
	float focal_dist, radius;

	Paraboloid() :Object((Surface*)&SmoothSurface::Silver) {}
	Paraboloid(Vector bottom_pos, Vector dir, float focal_dist, float radius)
		:bottom_pos(bottom_pos), focal_dist(focal_dist), radius(radius), Object((Surface*)&SmoothSurface::Silver)
	{
		SetDirection(dir);
	}
	void SetDirection(Vector dir) {
		direction = dir.normalize(); printf("dir:  %lf  %lf  %lf\n", direction.x, direction.y, direction.z);
	}

	virtual Hit Intersect(Vector offset, Vector dir)
	{
		float a, b, c, innerpart; Vector normal;

		Vector& n = direction;
		float a1 = dir * dir, a2 = (n * dir) * (dir * n);//a2 == (dir*(n*(n*dir))) == (n*(dir*(n*dir))) != (n*n)*(dir*dir)...
		a = a1 - a2;
		//b=((bottom_pos+(n*focal_dist)-offset)*(dir) + ((offset-bottom_pos)*(n*dir))*n)*2; //(dir*-1), *n fölösleges (010 up-ra mûködik)
		b = (bottom_pos + (n * focal_dist) - offset) * (dir * -1) * 2 - ((n * (offset - bottom_pos)) * (n * dir)) * 2;
		Vector rootc = bottom_pos + (n * focal_dist) - offset;
		c = n * (offset - bottom_pos);
		c = (rootc * rootc) - (c * c);//paraboloid metszéspontja

		innerpart = b * b - 4 * a * c; //printf("%lf\t%lf\t%lf\t%lf\n",innerpart,a,b,c);
		if (innerpart < 0) return NO_HIT;

		float t[2] = { ((-b) - sqrt(innerpart)) / (2 * a), ((-b) + sqrt(innerpart)) / (2 * a) };
		Vector ipoint[2] = { offset + (dir * t[0]), offset + (dir * t[1]) };

		if (ipoint[0].Distance(bottom_pos) > radius) {//parabola távolságához mérve túl messze van
			if (ipoint[1].Distance(bottom_pos) > radius) return NO_HIT;//ez is
			{normal = ipoint[1] - (bottom_pos + n * focal_dist); return Hit(t[1], ipoint[1], normal, material); }//p1
		}

		if (ipoint[0].Distance(offset) > ipoint[1].Distance(offset) && ipoint[1].Distance(bottom_pos) < radius) //p1 közelebb van a kamerához, mint p0 ÉS p1 valid (nem túl távoli parabolapont)
		{
			normal = ipoint[1] - (bottom_pos + n * focal_dist); /*printf("P: %lf %lf %lf\n",ipoint[0].x,ipoint[0].y,ipoint[0].z);*/ return Hit(t[1], ipoint[1], normal, material);
		}
		{normal = ipoint[0] - (bottom_pos + n * focal_dist); /*printf("P: %lf %lf %lf\n",ipoint[0].x,ipoint[0].y,ipoint[0].z);*/ return Hit(t[0], ipoint[0], normal, material); }
	}
};

class Camera
{
public:
	static Vector eye;
	static Vector lookat, up, right;

	inline static Vector GetRay(int& x, int& y) {
		Vector p = (lookat - eye) + right * (((2.f * x) / (screenWidth - 1)) - 1) + up * (((2.f * y) / (screenHeight - 1)) - 1);
		return p;
	}
	inline static Vector OverSampledGetRay4(int& x, int& y, char i) {
		Vector p = (lookat - eye) + right * (((2.f * (x + (i & 1) * 0.5f)) / (screenWidth - 1)) - 1) + up * (((2.f * (y + (i >> 1) * 0.5f)) / (screenHeight - 1)) - 1);
		return p;
	}
	inline static Vector OverSampledGetRay8(int& x, int& y, char i) {
		Vector p = (lookat - eye) + right * (((2.f * (x + (i & 3) * 0.25f)) / (screenWidth - 1)) - 1) + up * (((2.f * (y + (i >> 2) * 0.25f)) / (screenHeight - 1)) - 1);
		return p;
	}
};
Vector Camera::eye = Vector(0, 0, 0);
Vector Camera::lookat = Vector(0, 0, 3), Camera::up = Vector(0, 1, 0), Camera::right(1, 0, 0);

class Lights
{
public:
	static Vector plight;
	static float pintensity;
	static int nLights;
	static Vector plights[];
	static float pints[];

	static float sky_intensity;
	static float sky_pos_y;
	static Vector sky_dir;

	static void SetSky(float intensity, float y_level, Vector light_dir) {
		sky_intensity = intensity;
		sky_pos_y = y_level;
		sky_dir = light_dir;
	}
	static void AddPointLight(Vector pos, float intensity) {
		plights[nLights] = pos;
		pints[nLights] = intensity;
		nLights++;
	}
};
Vector Lights::plight = Vector(0, 10, 30);
float Lights::pintensity = 20.f;
int Lights::nLights = 0;
Vector Lights::plights[10];
float Lights::pints[10];
float Lights::sky_intensity = 0.3f;
float Lights::sky_pos_y = 100.f;
Vector Lights::sky_dir = Vector(0, -1, 0);

class Scene
{
public:
	static Object* objects[];  static int object_count;
	static void AddObject(Object* object)
	{
		objects[object_count] = object; object_count++;
	}

	static Hit IntersectAll(Vector offset, Vector dir)
	{
		Hit hit = Hit(pow(10.f, 10), Vector(0, 0), Vector(0, 0), 0);
		for (int i = 0; i < object_count; i++)
		{
			Hit newhit = objects[i]->Intersect(offset, dir);
			if (newhit.t > 0.001f/*epszilon minimum, a sajátütközés ellen, (>0:valid teszt)*/ && newhit.t < hit.t) { hit = newhit; }
		}
		if (hit.t >= pow(10.f, 10)) hit.t = -1;
		return hit;
	}
	static Color Trace(Vector offset, Vector dir, int rec_limit = 7)
	{
		if (rec_limit <= 0) return Color(0, 0, 0);
		Vector normal; Hit hit;
		hit = IntersectAll(offset, dir);
		if (hit.t < 0.001) return Color(0.75, 0.75, 0.75);//no intersection: sky-color:white (-black)
		Color result = hit.mat->ka * 0.02f;//there is an object: ambient object color color = ka*La //direct illumination*/

		hit.normal.normalize();

		//Hit skyshadowed=Scene::IntersectAll(hit.pos,Lights::sky_dir); // ez sokat lassít (lehet a párhuzamos nagy henger palástja miatt)
		//if(skyshadowed.t < 0.001f || skyshadowed.pos.y > Lights::sky_pos_y )
		{
			result += hit.mat->ReflectedRadiance(Lights::sky_dir, hit.normal, dir * (-1), Color(1, 1, 1) * Lights::sky_intensity);
		}

		for (int i = 0; i < Lights::nLights; i++)//foreach lightsource
		{
			Vector lightsource = Lights::plights[i]; Vector shadowdir = lightsource - hit.pos;
			Hit shadowhit = Scene::IntersectAll(hit.pos, shadowdir);

			if (shadowhit.t < 0.001f || hit.pos.Distance(shadowhit.pos)>hit.pos.Distance(lightsource))//nincs árnyékolva a fényforrás^^
			{
				float lightI = Lights::pints[i] / pow(hit.pos.Distance(lightsource), 2);
				//result+=hit.mat->ka*lightI;

				shadowdir.normalize();

				if (dir * hit.normal > 0) shadowdir = shadowdir * -1;// ez oldja meg a paraboloid belsõ oldalának világítását!!!!
				result += hit.mat->ReflectedRadiance(shadowdir/**-1*/, hit.normal, dir * (-1), Color(1, 1, 1) * lightI);

				if (hit.mat->IsDiffuse()) {
					result += ((DiffuseSurface*)hit.mat)->PhotonMap(hit.pos);
				}
			}
		}

		if (hit.mat->IsSmooth())
		{
			SmoothSurface* surface = (SmoothSurface*)(hit.mat);

			if (surface->isReflective)//isReflective
			{
				Vector reflection_dir = SmoothSurface::Reflection(dir, hit.normal);//néha kell -1 a dir-hez, néha nem xD
				Color mr_fresnel = surface->Fresnel(dir, hit.normal);
				Color mr_trace = Trace(hit.pos, reflection_dir, rec_limit - 1);
				Color mr_combo = mr_fresnel * mr_trace;
				if (!(mr_combo == Color(0, 0, 0))) result += mr_combo;
			}
			Vector refraction_out;
			if (surface->isRefractive && surface->Refraction(dir, hit.normal, refraction_out))//surface->ka==Color(0,0,0)//isRefractive
			{
				//if(dir*hit.normal>0) dir= dir*-1; //freshnel dir

				//if(refraction_out*hit.normal>0) refraction_out= refraction_out*-1;
				Color mr_fresnel = surface->Fresnel(dir, hit.normal);//Color mr_fresnel = surface->Fresnel(//dir//refraction_out//,hit.normal);
				Color mr_trace = Trace(hit.pos, refraction_out, rec_limit - 1);//Color mr_trace = Trace(hit.pos, refraction_out//*-1//, rec_limit-1);//-1-tõl szuperfényes lesz
				result += (Color(1, 1, 1) - (mr_fresnel)) * mr_trace;
			}
		}
		return result;

	}
	static void Shoot(Color light, Vector offset, Vector dir, bool isPointLight = true, int rec_limit = 3)
	{
		if (rec_limit < 0) return;
		Hit hit = IntersectAll(offset, dir);
		if (hit.t < 0.001f) return;
		if (hit.mat->IsSmooth()) {
			SmoothSurface* surface = ((SmoothSurface*)hit.mat);
			if (surface->isReflective) {
				Vector reflected_ray = SmoothSurface::Reflection(dir, hit.normal);
				Shoot(surface->Fresnel(dir, hit.normal) * light, hit.pos, reflected_ray, isPointLight, rec_limit - 1);
			}
			Vector refraction_out;
			if (surface->isRefractive && surface->Refraction(dir, hit.normal, refraction_out)) {
				Shoot((Color(1, 1, 1) - surface->Fresnel(dir, hit.normal)) * light, hit.pos, refraction_out, isPointLight, rec_limit - 1);
			}
		}
		if (hit.mat->IsDiffuse() && rec_limit != 3)
			((DiffuseSurface*)hit.mat)->AddPhoton(hit.pos, (isPointLight) ? (light * (1 / pow(offset.Distance(hit.pos), 2))) : (light));
	}
};
Object* Scene::objects[20];
int Scene::object_count = 0;

Barrel b[10];
Paraboloid p[10];
Color image[screenWidth * screenHeight];

// Inicializacio, a program futasanak kezdeten, az OpenGL kontextus letrehozasa utan hivodik meg (ld. main() fv.)
void onInitialization() {
	b[0] = Barrel(Vector(0, 10, 50), Vector(0.2, 0.1, 0.5), 4, 20, (Surface*)&SmoothSurface::Copper);//DiffuseSurface::plastic);
	b[2] = Barrel(Vector(2, -4.6, 25), Vector(-0.3, 1, 0.1), 2, 2, &SmoothSurface::Glass);
	b[3] = Barrel(Vector(-2.9, -0.2, 18), Vector(0.9, -0.2, -0.3), 1, 3.3, &SmoothSurface::Silver);

	p[0] = Paraboloid(Vector(15.5, 21.5, 60), Vector(-0.4, -1, 0.3), 5.f, 10.f);
	p[0].material = (Surface*)&SmoothSurface::Silver;
	b[1] = Barrel(Vector(0, -10, 35), Vector(0, 1, 0), 40, 50, &SmoothSurface::Gold); b[1].bottom_mat = &DiffuseSurface::plastic2; b[1].isTopEnabled = false;

	Scene::AddObject((Object*)(b + 1));//nagy henger
	Scene::AddObject((Object*)(b + 0));//kis réz
	Scene::AddObject((Object*)(b + 2));//üveg henger
	Scene::AddObject((Object*)(b + 3));//ezüst henger
	Scene::AddObject((Object*)(p + 0));//jobb oldali p, ezüst

	//Lights::AddPointLight(Vector(0,0,0),30.f);
	Lights::AddPointLight(p[0].bottom_pos + p[0].direction * 4, 80.f);
}

// Rajzolas, ha az alkalmazas ablak ervenytelenne valik, akkor ez a fuggveny hivodik meg
void onDisplay()
{
	for (int j = 0; j < 100000; j++)
	{
		float angle = rand(); float rad = (rand() * b[1].radius) / (float)RAND_MAX;
		Vector pos(b[1].bottom_circle_middle.x + cos(angle) * rad, b[1].bottom_circle_middle.y + b[1].height - 0.2f, b[1].bottom_circle_middle.z + sin(angle) * rad);
		Scene::Shoot(Color(1, 1, 1) * Lights::sky_intensity, pos, Lights::sky_dir, false);

		for (int i = 0; i < Lights::nLights; i++) {
			Vector dir = Vector(rand(), rand(), rand()).normalize();
			Scene::Shoot(Color(1, 1, 1) * Lights::pints[i], Lights::plights[i], dir);
		}
	}

	int progress = 0;
#if (displaymode & displaymode_normal)
	for (int y = 0, offset = 0; y < screenHeight; y++) {
		for (int x = 0; x < screenWidth; x++, offset++)
		{
			Vector ray = Camera::GetRay(x, y);
			image[offset] = Scene::Trace(Camera::eye, ray);
		}
#ifdef display_progress
		int pnew = (((y + 1) * 100) / screenHeight); if (pnew != progress) { progress = pnew; printf("%d\n", pnew); }
#endif
	}

	glDrawPixels(screenWidth, screenHeight, GL_RGB, GL_FLOAT, image);
	glutSwapBuffers();
#endif

#if (displaymode & (displaymode_high|displaymode_vhigh))
	for (int y = 0, offset = 0; y < screenHeight; y++) {
		for (int x = 0; x < screenWidth; x++, offset++)
		{
#if (displaymode & displaymode_high)
			Vector ray1 = Camera::OverSampledGetRay4(x, y, 0);
			Vector ray2 = Camera::OverSampledGetRay4(x, y, 1);
			Vector ray3 = Camera::OverSampledGetRay4(x, y, 2);
			Vector ray4 = Camera::OverSampledGetRay4(x, y, 3);
			image[offset] = (Scene::Trace(Camera::eye, ray1) + Scene::Trace(Camera::eye, ray2) + Scene::Trace(Camera::eye, ray3) + Scene::Trace(Camera::eye, ray4)) * 0.25f;
#endif
#if (displaymode & displaymode_vhigh)
			Vector ray1 = Camera::OverSampledGetRay8(x, y, 0); Vector ray5 = Camera::OverSampledGetRay8(x, y, 4);
			Vector ray2 = Camera::OverSampledGetRay8(x, y, 1); Vector ray6 = Camera::OverSampledGetRay8(x, y, 5);
			Vector ray3 = Camera::OverSampledGetRay8(x, y, 2); Vector ray7 = Camera::OverSampledGetRay8(x, y, 6);
			Vector ray4 = Camera::OverSampledGetRay8(x, y, 3); Vector ray8 = Camera::OverSampledGetRay8(x, y, 7);
			image[offset] = (Scene::Trace(Camera::eye, ray1) + Scene::Trace(Camera::eye, ray2) + Scene::Trace(Camera::eye, ray3) + Scene::Trace(Camera::eye, ray4)
				+ Scene::Trace(Camera::eye, ray4) + Scene::Trace(Camera::eye, ray5) + Scene::Trace(Camera::eye, ray6) + Scene::Trace(Camera::eye, ray7)) * 0.125f;
#endif
		}
#ifdef display_progress
		int pnew = (((y + 1) * 100) / screenHeight); if (pnew != progress) { progress = pnew; printf("%d\n", pnew); }
#endif
	}
	glDrawPixels(screenWidth, screenHeight, GL_RGB, GL_FLOAT, image);
	glutSwapBuffers();     				// Buffercsere: rajzolas vege
#endif
}
float camera_r = 0, camera_rr = 0;
// Billentyuzet esemenyeket lekezelo fuggveny (lenyomas)
void onKeyboard(unsigned char key, int x, int y)
{
	if (key == 'w') { camera_rr += 0.1f; Camera::lookat = Vector(3 * sin(camera_r) * cos(camera_rr), 3 * sin(camera_rr), 3 * cos(camera_r) * cos(camera_rr)); }
	if (key == 'd') { camera_r += 0.1f;  Camera::lookat = Vector(3 * sin(camera_r) * cos(camera_rr), 3 * sin(camera_rr), 3 * cos(camera_r) * cos(camera_rr)); }
	if (key == 'a') { camera_r -= 0.1f;  Camera::lookat = Vector(3 * sin(camera_r) * cos(camera_rr), 3 * sin(camera_rr), 3 * cos(camera_r) * cos(camera_rr)); }
	if (key == 's') { camera_rr -= 0.1f; Camera::lookat = Vector(3 * sin(camera_r) * cos(camera_rr), 3 * sin(camera_rr), 3 * cos(camera_r) * cos(camera_rr)); }

	if (key == '0') SmoothSurface::Glass.n = 1;
	if (key == '+') SmoothSurface::Glass.n += 0.1;
	if (key == ' ') { glutPostRedisplay(); }
}

// Billentyuzet esemenyeket lekezelo fuggveny (felengedes)
void onKeyboardUp(unsigned char key, int x, int y)
{

}

// Eger esemenyeket lekezelo fuggveny
void onMouse(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT && state == GLUT_DOWN)
	{
		int yy = 600 - y;
		Vector ray = Camera::GetRay(x, yy);
		Scene::Trace(Camera::eye, ray);
	}
}

// Eger mozgast lekezelo fuggveny
void onMouseMotion(int x, int y)
{

}
float Angle = 0;
// `Idle' esemenykezelo, jelzi, hogy az ido telik, az Idle esemenyek frekvenciajara csak a 0 a garantalt minimalis ertek
void onIdle() {
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)     
	Sleep(100);
#else
	usleep(100 * 1000);
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
