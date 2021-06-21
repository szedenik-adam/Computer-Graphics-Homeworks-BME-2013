#pragma once

#ifdef __EMSCRIPTEN__
// For emscripten, instead of using glad we use its built-in support for OpenGL.

#define GLEW_STATIC
#include <GL/glew.h>

#include <GLFW/glfw3.h>
#include <emscripten.h>

#else
#include <glad/glad.h>
#endif

#include <glm/glm.hpp>

#define Color glm::vec3

class MaterialBase {
	static bool allowCaching;
	static MaterialBase* currentMaterial;
	virtual void Select() = 0;
public:
	void Setup(); // Caches last Material (skips OpenGL calls when reusing the last used material).
	static void ToggleCaching()
	{
		allowCaching = !allowCaching;
		printf("Material caching "); printf(allowCaching?"enabled\n":"diabled\n");
	}
};

class Material : public MaterialBase {
public:
	float kd[4], ks[4], ka[4];
	Material() { ka[3] = 1; kd[3] = 1; ks[3] = 1; }
	Material(glm::vec3 a, glm::vec3 d, glm::vec3 s) {
		ka[0] = a.r; ka[1] = a.g; ka[2] = a.b; ka[3] = 1;
		kd[0] = d.r; kd[1] = d.g; kd[2] = d.b; kd[3] = 1;
		ks[0] = s.r; ks[1] = s.g; ks[2] = s.b; ks[3] = 1;
	}
	static Material frog_body;
	static Material frog_leg;
	static Material frog_eye;
	static Material golya_beak;
	static Material golya_eye;
	static Material firebug_body;
	static Material texture_default;
	static Material shadow;
private:
	virtual void Select();
};

class Texture : public MaterialBase {
public:
	GLuint texture;
	Material colors; bool colors_enabled;
	Texture() :texture(0), colors_enabled(false) {}
	Texture(GLuint texture) :texture(texture), colors_enabled(false) {}
	Texture(GLuint texture, glm::vec3 a, glm::vec3 d, glm::vec3 s) :texture(texture), colors(a, d, s), colors_enabled(true) {}

	static Texture grass;
	static Texture grass_frog;
	static Texture golya_;
	static GLuint textures[];

	static void TextureInit()
	{
		GLenum error = glGetError();
		//glEnable(GL_TEXTURE_2D); // Only needed for fixed-function pipeline.
		glGenTextures(1, textures + 0);
		glBindTexture(GL_TEXTURE_2D, textures[0]);
		//static INT32 bitmap[256*256];
		int32_t* bitmap = (int32_t*)malloc(sizeof(int32_t) * 1024 * 1024);

		uint8_t color[4]; int n = 256 * 256;
		int random = rand();
		color[1] = random & 127; random >>= 8; color[2] = random & 7; random >>= 3; color[0] = random & 7;  color[3] = 255;
		for (int i = 0; i < 256 * 256; i++) { bitmap[i] = *((int32_t*)color); }
		for (int i = 0; i < 12; i++) {
			random = rand();
			color[1] = random & 127; random >>= 8; color[2] = random & 7; random >>= 3; color[0] = random & 7;  color[3] = 255;

			n = (int)(n / 1.2);
			for (int j = 0; j < n; j++) {
				int offset = ((rand() & 255) << 8) | (rand() & 255);
				bitmap[offset] = *((int32_t*)color);
			}
		}
		// In OpenGL ES, format and internalFormat has to be the same (GL_RGBA or GL_RGB).
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 64, 64, 0,
			GL_RGBA, GL_UNSIGNED_BYTE, bitmap);
		error = glGetError();
		glGenerateMipmap(GL_TEXTURE_2D);
		Texture::grass = Texture(textures[0]);
		Texture::grass_frog = Texture(textures[0], Color(0.1, 0.1, 0.1), Color(0.3, 0.8, 0.2), Color(0.1, 0.3, 0.1));

		glGenTextures(1, textures + 1);
		glBindTexture(GL_TEXTURE_2D, textures[1]);

		uint8_t white[4], black[4];
		white[0] = 255; white[1] = 255; white[2] = 255; white[3] = 255;
		black[0] = 10; black[1] = 10; black[2] = 10; black[3] = 255;

		for (int i = 0; i < 1024 * 1024; i++) bitmap[i] = *((int32_t*)white);
		for (int y = 0; y < 280; y++)   for (int x = 0; x < 160; x++) bitmap[x + 1024 * y] = *((int32_t*)black);
		for (int y = 280; y < 1024; y++) for (int x = 0; x < 64; x++) bitmap[x + 1024 * y] = *((int32_t*)black);
		for (int y = 240; y < 280; y++)  for (int x = 64; x < (64 + (160 - 64) * (((y - 120) / (280 - 120.0f)))); x++) bitmap[x + 1024 * y] = *((int32_t*)white);
		for (int y = 120; y < 640; y++)  for (int x = 64; x < 64 + (440 - 64) * pow((y - 120) / (760 - 120.0f), 4); x++) bitmap[x + 1024 * y] = *((int32_t*)black);
		int endx = 64 + (440 - 64) * pow((640 - 120) / (760 - 120.0f), 2);
		for (int y = 640; y < 800; y++) for (int x = 64; x < endx - 120 * (pow((y - 640) / 160.0f, 2)); x++) bitmap[x + 1024 * y] = *((int32_t*)black);
		for (int i = 0; i < 1000; i++) {
			int x = rand() & 1023; int y = rand() & 1023;
			if (!x) x = 1; if (x > 1022) x = 1022;
			if (!y) y = 1; if (y > 1022) y = 1022;
			int32_t pixel = bitmap[x + 1024 * y];
			if ((pixel & 255) > 100) pixel -= 40 + (40 << 8) + (40 << 16);
			bitmap[x + 1024 * y] = pixel; bitmap[x + 1 + 1024 * y] = pixel; bitmap[x + 1024 * (y + 1)] = pixel;
			bitmap[x - 1 + 1024 * y] = pixel; bitmap[x + 1024 * (y - 1)] = pixel;
		}

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1024, 1024, 0,
			GL_RGBA, GL_UNSIGNED_BYTE, bitmap);
		glGenerateMipmap(GL_TEXTURE_2D);
		Texture::golya_ = Texture(textures[1], Color(0.05, 0.05, 0.05), Color(0.8, 0.8, 0.8), Color(0.2, 0.2, 0.2));

		free(bitmap);
	}
private:
	virtual void Select();
};