#pragma once

#include <glm/glm.hpp>
#define Color glm::vec3
#define PointW glm::vec4
#define tension       -0.5f

#include "Texture.h"
#include "Frog.h"

extern Frog frogs[]; 
extern int frog_count;

class Vector : public glm::vec3
{
public:
	Vector() :glm::vec3() {}
	Vector(const glm::vec3& v) :glm::vec3(v) {}
	Vector(float x, float y, float z) :glm::vec3(x,y,z) {}
	Vector operator%(const glm::vec3& v) { 	// cross product
		return Vector(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x);
	}
	/*float Length() { return sqrt(x * x + y * y + z * z); }
	float Distance(const Vector& v) {
		Vector delta = Vector(x - v.x, y - v.y, z - v.z);
		return sqrt(delta.x * delta.x + delta.y * delta.y + delta.z * delta.z);
	}
	Vector& normalize() {
		float len = this->Length();
		*this = (*this) * (1 / len);
		return *this;
	}*/
	Vector Rotate2D(const Vector& offset, float angleDeg) {
		float angle = angleDeg * PI / 180;  Vector result, delta = (*this) - offset;
		result.x = cos(angle) * delta.x - sin(angle) * delta.y + offset.x;
		result.y = sin(angle) * delta.x + cos(angle) * delta.y + offset.y; result.z = this->z;
		return result;
	}
};

class TCRSpline
{
public:
	Color color;
	PointW  cpoints[10];   int cp_count;
	float last_t; int last_t_i; bool points_valid;
	float sumW; bool sumW_valid; int begin, end;
	int resolution;
public:
	glm::vec3   points[120];  int  p_count;
	TCRSpline() :resolution(100), cp_count(0), begin(0), sumW_valid(false), points_valid(false), last_t(-10) {}

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
	void CPointSkip(int first, int last) { begin = first; end = cp_count - last; }
	void SetAlmostLastCPointWeight(float w) { if (cp_count >= 2) { cpoints[cp_count - 2].w = w; sumW_valid = false; } }
	void SetLastCPointWeight(float w) { if (cp_count >= 1) { cpoints[cp_count - 1].w = w; sumW_valid = false; } }

	static inline glm::vec3 TCatmullRomV(PointW& cp1, PointW& cp2, PointW& cp0)
	{
		glm::vec3 v;
		v.x = (((cp2.x - cp1.x) / (cp1.w)) + ((cp1.x - cp0.x) / (cp0.w))) * (1 - tension) * 0.5f;
		v.y = (((cp2.y - cp1.y) / (cp1.w)) + ((cp1.y - cp0.y) / (cp0.w))) * (1 - tension) * 0.5f;
		v.z = (((cp2.z - cp1.z) / (cp1.w)) + ((cp1.z - cp0.z) / (cp0.w))) * (1 - tension) * 0.5f;
		return v;
	}

	static inline glm::vec3 TCatmullRomR(float t, PointW& cp1, PointW& cp2, glm::vec3& v, glm::vec3& v2)
	{
		glm::vec3 r;      float w = cp1.w;
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
		glm::vec3 v, v2, r;
		float step = this->SumWeight() / resolution;
		int j = 0;
		for (int i = begin; i < end - 1; i++)
		{
			PointW& cp1 = this->cpoints[i];
			PointW& cp2 = this->cpoints[i + 1];
			PointW& cp0 = this->cpoints[i - 1];
			PointW& cp3 = this->cpoints[i + 2];

			if (i == 0) { v = glm::vec3(0, 0, 0); }
			else { v = TCatmullRomV(cp1, cp2, cp0); }
			if (i == this->cp_count - 2) { v2 = glm::vec3(0, 0, 0); }
			else { v2 = TCatmullRomV(cp2, cp3, cp1); }

			for (float f = 0; f < cp1.w; f += step) {
				points[j] = TCatmullRomR(f, cp1, cp2, v, v2); j++;
			}
		}
		points[j] = cpoints[end - 1]; j++;
		p_count = j;
		points_valid = true;
	}

	glm::vec3 TCatmullRomVect(float t)
	{
		glm::vec3 v, v2, r;

		int i;
		if (t < last_t) i = 0; else i = last_t_i;
		for (; i < this->cp_count - 1; i++)
		{
			PointW& cp1 = this->cpoints[i];
			PointW& cp2 = this->cpoints[i + 1];
			PointW& cp0 = this->cpoints[i - 1];
			PointW& cp3 = this->cpoints[i + 2];

			if (t > cp1.w) { t -= cp1.w; continue; }

			if (i == 0) { v = glm::vec3(0, 0, 0); }
			else { v = TCatmullRomV(cp1, cp2, cp0); }
			if (i == this->cp_count - 2) { v2 = glm::vec3(0, 0, 0); }
			else { v2 = TCatmullRomV(cp2, cp3, cp1); }

			r = TCatmullRomR(t, cp1, cp2, v, v2);

			return glm::vec3(r.x, r.y, r.z);
		}
		return glm::vec3(0, 0, 0);
	}
};

