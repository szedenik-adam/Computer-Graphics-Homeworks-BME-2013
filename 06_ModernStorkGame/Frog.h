#pragma once

#include <glm/glm.hpp>
#include <unordered_map>
#include "Plane.h"
#include "Texture.h"

#define PI 3.14159265358979323846f

class Ellipsoid : public Object
{
	static std::unordered_map<int, VertexBuffer> resToVBuf;
public:
	Color color;
	MaterialBase* material;
	int resolution;

	Ellipsoid() :Object(), color(Color(0.1, 1, 1)), material(0), resolution(30) {}
	Ellipsoid(glm::vec3 pos, glm::vec3 rotation, MaterialBase* mat, float x_radius = 1, float y_radius = 1, float z_radius = 1, int resolution = 30)
		:Object(pos, rotation, glm::vec3(x_radius, y_radius, z_radius)), material(mat), color(Color(0.1, 1, 1)), resolution(resolution) {}

	virtual void LocalDraw(bool isShadow = false)
	{
		if (!material) return;
		if (!isShadow) {
			material->Setup();
			Ellipsoid::DrawSphere(resolution);
		}
		else {
			Material::shadow.Setup();
			Ellipsoid::DrawSphere(10);
		}
	}

	static void DrawSphere(int res = 30)
	{
		size_t vertexCount = ((size_t)res / 2) * res * 6;
		auto search = resToVBuf.find(res);
		if (search == resToVBuf.end()) // No VAO & VBO for this resolution yet.
		{
			Vertex* verts = new Vertex[vertexCount];
			Vertex* vertit = verts;

			float y_angle = 0, old_y = 0;

			//glBegin(GL_TRIANGLES);
			for (int i = 0; i < res / 2; i++)
			{
				float xz_angle = 0, y_offset = cos(y_angle), radius = sin(y_angle), old_xz = 0;
				old_y = y_angle;  y_angle += (2 * PI) / ((float)res);
				float next_y_offset = cos(y_angle), next_radius = sin(y_angle);

				for (int j = 0; j < res; j++)
				{
					glm::vec3 current_point = glm::vec3(cos(xz_angle) * radius, y_offset, sin(xz_angle) * radius);
					glm::vec3 below_point = glm::vec3(cos(xz_angle) * next_radius, next_y_offset, sin(xz_angle) * next_radius);

					old_xz = xz_angle;  xz_angle += (2 * PI) / ((float)res);

					glm::vec3 current_plus1 = glm::vec3(cos(xz_angle) * radius, y_offset, sin(xz_angle) * radius);
					glm::vec3 below_plus1 = glm::vec3(cos(xz_angle) * next_radius, next_y_offset, sin(xz_angle) * next_radius);

					*vertit++ = { .pos = glm::vec3(current_point.x, current_point.y, current_point.z), .tex = glm::vec2(old_y / PI, old_xz / (2 * PI)),   .norm = current_point };
					*vertit++ = { .pos = glm::vec3(current_plus1.x, current_plus1.y, current_plus1.z), .tex = glm::vec2(old_y / PI, xz_angle / (2 * PI)), .norm = current_plus1 };
					*vertit++ = { .pos = glm::vec3(below_point.x, below_point.y, below_point.z),       .tex = glm::vec2(y_angle / PI, old_xz / (2 * PI)), .norm = below_point };

					*vertit++ = { .pos = glm::vec3(current_plus1.x, current_plus1.y, current_plus1.z), .tex = glm::vec2(old_y / PI, xz_angle / (2 * PI)),   .norm = current_plus1 };
					*vertit++ = { .pos = glm::vec3(below_plus1.x, below_plus1.y, below_plus1.z),       .tex = glm::vec2(y_angle / PI, xz_angle / (2 * PI)), .norm = below_plus1 };
					*vertit++ = { .pos = glm::vec3(below_point.x, below_point.y, below_point.z),       .tex = glm::vec2(y_angle / PI, old_xz / (2 * PI)),   .norm = below_point };
					/*if (AddTexCoord) {
						glTexCoord2f(old_y / PI, old_xz / (2 * PI));    glNormal3fv((GLfloat*)&current_point);   glVertex3f(current_point.x, current_point.y, current_point.z);
						glTexCoord2f(old_y / PI, xz_angle / (2 * PI));  glNormal3fv((GLfloat*)&current_plus1);   glVertex3f(current_plus1.x, current_plus1.y, current_plus1.z);
						glTexCoord2f(y_angle / PI, old_xz / (2 * PI));  glNormal3fv((GLfloat*)&below_point);     glVertex3f(below_point.x, below_point.y, below_point.z);

						glTexCoord2f(old_y / PI, xz_angle / (2 * PI));    glNormal3fv((GLfloat*)&current_plus1);   glVertex3f(current_plus1.x, current_plus1.y, current_plus1.z);
						glTexCoord2f(y_angle / PI, xz_angle / (2 * PI));  glNormal3fv((GLfloat*)&below_plus1);     glVertex3f(below_plus1.x, below_plus1.y, below_plus1.z);
						glTexCoord2f(y_angle / PI, old_xz / (2 * PI));    glNormal3fv((GLfloat*)&below_point);     glVertex3f(below_point.x, below_point.y, below_point.z);
					}
					else
						if (AddNormals) {
							glNormal3fv((GLfloat*)&current_point);   glVertex3f(current_point.x, current_point.y, current_point.z);
							glNormal3fv((GLfloat*)&current_plus1);   glVertex3f(current_plus1.x, current_plus1.y, current_plus1.z);
							glNormal3fv((GLfloat*)&below_point);     glVertex3f(below_point.x, below_point.y, below_point.z);

							glNormal3fv((GLfloat*)&current_plus1);   glVertex3f(current_plus1.x, current_plus1.y, current_plus1.z);
							glNormal3fv((GLfloat*)&below_plus1);     glVertex3f(below_plus1.x, below_plus1.y, below_plus1.z);
							glNormal3fv((GLfloat*)&below_point);     glVertex3f(below_point.x, below_point.y, below_point.z);
						}
						else {
							glVertex3f(current_point.x, current_point.y, current_point.z);
							glVertex3f(current_plus1.x, current_plus1.y, current_plus1.z);
							glVertex3f(below_point.x, below_point.y, below_point.z);

							glVertex3f(current_plus1.x, current_plus1.y, current_plus1.z);
							glVertex3f(below_plus1.x, below_plus1.y, below_plus1.z);
							glVertex3f(below_point.x, below_point.y, below_point.z);
						}*/
				}
			}
			//glEnd();
			
			//resToVBuf[res] = VertexBuffer(verts, vertexCount);
			std::pair<std::unordered_map<int, VertexBuffer>::iterator, bool> insertResult = resToVBuf.emplace(std::make_pair(res, VertexBuffer(verts, vertexCount)));
			delete[] verts;
			search = insertResult.first;
		}
		(*search).second.Draw();
	}
};

