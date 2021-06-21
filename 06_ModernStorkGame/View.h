#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "shader_m.h"
#include "Texture.h"
#include <stack>

#define Color glm::vec3

#pragma pack(push, 1)
struct Vertex {
	glm::vec3 pos;
	glm::vec2 tex;
	glm::vec3 norm;
};
#pragma pack(pop)

class VertexBuffer
{
	GLuint vao;
	GLuint vbo;
	size_t vertex_count;
public:
	VertexBuffer(Vertex* verts, size_t vertex_count)
		:
		vao(0), vbo(0),
		vertex_count(vertex_count)
	{
		printf("VertexBuffer ctor\n");
		glGenVertexArrays(1, &vao); // Create VAO.
		glGenBuffers(1, &vbo); // Create VBO.

		glBindVertexArray(vao); // Bind VAO (global state).

		glBindBuffer(GL_ARRAY_BUFFER, vbo); // Bind VBO (global state).
		glBufferData(GL_ARRAY_BUFFER, sizeof(verts[0]) * vertex_count, verts, GL_STATIC_DRAW); // Upload vertex data to VBO.

		// [position attribute]
		// Define how the vertex shader's first parameter is read from the VBO.
		// For every 8 float, the first 3 float will be fed to the vertex shader's first parameter.
		// This also associates VBO to VAO, so when drawing from this buffer, only the VAO needs to be selected.
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		// [texture coord attribute]
		// The vertex shader's second parameter's description.
		// For every 8 float, the 4th and 5th float will be sent to the vertex shader's second parameter.
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);
		// [normal attribute]
		// The vertex shader's third parameter's description.
		// For every 8 float, the 6-8th floats will be loaded to the vertex shader's third parameter.
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
		glEnableVertexAttribArray(2);
	}
	~VertexBuffer()
	{
		if (vao == -1 && vbo == -1) { printf("VertexBuffer empty dtor\n"); return; }
		printf("VertexBuffer dtor\n");
		glDeleteBuffers(1, &vbo);
		glDeleteVertexArrays(1, &vao);
		// Note: VBO can also read from VAO with the following function:
		// glGetVertexAttribiv(0, GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING, &vboId);
	}
	VertexBuffer(const VertexBuffer&) = delete; // Delete copy constructor.
	void operator=(const VertexBuffer&) = delete; // Delete assignment operator.
	VertexBuffer(VertexBuffer&& o) noexcept // Move constructor.
		:vao(o.vao), vbo(o.vbo), vertex_count(o.vertex_count)
	{
		o.vao = -1;
		o.vbo = -1;
	}
	void operator=(VertexBuffer&& o) noexcept // Move assignment operator.
	{
		vao = o.vao;
		vbo = o.vbo;
		vertex_count = o.vertex_count;
		o.vao = -1;
		o.vbo = -1;
	}
	void Draw(GLenum mode = GL_TRIANGLES)
	{
		// Don't forget to set the model. -> shader.setMat4("model", model);
		glBindVertexArray(vao);
		glDrawArrays(mode, 0, (GLsizei)vertex_count);
	}
	void Draw(GLenum mode, GLint first, GLsizei count)
	{
		glBindVertexArray(vao);
		glDrawArrays(mode, first, count);
	}
	void Update(Vertex* verts, size_t vertex_count)
	{
		glBindBuffer(GL_ARRAY_BUFFER, vbo); // Bind VBO (global state).
		glBufferData(GL_ARRAY_BUFFER, sizeof(verts[0]) * vertex_count, verts, GL_STATIC_DRAW); // Upload vertex data to VBO.
	}
};

class View
{
	static glm::vec3 light_pos[];
	static bool light_isDirectional[];
	static int lightN;
public:
	static glm::vec3 eye, lookat, up;
	static float zoom, fov;
	static glm::mat4 model;
	static std::stack<glm::mat4> modelStack;
	static Shader* activeShader;
	static MaterialBase* currentMaterial;

	static inline void Init(Shader& shader, int screenWidth, int screenHeight)
	{
		glViewport(0, 0, screenWidth, screenHeight);
		/*glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(fov, screenWidth / (float)screenHeight, 1, 100);*/
		glm::mat4 projection = glm::perspective(glm::radians(View::fov), (float)screenWidth / (float)screenHeight, 0.1f, 100.0f);
		shader.setMat4("projection", projection);
	}
	static void SetShader(Shader& shader)
	{
		activeShader = &shader;
		shader.use();
	}
	static void Clear()
	{
		glClearColor(0.01f, 0.05f, 0.35f, 0.0f);		    // torlesi szin beallitasa
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // kepernyo torles

		//View::ResetTransform();
	}
	static void Move(glm::vec3 delta) {
		eye += delta;
	}

	static inline void ResetTransform()
	{
		//glMatrixMode(GL_MODELVIEW);
		//glLoadIdentity();
		//gluLookAt(View::eye.x, View::eye.y, View::eye.z, View::lookat.x, View::lookat.y, View::lookat.z, View::up.x, View::up.y, View::up.z);
		glm::mat4 view = glm::lookAt(eye, eye + lookat, up);
		activeShader->setMat4("view", view);

		ResetModel();
	}
	static inline void ResetModel()
	{
		while (!modelStack.empty()) modelStack.pop();
		model = glm::mat4(1.0f); // Initialize model matrix to identity matrix.
		activeShader->setMat4("model", model);
	}
	static inline void TranslateModel(const glm::vec3& v)
	{
		model = glm::translate(model, v);
	}
	static inline void RotateModel(const float angle, const glm::vec3& axis)
	{
		//float angle = 20.0f * i;
		//model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
		model = glm::rotate(model, glm::radians(angle), axis);
	}
	static inline void ScaleModel(const glm::vec3& s)
	{
		model = glm::scale(model, s);
	}
	static void PushMatrix()
	{
		modelStack.push(model);
	}
	static void PopMatrix()
	{
		model = modelStack.top();
		modelStack.pop();
	}
	static inline void UpdateModel()
	{
		//activeShader->setMat4("model", model);
		activeShader->setModel(model);
	}

	/*static void AddLight(glm::vec3 pos, bool isDirectional, Color IDiffuse, Color ISpecular, Color IAmbient = Color(0, 0, 0))
	{
		if (lightN == 8) lightN--;
		light_pos[lightN] = pos;
		light_isDirectional[lightN] = isDirectional;
		float array_[4] = { pos.x, pos.y, pos.z, (float)((isDirectional) ? (0) : (1)) };
		glLightfv(GL_LIGHT0 + lightN, GL_POSITION, array_);

		array_[0] = IAmbient.r; array_[1] = IAmbient.g; array_[2] = IAmbient.b; array_[3] = 1;
		glLightfv(GL_LIGHT0 + lightN, GL_AMBIENT, array_);

		array_[0] = IDiffuse.r; array_[1] = IDiffuse.g; array_[2] = IDiffuse.b;
		glLightfv(GL_LIGHT0 + lightN, GL_DIFFUSE, array_);

		array_[0] = ISpecular.r; array_[1] = ISpecular.g; array_[2] = ISpecular.b;
		glLightfv(GL_LIGHT0 + lightN, GL_SPECULAR, array_);

		glEnable(GL_LIGHT0 + lightN); lightN++;
	}*/

};