class MultiCylinder
{
	static const int max_resolution;
	static glm::vec3 buffer[];
	static float sincosbuffer[];
	static std::optional<VertexBuffer> vbuf;
public:
	static void Draw(glm::vec3 pbottom, glm::vec3 ptop, size_t subcylinder_count, float* subcylinder_rads, size_t resolution = 10)
	{
		glm::vec3* old_point = buffer, * current_point = buffer + resolution, * next_point = buffer + 4 * resolution;
		glm::vec3* old_normal = buffer + 2 * resolution, * current_normal = buffer + 3 * resolution;
		float* _sin = sincosbuffer; float* _cos = sincosbuffer + resolution;
		float angle = 0;
		glm::vec3 dir = glm::normalize(ptop - pbottom);
		glm::vec3 d1;
		if (dir.z != 0) { d1 = glm::vec3(dir.z * (-1), dir.z * (-1), dir.x + dir.y); } // if-else nélkül rossz lenne 45fokra
		else { d1 = glm::vec3(dir.y + dir.z, dir.x * (-1), dir.x * (-1)); } d1 = glm::normalize(d1);
		glm::vec3 d2(glm::cross(dir, d1));							  d2 = glm::normalize(d2);
		dir = (ptop - pbottom) / (float)(subcylinder_count - 1);

		for (int i = 0; i < resolution; i++, angle += 2 * PI / resolution) {
			_sin[i] = sin(angle); _cos[i] = cos(angle);
			old_normal[i] = (d1 * _cos[i] + d2 * _sin[i]) * subcylinder_rads[0];
			old_point[i] = pbottom + old_normal[i];
			current_point[i] = pbottom + dir + (d1 * _cos[i] + d2 * _sin[i]) * subcylinder_rads[1];
		}
		//glBegin(GL_TRIANGLES);
		size_t vertexCount = (subcylinder_count - 1) * (6 + (resolution - 1) * 6);
		Vertex* verts = new Vertex[vertexCount];
		Vertex* vertit = verts;

		for (int i = 1; i < subcylinder_count; i++) {
			float rad = *(++subcylinder_rads);
			pbottom += dir;

			if (i == subcylinder_count - 1) { current_normal[0] = (d1 * _cos[0] + d2 * _sin[0]) * rad; current_point[0] = pbottom + current_normal[0]; }
			else {
				next_point[0] = pbottom + dir + (d1 * _cos[0] + d2 * _sin[0]) * (*(subcylinder_rads + 1));
				current_normal[0] = glm::cross(glm::cross((next_point[0] - old_point[0]), (current_point[0] - pbottom)), (next_point[0] - old_point[0]));
			}

			for (int j = 1; j < resolution; j++) {
				if (i == subcylinder_count - 1) { current_normal[j] = (d1 * _cos[j] + d2 * _sin[j]) * rad; current_point[j] = pbottom + current_normal[j]; }
				else {
					next_point[j] = pbottom + dir + (d1 * _cos[j] + d2 * _sin[j]) * (*(subcylinder_rads + 1));
					current_normal[j] = glm::cross(glm::cross((next_point[j] - old_point[j]), (current_point[j] - pbottom)), (next_point[j] - old_point[j]));
				}

				/*glNormal3fv((GLfloat*)(current_normal + j - 1));  glVertex3f(current_point[j - 1].x, current_point[j - 1].y, current_point[j - 1].z);
				glNormal3fv((GLfloat*)(current_normal + j));  glVertex3f(current_point[j].x, current_point[j].y, current_point[j].z);
				glNormal3fv((GLfloat*)(old_normal + j - 1));  glVertex3f(old_point[j - 1].x, old_point[j - 1].y, old_point[j - 1].z);

				glNormal3fv((GLfloat*)(current_normal + j));  glVertex3f(current_point[j].x, current_point[j].y, current_point[j].z);
				glNormal3fv((GLfloat*)(old_normal + j));  glVertex3f(old_point[j].x, old_point[j].y, old_point[j].z);
				glNormal3fv((GLfloat*)(old_normal + j - 1));  glVertex3f(old_point[j - 1].x, old_point[j - 1].y, old_point[j - 1].z);*/

				*vertit++ = { .pos = glm::vec3(current_point[j - 1].x, current_point[j - 1].y, current_point[j - 1].z), .tex = glm::vec2(0,0), .norm = current_normal[j - 1] };
				*vertit++ = { .pos = glm::vec3(current_point[j].x, current_point[j].y, current_point[j].z), .tex = glm::vec2(0,0), .norm = current_normal[j] };
				*vertit++ = { .pos = glm::vec3(old_point[j - 1].x, old_point[j - 1].y, old_point[j - 1].z), .tex = glm::vec2(0,0), .norm = old_normal[j - 1] };

				*vertit++ = { .pos = glm::vec3(current_point[j].x, current_point[j].y, current_point[j].z), .tex = glm::vec2(0,0), .norm = current_normal[j] };
				*vertit++ = { .pos = glm::vec3(old_point[j].x, old_point[j].y, old_point[j].z), .tex = glm::vec2(0,0), .norm = old_normal[j] };
				*vertit++ = { .pos = glm::vec3(old_point[j - 1].x, old_point[j - 1].y, old_point[j - 1].z), .tex = glm::vec2(0,0), .norm = old_normal[j - 1] };
			}
			/*glNormal3fv((GLfloat*)(current_normal + resolution - 1));  glVertex3f(current_point[resolution - 1].x, current_point[resolution - 1].y, current_point[resolution - 1].z);
			glNormal3fv((GLfloat*)(current_normal + 0));           glVertex3f(current_point[0].x, current_point[0].y, current_point[0].z);
			glNormal3fv((GLfloat*)(old_normal + resolution - 1));  glVertex3f(old_point[resolution - 1].x, old_point[resolution - 1].y, old_point[resolution - 1].z);

			glNormal3fv((GLfloat*)(current_normal + 0));           glVertex3f(current_point[0].x, current_point[0].y, current_point[0].z);
			glNormal3fv((GLfloat*)(old_normal + 0));           glVertex3f(old_point[0].x, old_point[0].y, old_point[0].z);
			glNormal3fv((GLfloat*)(old_normal + resolution - 1));  glVertex3f(old_point[resolution - 1].x, old_point[resolution - 1].y, old_point[resolution - 1].z);*/

			*vertit++ = { .pos = glm::vec3(current_point[resolution - 1].x, current_point[resolution - 1].y, current_point[resolution - 1].z), .tex = glm::vec2(0,0), .norm = current_normal[resolution - 1] };
			*vertit++ = { .pos = glm::vec3(current_point[0].x, current_point[0].y, current_point[0].z), .tex = glm::vec2(0,0), .norm = current_normal[0] };
			*vertit++ = { .pos = glm::vec3(old_point[resolution - 1].x, old_point[resolution - 1].y, old_point[resolution - 1].z), .tex = glm::vec2(0,0), .norm = old_normal[resolution - 1] };

			*vertit++ = { .pos = glm::vec3(current_point[0].x, current_point[0].y, current_point[0].z), .tex = glm::vec2(0,0), .norm = current_normal[0] };
			*vertit++ = { .pos = glm::vec3(old_point[0].x, old_point[0].y, old_point[0].z), .tex = glm::vec2(0,0), .norm = old_normal[0] };
			*vertit++ = { .pos = glm::vec3(old_point[resolution - 1].x, old_point[resolution - 1].y, old_point[resolution - 1].z), .tex = glm::vec2(0,0), .norm = old_normal[resolution - 1] };

			glm::vec3* tmp = current_normal; current_normal = old_normal; old_normal = tmp;
			tmp = old_point;      old_point = current_point; current_point = next_point; next_point = tmp;
		}
		//glEnd();
		if (vbuf.has_value()) { vbuf.value().Update(verts, vertexCount); }
		else { vbuf.emplace(verts, vertexCount); }
		delete[] verts;

		vbuf.value().Draw();
	}
};
/*const int MultiCylinder::max_resolution = 20;
glm::vec3 MultiCylinder::buffer[max_resolution * 5];
float MultiCylinder::sincosbuffer[max_resolution * 2];*/