class Frog
{
	bool needs_animation; glm::vec3 speed;
public:
	Ellipsoid ellipsoids[10];
	glm::vec3 pos, rotation;
	float size;

	Frog() :pos(0, 0, 0), rotation(0, 0, 0), size(1) {
		InitFrogBody();
	}
	Frog(glm::vec3 pos, glm::vec3 rotation, float size) :pos(pos), rotation(rotation), size(size), needs_animation(false), speed(glm::vec3(0, 0, 0)) {
		InitFrogBody();
	}
	void InitFrogBody() {
		ellipsoids[0] = Ellipsoid(glm::vec3(0, 0, 0), glm::vec3(20, 0, 0), &Material::frog_body, 0.7, 0.6, 1.1);
		ellipsoids[1] = Ellipsoid(glm::vec3(0, 0.2, -0.5), glm::vec3(-20, 0, 0), &Material::frog_body, 0.7, 0.6, 1.0);
		ellipsoids[2] = Ellipsoid(glm::vec3(-0.5, 0.6, -0.5), glm::vec3(0, 0, 0), &Material::frog_eye, 0.2, 0.17, 0.1);
		ellipsoids[3] = Ellipsoid(glm::vec3(+0.5, 0.6, -0.5), glm::vec3(0, 0, 0), &Material::frog_eye, 0.2, 0.17, 0.1);

		ellipsoids[4] = Ellipsoid(glm::vec3(-0.7, -0.7, -0.6), glm::vec3(0, 0, 0), &Material::frog_leg, 0.2, 0.7, 0.13);
		ellipsoids[5] = Ellipsoid(glm::vec3(+0.7, -0.7, -0.6), glm::vec3(0, 0, 0), &Material::frog_leg, 0.2, 0.7, 0.13);

		ellipsoids[6] = Ellipsoid(glm::vec3(-0.65, -0.2, +0.6), glm::vec3(0, 40, 0), &Material::frog_leg, 0.4, 0.45, 0.6);
		ellipsoids[7] = Ellipsoid(glm::vec3(+0.65, -0.2, +0.6), glm::vec3(0, -40, 0), &Material::frog_leg, 0.4, 0.45, 0.6);

		for (int i = 0; i < 8; i++) { ellipsoids[i].resolution = 20; }
		ellipsoids[0].resolution = 30;

		ellipsoids[0].AddPart(ellipsoids + 1);
		ellipsoids[1].AddPart(ellipsoids + 2);
		ellipsoids[1].AddPart(ellipsoids + 3);
		ellipsoids[0].AddPart(ellipsoids + 4);
		ellipsoids[0].AddPart(ellipsoids + 5);
		ellipsoids[0].AddPart(ellipsoids + 6);
		ellipsoids[0].AddPart(ellipsoids + 7);
	}
	void Draw(float distance, bool isShadow = false) {
		int res = 5;
		if (distance < 4) res = 30; else if (distance < 7) res = 20; else if (distance < 30) res = 10;
		if (res != ellipsoids[0].resolution) { // a távol lévõ békák felbontását leveszi
			ellipsoids[0].resolution = res;
			res = ((res * 2) / 3) + (!(res & 1)); // végtagok felbontása a törzs felbontásának 2/3-a + párossá kiegészítés
			for (int i = 1; i < 8; i++) { ellipsoids[i].resolution = res; }
		}

		View::ResetTransform();
		View::TranslateModel(glm::vec3(pos.x, (isShadow) ? 0.001 : pos.y, pos.z));
		View::RotateModel(rotation.x, glm::vec3(1, 0, 0));
		View::RotateModel(rotation.y, glm::vec3(0, 1, 0));
		View::RotateModel(rotation.z, glm::vec3(0, 0, 1));
		View::ScaleModel(glm::vec3(size, (isShadow) ? 0 : size, size));

		ellipsoids[0].RecursiveDraw(isShadow);
		if (!isShadow) Draw(distance, true);
	}
	void JumpAway(glm::vec3 from) {
		from.y = pos.y; // egy síkon való elugrás
		glm::vec3 jump_dir = pos - from; glm::normalize(jump_dir); jump_dir.y = ((rand() & 31) + 15) / 64.f + 1;
		jump_dir = glm::normalize(jump_dir) * 2.f;
		needs_animation = true; 
		speed = jump_dir * (1.0f + (rand() & 31) / 31.f);
	}
	void Animate(float deltat) {
		if (needs_animation) {
			float tstep;
			while (deltat > 0) {
				if (deltat > 0.04) { tstep = 0.04; deltat -= 0.04; }
				else { tstep = deltat; deltat = 0; }
				pos += speed * tstep;
				speed.y -= tstep * 1.8;
				if (pos.y < 0.22) { pos.y = 0.22; speed = glm::vec3(0, 0, 0); needs_animation = false; }
			}
		}
	}
};