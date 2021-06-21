#pragma once

#include <glm/glm.hpp>
#include "Texture.h"
#include "View.h"
#include <optional>

class Object
{
protected:
	Object* parts[8]; int part_count;
public:
	glm::vec3 pos, rotation, scale;

	Object() :part_count(0), pos(0, 0, 0), rotation(0, 0, 0), scale(1, 1, 1) {}
	Object(glm::vec3 pos, glm::vec3 rotation, glm::vec3 scale) :part_count(0), pos(pos), rotation(rotation), scale(scale) {}

	void Draw(bool isShadow = false) {
		View::ResetTransform();
		RecursiveDraw(isShadow);
	}
	void RecursiveDraw(bool isShadow = false) {
		/*glPushMatrix();

		glTranslatef(pos.x, pos.y, pos.z);
		glRotatef(rotation.x, 1, 0, 0);
		glRotatef(rotation.y, 0, 1, 0);
		glRotatef(rotation.z, 0, 0, 1);
		glScalef(scale.x, scale.y, scale.z);*/
		View::PushMatrix();
		View::TranslateModel(pos);
		View::RotateModel(rotation.x, glm::vec3(1, 0, 0));
		View::RotateModel(rotation.y, glm::vec3(0, 1, 0));
		View::RotateModel(rotation.z, glm::vec3(0, 0, 1));
		View::ScaleModel(scale);
		View::UpdateModel();

		LocalDraw(isShadow);

		for (int i = 0; i < part_count; i++) parts[i]->RecursiveDraw(isShadow);

		//glPopMatrix();
		View::PopMatrix();
	}
	virtual void LocalDraw(bool isShadow = false) = 0;

	void AddPart(Object* part) {
		if (part_count < 8) { parts[part_count] = part; part_count++; }
	}
};

class Plane : public Object
{
public:
	glm::vec3 pos, up; float size_x, size_z, quad_size_x, quad_size_z;
	MaterialBase* material;
	std::optional<VertexBuffer> vbuf; //unsigned int VAO, VBO, vertexCount;


	Plane() :material(0) {}
	Plane(glm::vec3 pos, glm::vec3 up, float size_x, float size_z, float quad_size_x = 1, float quad_size_z = 1)
		:Object(pos, glm::vec3(0, 0, 0), glm::vec3(1, 1, 1)),
		 up(up), size_x(size_x), size_z(size_z), quad_size_x(quad_size_x), quad_size_z(quad_size_z), material(0)/*, VAO(-1), VBO(-1), vertexCount(0)*/ {}

	virtual void LocalDraw(bool isShadow = false)
	{
		if (!material) { return; }
		material->Setup();

		if (!vbuf.has_value())
		{

			size_t xCnt = (size_t)(size_x / quad_size_x);
			size_t zCnt = (size_t)(size_z / quad_size_z);
			Vertex* verts = new Vertex[xCnt * zCnt * 6];
			size_t vertexCount = xCnt * zCnt * 6;

			//glBegin(GL_TRIANGLES);
			//glNormal3fv((GLfloat*)&up);
			Vertex* vertit = verts;
			float x = 0, newx;
			for (int i = 0; i < xCnt; i++, x = newx)
			{
				float z = 0, newz; newx = x + quad_size_x;
				for (int j = 0; j < zCnt; j++, z = newz)
				{
					newz = z + quad_size_z;
#if 0
					glTexCoord2f(0, 0);  glVertex3f(x, 0, z);
					glTexCoord2f(1, 0);  glVertex3f(newx, 0, z);
					glTexCoord2f(1, 1);  glVertex3f(newx, 0, newz);

					glTexCoord2f(0, 0);  glVertex3f(x, 0, z);
					glTexCoord2f(1, 1);  glVertex3f(newx, 0, newz);
					glTexCoord2f(0, 1);  glVertex3f(x, 0, newz);
#endif
					*vertit++ = { .pos = glm::vec3(x, 0, z),   .tex = glm::vec2(0, 0), .norm = up };
					*vertit++ = { .pos = glm::vec3(newx, 0, z), .tex = glm::vec2(1, 0), .norm = up };
					*vertit++ = { .pos = glm::vec3(newx, 0, newz), .tex = glm::vec2(1, 1), .norm = up };

					*vertit++ = { .pos = glm::vec3(x, 0, z),   .tex = glm::vec2(0, 0), .norm = up };
					*vertit++ = { .pos = glm::vec3(newx, 0, newz), .tex = glm::vec2(1, 1), .norm = up };
					*vertit++ = { .pos = glm::vec3(x, 0, newz), .tex = glm::vec2(0, 1), .norm = up };
				}
			}
			for (int i = 0; i < 3; i++) printf("%.2f,%.2f,%.2f    %.2f,%.2f    %.2f,%.2f,%.2f\n", verts[i].pos.x, verts[i].pos.y, verts[i].pos.z, verts[i].tex.x, verts[i].tex.y, verts[i].norm.x, verts[i].norm.y, verts[i].norm.z);
			//glEnd();

			vbuf = VertexBuffer(verts, vertexCount);

			delete[] verts;
		}
		// Draw Vectices.
		vbuf.value().Draw();
	}
};