class SalamiGolya
{
	static float leg_rads[];
	const static int leg_rad_count;
	static bool leg_rads_inited;
	static const float upperlegdist; static const float lowerlegdist;
	static const float upperlegX;    static const float upperlegY;    static const float upperlegZ;
	static const PointW spine_vals[], body_vals[];

	Ellipsoid eye[2];
	glm::vec3 beak_start; float old_feetX, old_feetY; bool standing_on_left;
	std::optional<VertexBuffer> foot_vbuf, body_vbuf, beak_vbuf;
public:
	TCRSpline spine;
	TCRSpline body;
	float leg_angles[6]; //top,middle,bottom angle(45..0(normál)- -90(hátrafeszített talp))
	float height;
	float beak_angle, head_angle, neck_upper_angle, neck_lower_angle; //these are read only
	glm::vec3 beak_end[2]; //read only too
public:
	glm::vec3 pos, rotation;

	SalamiGolya() :pos(-2.95, 0, 10), rotation(0, 10, 0), old_feetX(0), standing_on_left(true), beak_angle(0), head_angle(0), neck_upper_angle(0), neck_lower_angle(0) {
		for (int i = 0; i < 8; i++) {
			spine.AddCPoint(SalamiGolya::spine_vals[i]);
			body.AddCPoint(SalamiGolya::body_vals[i]);
		}

		eye[0].material = &Material::golya_eye;     eye[1].material = &Material::golya_eye;
		eye[0].scale = glm::vec3(0.1, 0.1, 0.1); eye[1].scale = glm::vec3(0.1, 0.1, 0.1);

		this->SetBeakAngle(5);

#if golya_mode == salami
		for (int i = 0; i < 10; i++) { float avg = (body.cpoints[i].x + body.cpoints[i].y + body.cpoints[i].z) / 3;  body.cpoints[i] = PointW(avg, avg, avg, body.cpoints[i].w); }
#endif

		spine.CPointSkip(1, 1);
		body.CPointSkip(1, 1);

		if (!SalamiGolya::leg_rads_inited) {
			float a = 0;               SalamiGolya::leg_rads_inited = true;
			for (int i = 0; i < leg_rad_count; i++, a += PI / (leg_rad_count - 1)) SalamiGolya::leg_rads[i] = 0.1f - 0.055f * sin(a);
		}
		for (int i = 0; i < 6; i++) leg_angles[i] = 0;
		height = std::max(upperlegdist * sin(leg_angles[0] * PI / 180 + PI / 2) + lowerlegdist * sin((leg_angles[1] + leg_angles[0]) * PI / 180 + PI / 2),
			upperlegdist * sin(leg_angles[3] * PI / 180 + PI / 2) + lowerlegdist * sin((leg_angles[4] + leg_angles[3]) * PI / 180 + PI / 2)) - 1.57 + 2;
	}
	static glm::vec3 Rotate2D(const glm::vec3& point, const glm::vec3& offset, float angleDeg)
	{
		float angle = angleDeg * PI / 180;  glm::vec3 result, delta = point - offset;
		result.x = cos(angle) * delta.x - sin(angle) * delta.y + offset.x;
		result.y = sin(angle) * delta.x + cos(angle) * delta.y + offset.y; result.z = point.z;
		return result;
	}

	inline void CalculateLegArray(float* leg_x, float* leg_y) {
		float leg_sin[4], leg_cos[4]; //leftMiddle,leftFeet,rightMiddle,rightFeet

		leg_sin[0] = leg_angles[0] * PI / 180 + PI / 2;       //felsõ láb szögének szinusza
		leg_sin[1] = leg_angles[1] * PI / 180 + leg_sin[0];   //alsó láb szögének szinusza
		leg_sin[2] = leg_angles[3] * PI / 180 + PI / 2;       //jobb oldali felsõ láb szinusza
		leg_sin[3] = leg_angles[4] * PI / 180 + leg_sin[2];
		leg_cos[0] = cos(leg_sin[0]); leg_sin[0] = sin(leg_sin[0]);
		leg_cos[1] = cos(leg_sin[1]); leg_sin[1] = sin(leg_sin[1]);
		leg_cos[2] = cos(leg_sin[2]); leg_sin[2] = sin(leg_sin[2]);
		leg_cos[3] = cos(leg_sin[3]); leg_sin[3] = sin(leg_sin[3]);

		float hl = upperlegdist * leg_sin[0] + lowerlegdist * leg_sin[1];
		float hr = upperlegdist * leg_sin[2] + lowerlegdist * leg_sin[3];
		leg_y[0] = upperlegY - upperlegdist * leg_sin[0];//leftMiddle
		leg_x[0] = upperlegX - upperlegdist * leg_cos[0];
		leg_y[1] = leg_y[0] - lowerlegdist * leg_sin[1];//leftFeet
		leg_x[1] = leg_x[0] - lowerlegdist * leg_cos[1];
		leg_y[2] = upperlegY - upperlegdist * leg_sin[2];//rightMiddle
		leg_x[2] = upperlegX - upperlegdist * leg_cos[2];
		leg_y[3] = leg_y[2] - lowerlegdist * leg_sin[3];//rightFeet
		leg_x[3] = leg_x[2] - lowerlegdist * leg_cos[3];

		if (hl > hr) { height = hl - 1.57 + 2; if (standing_on_left) { float step = old_feetX - leg_x[1]; this->pos += glm::vec3(cos(rotation.y * PI / 180) * step, 0, sin(rotation.y * PI / -180) * step); } old_feetX = leg_x[1]; old_feetY = leg_y[1]; leg_angles[2] = 0; standing_on_left = true; }
		else { height = hr - 1.57 + 2; if (!standing_on_left) { float step = old_feetX - leg_x[3]; this->pos += glm::vec3(cos(rotation.y * PI / 180) * step, 0, sin(rotation.y * PI / -180) * step); } old_feetX = leg_x[3]; old_feetY = leg_y[3]; leg_angles[5] = 0; standing_on_left = false; }
	}
	inline void DrawOG(Vertex*& vertit, glm::vec3* normal1, glm::vec3* normal2, glm::vec3* next_normal1, glm::vec3* next_normal2, glm::vec3* point1, glm::vec3* point2, glm::vec3* next_point1, glm::vec3* next_point2, float texX, float texY, float texX2, float texY2)
	{
		/*glTexCoord2f(texX, texY);	glNormal3fv((GLfloat*)normal1);		  glVertex3f(point1->x, point1->y, point1->z);
		glTexCoord2f(texX2, texY);	glNormal3fv((GLfloat*)next_normal1);  glVertex3f(next_point1->x, next_point1->y, next_point1->z);
		glTexCoord2f(texX, texY2);	glNormal3fv((GLfloat*)normal2);		  glVertex3f(point2->x, point2->y, point2->z);

		glTexCoord2f(texX, texY2);	glNormal3fv((GLfloat*)normal2);       glVertex3f(point2->x, point2->y, point2->z);
		glTexCoord2f(texX2, texY);	glNormal3fv((GLfloat*)next_normal1);  glVertex3f(next_point1->x, next_point1->y, next_point1->z);
		glTexCoord2f(texX2, texY2);	glNormal3fv((GLfloat*)next_normal2);  glVertex3f(next_point2->x, next_point2->y, next_point2->z);*/
		*vertit++ = { .pos = glm::vec3(point1->x, point1->y, point1->z), .tex = glm::vec2(texX, texY), .norm = *normal1 };
		*vertit++ = { .pos = glm::vec3(next_point1->x, next_point1->y, next_point1->z), .tex = glm::vec2(texX2, texY), .norm = *next_normal1 };
		*vertit++ = { .pos = glm::vec3(point2->x, point2->y, point2->z), .tex = glm::vec2(texX, texY2), .norm = *normal2 };

		*vertit++ = { .pos = glm::vec3(point2->x, point2->y, point2->z), .tex = glm::vec2(texX, texY2), .norm = *normal2 };
		*vertit++ = { .pos = glm::vec3(next_point1->x, next_point1->y, next_point1->z), .tex = glm::vec2(texX2, texY), .norm = *next_normal1 };
		*vertit++ = { .pos = glm::vec3(next_point2->x, next_point2->y, next_point2->z), .tex = glm::vec2(texX2, texY2), .norm = *next_normal2 };
	}
	void DrawFeet(glm::vec3 position, float rotation) {
		const glm::vec3 middletoeend(6, 0, 0); const glm::vec3 middletoenear(1.5, 0, 1.0);
		const glm::vec3 lefttoeend(4.3, 0, 1.7); const glm::vec3 lefttoenear(0, 0, 1.6);
		const glm::vec3 backtoeend(-2.5, 0, 0);

		View::PushMatrix(); //glPushMatrix();
		View::TranslateModel(glm::vec3(position.x, position.y, position.z)); //glTranslatef(position.x, position.y, position.z);
		View::RotateModel(rotation, glm::vec3(0, 0, 1));//glRotatef(rotation, 0, 0, 1);
		View::ScaleModel(glm::vec3(0.1, 0.1, 0.1)); //glScalef(0.1, 0.1, 0.1);
		View::UpdateModel(); //glBegin(GL_TRIANGLES);
		if (!foot_vbuf.has_value()) {
			const size_t vertexCount = 9 + 7 + 10;
			Vertex verts[vertexCount];
			Vertex* vertit = verts;
			*vertit++ = { .pos = glm::vec3(middletoeend.x, 0, 0), .tex = glm::vec2(0,0), .norm = glm::vec3(0, -1, 0) };
			*vertit++ = { .pos = glm::vec3(middletoenear.x, 0, middletoenear.z), .tex = glm::vec2(0,0), .norm = glm::vec3(0, -1, 0) };
			*vertit++ = { .pos = glm::vec3(middletoenear.x, 0, -1 * middletoenear.z), .tex = glm::vec2(0,0), .norm = glm::vec3(0, -1, 0) };

			*vertit++ = { .pos = glm::vec3(middletoenear.x, 0, middletoenear.z), .tex = glm::vec2(0,0), .norm = glm::vec3(0, -1, 0) };
			*vertit++ = { .pos = glm::vec3(lefttoeend.x, 0, lefttoeend.z), .tex = glm::vec2(0,0), .norm = glm::vec3(0, -1, 0) };
			*vertit++ = { .pos = glm::vec3(lefttoenear.x, 0, lefttoenear.z), .tex = glm::vec2(0,0), .norm = glm::vec3(0, -1, 0) };

			*vertit++ = { .pos = glm::vec3(middletoenear.x, 0, middletoenear.z * -1), .tex = glm::vec2(0,0), .norm = glm::vec3(0, -1, 0) };
			*vertit++ = { .pos = glm::vec3(lefttoeend.x, 0, lefttoeend.z * -1), .tex = glm::vec2(0,0), .norm = glm::vec3(0, -1, 0) };
			*vertit++ = { .pos = glm::vec3(lefttoenear.x, 0, lefttoenear.z * -1), .tex = glm::vec2(0,0), .norm = glm::vec3(0, -1, 0) };

			*vertit++ = { .pos = glm::vec3(0, 0, 0), .tex = glm::vec2(0,0), .norm = glm::vec3(0, -1, 0) };
			*vertit++ = { .pos = glm::vec3(middletoenear.x, 0, -1 * middletoenear.z), .tex = glm::vec2(0,0), .norm = glm::vec3(0, -1, 0) };
			*vertit++ = { .pos = glm::vec3(middletoenear.x, 0, middletoenear.z), .tex = glm::vec2(0,0), .norm = glm::vec3(0, -1, 0) };
			*vertit++ = { .pos = glm::vec3(lefttoenear.x, 0, lefttoenear.z), .tex = glm::vec2(0,0), .norm = glm::vec3(0, -1, 0) };
			*vertit++ = { .pos = glm::vec3(backtoeend.x, 0, 0), .tex = glm::vec2(0,0), .norm = glm::vec3(0, -1, 0) };
			*vertit++ = { .pos = glm::vec3(lefttoenear.x, 0, -1 * lefttoenear.z), .tex = glm::vec2(0,0), .norm = glm::vec3(0, -1, 0) };
			*vertit++ = { .pos = glm::vec3(middletoenear.x, 0, -1 * middletoenear.z), .tex = glm::vec2(0,0), .norm = glm::vec3(0, -1, 0) };

			*vertit++ = { .pos = glm::vec3(0, 1.0, 0), .tex = glm::vec2(0,0), .norm = glm::vec3(0, 1, 0) };
			*vertit++ = { .pos = glm::vec3(middletoenear.x, 0, -1 * middletoenear.z), .tex = glm::vec2(0,0), .norm = glm::vec3(middletoenear.x, 0.2, -1 * middletoenear.z) };
			*vertit++ = { .pos = glm::vec3(middletoeend.x, 0, 0), .tex = glm::vec2(0,0), .norm = glm::vec3(middletoeend.x, 0.2, 0) };
			*vertit++ = { .pos = glm::vec3(middletoenear.x, 0, middletoenear.z), .tex = glm::vec2(0,0), .norm = glm::vec3(middletoenear.x, 0.2, middletoenear.z) };
			*vertit++ = { .pos = glm::vec3(lefttoeend.x, 0, lefttoeend.z), .tex = glm::vec2(0,0), .norm = glm::vec3(lefttoeend.x, 0.2, lefttoeend.z) };
			*vertit++ = { .pos = glm::vec3(lefttoenear.x, 0, lefttoenear.z), .tex = glm::vec2(0,0), .norm = glm::vec3(lefttoenear.x, 0.2, lefttoenear.z) };
			*vertit++ = { .pos = glm::vec3(backtoeend.x, 0, 0), .tex = glm::vec2(0,0), .norm = glm::vec3(backtoeend.x, 0.2, 0) };
			*vertit++ = { .pos = glm::vec3(lefttoenear.x, 0, lefttoenear.z * -1), .tex = glm::vec2(0,0), .norm = glm::vec3(lefttoenear.x, 0.2, lefttoenear.z * -1) };
			*vertit++ = { .pos = glm::vec3(lefttoeend.x, 0, lefttoeend.z * -1), .tex = glm::vec2(0,0), .norm = glm::vec3(lefttoeend.x, 0.2, lefttoeend.z * -1) };
			*vertit++ = { .pos = glm::vec3(middletoenear.x, 0, -1 * middletoenear.z), .tex = glm::vec2(0,0), .norm = glm::vec3(middletoenear.x, 0.2, -1 * middletoenear.z) };

			foot_vbuf.emplace(verts, vertexCount);
		}
		/*glNormal3f(0, -1, 0);
		glVertex3f(middletoeend.x, 0, 0);   glVertex3f(middletoenear.x, 0, middletoenear.z);   glVertex3f(middletoenear.x, 0, -1 * middletoenear.z);
		glVertex3f(middletoenear.x, 0, middletoenear.z);   glVertex3f(lefttoeend.x, 0, lefttoeend.z);    glVertex3f(lefttoenear.x, 0, lefttoenear.z);
		glVertex3f(middletoenear.x, 0, middletoenear.z * -1); glVertex3f(lefttoeend.x, 0, lefttoeend.z * -1); glVertex3f(lefttoenear.x, 0, lefttoenear.z * -1);
		glEnd();
		glBegin(GL_TRIANGLE_FAN);
		glVertex3f(0, 0, 0); glVertex3f(middletoenear.x, 0, -1 * middletoenear.z);             glVertex3f(middletoenear.x, 0, middletoenear.z);
		glVertex3f(lefttoenear.x, 0, lefttoenear.z); glVertex3f(backtoeend.x, 0, 0); glVertex3f(lefttoenear.x, 0, -1 * lefttoenear.z);
		glVertex3f(middletoenear.x, 0, -1 * middletoenear.z);
		glEnd();
		glBegin(GL_TRIANGLE_FAN);
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
		glEnd();*/
		foot_vbuf.value().Draw(GL_TRIANGLES, 0, 9);
		foot_vbuf.value().Draw(GL_TRIANGLE_FAN, 9, 7);
		foot_vbuf.value().Draw(GL_TRIANGLE_FAN, 9+7, 10);
		View::PopMatrix(); //glPopMatrix();
	}

	void Draw(bool isShadow = false) {
		float leg_y[4], leg_x[4]; //leftMiddle,leftFeet,rightMiddle,rightFeet
		CalculateLegArray(leg_x, leg_y); //height beállítás miatt elõrehozva

		View::ResetTransform();
		/*glTranslatef(pos.x, (isShadow) ? (pos.y + 0.001f) : (pos.y + height), pos.z);
		glRotatef(rotation.x, 1, 0, 0);
		glRotatef(rotation.y, 0, 1, 0);
		glRotatef(rotation.z, 0, 0, 1);*/
		View::TranslateModel(glm::vec3(pos.x, (isShadow) ? (pos.y + 0.001f) : (pos.y + height), pos.z));
		View::RotateModel(rotation.x, glm::vec3(1, 0, 0));
		View::RotateModel(rotation.y, glm::vec3(0, 1, 0));
		View::RotateModel(rotation.z, glm::vec3(0, 0, 1));

		if (isShadow) { View::ScaleModel(glm::vec3(1, 0, 1)); Material::shadow.Setup(); }
		else { Texture::golya_.Setup(); }
		spine.TCatmullRom();
		body.TCatmullRom();

		static glm::vec3 buffer[160];
		glm::vec3* circle = buffer, * nextcircle = buffer + 32, * nextnextcircle = buffer + 128;
		glm::vec3* normals = buffer + 64, * nextnormals = buffer + 96;
		float angle = 0, sin_[32], cos_[32], texY[32], texX, texX2;
		glm::vec3 left(0, 0, -1), up = glm::normalize(glm::cross(left, glm::normalize(spine.points[1] - spine.points[0])));

		for (int j = 0; j < 32; j++, angle += (2 * PI) / ((float)32)) {
			sin_[j] = sin(angle);
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

		//glBegin(GL_TRIANGLES);
		View::UpdateModel();
		size_t vertexCount = (spine.p_count - 1) * 32 * 6;
		Vertex* verts = new Vertex[vertexCount];
		Vertex* vertit = verts;

		for (int i = 1; i < spine.p_count; i++)
		{
			if (i == spine.p_count - 1) nextnormals[0] = nextcircle[0] - spine.points[i];
			else {
				up = glm::normalize(glm::cross(left, glm::normalize(spine.points[i + 1] - spine.points[i])));
				nextnextcircle[0] = spine.points[i + 1] + up * (sin_[0] * body.points[i + 1].x) + left * (cos_[0] * body.points[i + 1].y);
				nextnormals[0] = glm::cross(glm::cross((nextnextcircle[0] - circle[0]), (nextcircle[0] - spine.points[i])), (nextnextcircle[0] - circle[0]));
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
					nextnormals[j] = glm::cross(glm::cross((nextnextcircle[j] - circle[j]), (nextcircle[j] - spine.points[i])), (nextnextcircle[j] - circle[j]));
				}

				DrawOG(vertit, normals + j - 1, normals + j, nextnormals + j - 1, nextnormals + j, circle + j - 1, circle + j, nextcircle + j - 1, nextcircle + j, texX, texY[j - 1], texX2, texY[j]);
			}
			DrawOG(vertit, normals + 31, normals + 0, nextnormals + 31, nextnormals + 0, circle + 31, circle + 0, nextcircle + 31, nextcircle + 0, texX, texY[31], texX2, texY[0]);

			glm::vec3* tmp = circle;   circle = nextcircle;   nextcircle = nextnextcircle;  nextnextcircle = tmp;
			tmp = normals; normals = nextnormals; nextnormals = tmp;

			texX = texX2; texX2 += 1 / (spine.p_count - 1.0f);
		}
		//glEnd();
		if (body_vbuf.has_value()) body_vbuf.value().Update(verts, vertexCount);
		else body_vbuf.emplace(verts, vertexCount);
		body_vbuf.value().Draw();
		delete[] verts;

		if (!isShadow) {
			eye[0].pos = spine.points[98] + up * (0.7f * sin_[29] * ((27 >= 16) ? (body.points[98].x) : (body.points[98].z))) + left * (0.7f * cos_[29] * body.points[98].y);
			eye[0].RecursiveDraw();
			eye[1].pos = spine.points[98] + up * (0.7f * sin_[19] * ((18 >= 16) ? (body.points[98].x) : (body.points[98].z))) + left * (0.7f * cos_[19] * body.points[98].y);
			eye[1].RecursiveDraw();
		}

		float radius = body.cpoints[6].x;
		up = glm::normalize(glm::cross(left, glm::normalize((glm::vec3)(spine.cpoints[7]) - (glm::vec3)(spine.cpoints[6]))));

		{ // Beak drawing. TODO: move it to a method.
			if (!isShadow) Material::golya_beak.Setup();
			//glBegin(GL_TRIANGLES);
			size_t vertexCount = 17 * 3 + 16 * 3 + 3;
			Vertex* verts = new Vertex[vertexCount];
			Vertex* vertit = verts;
			
			glm::vec3 dir[2]; dir[1] = left * cos_[16] * radius + up * sin_[16] * radius;
			glm::vec3 b[2];   b[1] = beak_start + dir[1];       glm::vec3 end_normal;

			for (int j = 0; j <= 16; j++) {
				dir[j & 1] = left * cos_[j] * radius + up * sin_[j] * radius;
				b[j & 1] = beak_start + dir[j & 1];

				//glNormal3f(dir[0].x, dir[0].y, dir[0].z);
				//glVertex3f(b[0].x, b[0].y, b[0].z);
				*vertit++ = { .pos = glm::vec3(b[0].x, b[0].y, b[0].z), .tex = glm::vec2(0,0), .norm = glm::vec3(dir[0].x, dir[0].y, dir[0].z) };
				end_normal = (dir[0] + dir[1]) / 2.f;
				//glNormal3f(dir[1].x, dir[1].y, dir[1].z);
				//glVertex3f(b[1].x, b[1].y, b[1].z);
				*vertit++ = { .pos = glm::vec3(b[1].x, b[1].y, b[1].z), .tex = glm::vec2(0,0), .norm = glm::vec3(dir[1].x, dir[1].y, dir[1].z) };
				//glNormal3f(end_normal.x, end_normal.y, end_normal.z);
				//glVertex3f(beak_end[0].x, beak_end[0].y, beak_end[0].z);
				*vertit++ = { .pos = glm::vec3(beak_end[0].x, beak_end[0].y, beak_end[0].z), .tex = glm::vec2(0,0), .norm = glm::vec3(end_normal.x, end_normal.y, end_normal.z) };
			}
			dir[1] = left * cos_[0] * radius + up * sin_[0] * radius;
			b[1] = beak_start + dir[1];
			for (int j = 16; j < 32; j++) {
				dir[j & 1] = left * cos_[j] * radius + up * sin_[j] * radius;
				b[j & 1] = beak_start + dir[j & 1];

				//glNormal3f(dir[0].x, dir[0].y, dir[0].z);
				//glVertex3f(b[0].x, b[0].y, b[0].z);
				*vertit++ = { .pos = glm::vec3(b[0].x, b[0].y, b[0].z), .tex = glm::vec2(0,0), .norm = glm::vec3(dir[0].x, dir[0].y, dir[0].z) };
				end_normal = (dir[0] + dir[1]) / 2.f;
				//glNormal3f(dir[1].x, dir[1].y, dir[1].z);
				//glVertex3f(b[1].x, b[1].y, b[1].z);
				*vertit++ = { .pos = glm::vec3(b[1].x, b[1].y, b[1].z), .tex = glm::vec2(0,0), .norm = glm::vec3(dir[1].x, dir[1].y, dir[1].z) };
				//glNormal3f(end_normal.x, end_normal.y, end_normal.z);
				//glVertex3f(beak_end[1].x, beak_end[1].y, beak_end[1].z);
				*vertit++ = { .pos = glm::vec3(beak_end[1].x, beak_end[1].y, beak_end[1].z), .tex = glm::vec2(0,0), .norm = glm::vec3(end_normal.x, end_normal.y, end_normal.z) };
			}
			dir[0] = left * cos_[0] * radius + up * sin_[0] * radius;
			b[0] = beak_start + dir[0];

			//glNormal3f(dir[0].x, dir[0].y, dir[0].z);
			//glVertex3f(b[0].x, b[0].y, b[0].z);
			*vertit++ = { .pos = glm::vec3(b[0].x, b[0].y, b[0].z), .tex = glm::vec2(0,0), .norm = glm::vec3(dir[0].x, dir[0].y, dir[0].z) };
			end_normal = (dir[0] + dir[1]) / 2.f;
			//glNormal3f(dir[1].x, dir[1].y, dir[1].z);
			//glVertex3f(b[1].x, b[1].y, b[1].z);
			*vertit++ = { .pos = glm::vec3(b[1].x, b[1].y, b[1].z), .tex = glm::vec2(0,0), .norm = glm::vec3(dir[1].x, dir[1].y, dir[1].z) };
			//glNormal3f(end_normal.x, end_normal.y, end_normal.z);
			//glVertex3f(beak_end[1].x, beak_end[1].y, beak_end[1].z);
			*vertit++ = { .pos = glm::vec3(beak_end[1].x, beak_end[1].y, beak_end[1].z), .tex = glm::vec2(0,0), .norm = glm::vec3(end_normal.x, end_normal.y, end_normal.z) };

			//glEnd();
			View::UpdateModel();
			if (beak_vbuf.has_value()) beak_vbuf.value().Update(verts, vertexCount);
			else beak_vbuf.emplace(verts, vertexCount);
			beak_vbuf.value().Draw();
			delete[] verts;
		}

		if (isShadow) return;
		//0,0,0(áll);  -60,100,-60(hátrahúz);      5,20,0(elõre_lerak)

		float middleX, middleY, feetX, feetY;
		middleX = leg_x[0]; middleY = leg_y[0];
		feetX = leg_x[1];
		feetY = leg_y[1];

		View::UpdateModel();
		MultiCylinder::Draw(glm::vec3(middleX, middleY, -upperlegZ), glm::vec3(upperlegX, upperlegY, -upperlegZ), leg_rad_count, leg_rads);
		MultiCylinder::Draw(glm::vec3(feetX, feetY, -upperlegZ), glm::vec3(middleX, middleY, -upperlegZ), leg_rad_count, leg_rads);
		View::PushMatrix(); //glPushMatrix();
		View::TranslateModel(glm::vec3(middleX, middleY, -upperlegZ)); // glTranslatef(middleX, middleY, -upperlegZ);
		View::ScaleModel(glm::vec3(0.115, 0.115, 0.115)); // glScalef(0.115, 0.115, 0.115);
		View::UpdateModel();
		Ellipsoid::DrawSphere(20);
		View::PopMatrix(); // glPopMatrix();
		DrawFeet(glm::vec3(feetX, feetY, -0.25), leg_angles[2]);

		middleX = leg_x[2]; middleY = leg_y[2];
		feetX = leg_x[3];
		feetY = leg_y[3];

		View::UpdateModel();
		MultiCylinder::Draw(glm::vec3(middleX, middleY, upperlegZ), glm::vec3(upperlegX, upperlegY, upperlegZ), leg_rad_count, leg_rads);
		MultiCylinder::Draw(glm::vec3(feetX, feetY, upperlegZ), glm::vec3(middleX, middleY, upperlegZ), leg_rad_count, leg_rads);
		View::PushMatrix(); //glPushMatrix();
		View::TranslateModel(glm::vec3(middleX, middleY, upperlegZ)); //glTranslatef(middleX, middleY, upperlegZ);
		View::ScaleModel(glm::vec3(0.115, 0.115, 0.115)); // glScalef(0.115, 0.115, 0.115);
		View::UpdateModel();
		Ellipsoid::DrawSphere(20);
		View::PopMatrix(); // glPopMatrix();
		DrawFeet(glm::vec3(feetX, feetY, upperlegZ), leg_angles[5]);

		if (!isShadow) this->Draw(true);
	}
	void SetBeakAngle(float angle) {
		glm::vec3 beakEnd = (glm::vec3)(spine.cpoints[7]);     beak_angle = angle; //4:neck end
		beak_start = (glm::vec3)(spine.cpoints[6]);
		beak_end[1] = Rotate2D(beakEnd, beak_start, angle);
		beak_end[0] = Rotate2D(beakEnd, beak_start, angle * -1);
	}
	void SetHeadAngle(float angle, float beakangle) {
		glm::vec3 base = (glm::vec3)(spine.cpoints[4]);            head_angle = angle;
		spine.cpoints[5] = glm::vec4(Rotate2D((glm::vec3)(spine.cpoints[5]),base, angle), spine.cpoints[5].w);
		spine.cpoints[6] = glm::vec4(Rotate2D((glm::vec3)(spine.cpoints[6]),base, angle), spine.cpoints[6].w);
		spine.cpoints[7] = glm::vec4(Rotate2D((glm::vec3)(spine.cpoints[7]),base, angle), spine.cpoints[7].w);
		spine.points_valid = false;               SetBeakAngle(beakangle);
	}
	void SetHeadAngle(float angle) { SetHeadAngle(angle, beak_angle); }
	void SetNeckAngle(float angle, float headangle, float beakangle) {
		neck_upper_angle = angle;
		neck_lower_angle = angle / 2;
		float body_rotation = angle / 10;

		glm::vec3 base(upperlegX, upperlegY, 0); //leg point 
		spine.cpoints[0] = glm::vec4(Rotate2D((glm::vec3)(spine_vals[0]), base, body_rotation), spine.cpoints[0].w);
		spine.cpoints[1] = glm::vec4(Rotate2D((glm::vec3)(spine_vals[1]),base, body_rotation), spine.cpoints[1].w);
		spine.cpoints[2] = glm::vec4(Rotate2D((glm::vec3)(spine_vals[2]),base, body_rotation), spine.cpoints[2].w);
		spine.cpoints[3] = glm::vec4(Rotate2D((glm::vec3)(spine_vals[3]),base, body_rotation), spine.cpoints[3].w);
		spine.cpoints[4] = glm::vec4(Rotate2D((glm::vec3)(spine_vals[4]),base, body_rotation), spine.cpoints[4].w);
		spine.cpoints[5] = glm::vec4(Rotate2D((glm::vec3)(spine_vals[5]),base, body_rotation), spine.cpoints[5].w);
		spine.cpoints[6] = glm::vec4(Rotate2D((glm::vec3)(spine_vals[6]),base, body_rotation), spine.cpoints[6].w);
		spine.cpoints[7] = glm::vec4(Rotate2D((glm::vec3)(spine_vals[7]),base, body_rotation), spine.cpoints[7].w);

		base = (((glm::vec3)(spine.cpoints[2])) + ((glm::vec3)(spine.cpoints[3])) * 4.f) / 5.f;
		spine.cpoints[3] = glm::vec4(Rotate2D((glm::vec3)(spine.cpoints[3]),base, neck_lower_angle), spine.cpoints[3].w);
		spine.cpoints[4] = glm::vec4(Rotate2D((glm::vec3)(spine.cpoints[4]),base, neck_lower_angle), spine.cpoints[4].w);
		spine.cpoints[5] = glm::vec4(Rotate2D((glm::vec3)(spine.cpoints[5]),base, neck_lower_angle), spine.cpoints[5].w);
		spine.cpoints[6] = glm::vec4(Rotate2D((glm::vec3)(spine.cpoints[6]),base, neck_lower_angle), spine.cpoints[6].w);
		spine.cpoints[7] = glm::vec4(Rotate2D((glm::vec3)(spine.cpoints[7]),base, neck_lower_angle), spine.cpoints[7].w);

		base = (((glm::vec3)(spine.cpoints[2])) + ((glm::vec3)(spine.cpoints[3])) * 4.f) / 5.f;
		spine.cpoints[4] = glm::vec4(Rotate2D((glm::vec3)(spine.cpoints[4]),base, neck_upper_angle), spine.cpoints[4].w);
		spine.cpoints[5] = glm::vec4(Rotate2D((glm::vec3)(spine.cpoints[5]),base, neck_upper_angle), spine.cpoints[5].w);
		spine.cpoints[6] = glm::vec4(Rotate2D((glm::vec3)(spine.cpoints[6]),base, neck_upper_angle), spine.cpoints[6].w);
		spine.cpoints[7] = glm::vec4(Rotate2D((glm::vec3)(spine.cpoints[7]),base, neck_upper_angle), spine.cpoints[7].w);
		spine.points_valid = false;
		if (angle < -56) { headangle = -56 - angle; }
		else { headangle = 0; }
		SetHeadAngle(headangle, beakangle);
	}
	glm::vec3 GetStandingLeg() {
		glm::vec3 feet(old_feetX, SalamiGolya::upperlegZ, 0); if (!standing_on_left)feet.y *= -1;
		feet = Rotate2D(feet, glm::vec3(0, 0, 0), rotation.y);
		feet.z = feet.y; feet.y = old_feetY;
		return feet;
	}

};

class MovingGolya : public SalamiGolya {
	static TCRSpline walking;
	static bool loaded;
	static float walk_phase_delay;
	static glm::vec3 standing_main;
	static glm::vec3 standing_float;

	float t, headangle; float beak_attack1; bool walk;
public:
	MovingGolya() :t(0), headangle(0), walk(false), beak_attack1(0) {
		if (!loaded) {//0,0,0(áll);  -60,100,-60(hátrahúz);      5,20,0(elõre_lerak)
			walking.AddCPoint(PointW(-30, 20, 0, 0.4));
			walking.AddCPoint(PointW(-60, 100, -60, 0.5));
			walking.AddCPoint(PointW(-30, 110, -65, 0.5));
			walking.AddCPoint(PointW(5, 20, 0, 1));//   -+---------------------
			walking.AddCPoint(PointW(-30, 20, 0, 1));//    \-kettõ között: (0,0,0)

			walk_phase_delay = walking.SumWeight() / 2;

			standing_main = glm::vec3(0, 0, 0);
			standing_float = glm::vec3(-50, 95, -60);
		}
	}

	void Animate(float deltat) {
		if (walk) {
			float walktsum = walking.SumWeight();
			while (t > walktsum)t -= walktsum;
			glm::vec3 angles = walking.TCatmullRomVect(t);

			leg_angles[0] = angles.x; leg_angles[1] = angles.y; leg_angles[2] = angles.z;

			float otherleg = t + walk_phase_delay;
			while (otherleg > walktsum)otherleg -= walktsum;
			angles = walking.TCatmullRomVect(otherleg);

			leg_angles[3] = angles.x; leg_angles[4] = angles.y; leg_angles[5] = angles.z;

			this->SetNeckAngle(cos(4 * PI * t / walktsum) * (-10), cos(4 * PI * t / walktsum) * 10, 0);

			float leg_y[4], leg_x[4];
			CalculateLegArray(leg_x, leg_y);

			t += deltat * 0.02; headangle += deltat * 0.1; if (headangle > 30) { headangle = -20; }
		}
		if (beak_attack1 > 0) {
			if (beak_attack1 < 50)
				this->SetNeckAngle(-1 * beak_attack1, 0, 0);
			else if (beak_attack1 < 64)
				this->SetNeckAngle(-1 * beak_attack1, 0, beak_attack1 - 50);
			else if (beak_attack1 < 78)
				this->SetNeckAngle(-64, 0, 14 + 64 - beak_attack1);
			else this->SetNeckAngle(-64 - 78 + beak_attack1, 0, 0);

			beak_attack1 += deltat * (1.5 - abs((long long)(142 / 2 - beak_attack1) / (142 / 2)));

			if (beak_attack1 >= 142) { this->SetNeckAngle(0, 0, 0); beak_attack1 = 0; }

			glm::vec3 beaks[2]; beaks[0] = Rotate2D(glm::vec3(this->beak_end[0].x, this->beak_end[0].z, 0), glm::vec3(0, 0, 0), rotation.y);
			beaks[0].z = -1 * beaks[0].y; beaks[0].y = this->beak_end[0].y; beaks[0] += this->pos; beaks[0].y += this->height;
			beaks[1] = Rotate2D(glm::vec3(this->beak_end[1].x, this->beak_end[1].z, 0), glm::vec3(0, 0, 0), rotation.y);
			beaks[1].z = -1 * beaks[1].y; beaks[1].y = this->beak_end[1].y; beaks[1] += this->pos; beaks[1].y += this->height;
			for (int i = 0; i < frog_count; i++) {
				if (glm::distance(beaks[0], frogs[i].pos) < frogs[i].size || glm::distance(beaks[1],frogs[i].pos) < frogs[i].size) {
					frogs[i] = frogs[frog_count - 1]; frog_count--;
				}
			}
		}

	}
	void StartBeakAttack1() { beak_attack1 += 0.001f; }
	void StartWalking() { this->walk = true; }
	void StopWalking() { this->walk = false; }
	void RotateLeft(float angle) {
		this->rotation.y += angle;
		glm::vec3 leg = this->GetStandingLeg(); leg.y = leg.z;
		glm::vec3 newleg = Rotate2D(leg, glm::vec3(0, 0, 0), angle);
		glm::vec3 offset = newleg - leg;
		this->pos.x -= offset.x; this->pos.z += offset.y;
	}
	void RotateRight(float angle) {
		this->rotation.y -= angle;
		glm::vec3 leg = this->GetStandingLeg(); leg.y = leg.z;
		glm::vec3 newleg = Rotate2D(leg, glm::vec3(0, 0, 0), -angle);
		glm::vec3 offset = newleg - leg;
		this->pos.x -= offset.x; this->pos.z += offset.y;
	}
};