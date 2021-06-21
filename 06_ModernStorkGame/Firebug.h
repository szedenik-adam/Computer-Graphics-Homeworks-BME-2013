#pragma once

#include "View.h"
#include "Texture.h"
#include "Frog.h"

class FireBug {
	glm::vec3 pos, rotation; float size;
	bool posChanged;
public:
	FireBug(): pos(1, 3.3, 4), rotation(0, 0, 0), size(0.1), posChanged(true) {
	}
	void SetPosition(const glm::vec3& pos) {
		if (pos != this->pos) {
			this->pos = pos;
			this->posChanged = true;
		}
	}
	void Draw() {
		if (this->posChanged) {
			this->posChanged = false;
			glm::vec3 lightPos(pos.x, pos.y, pos.z - size / 2);
			View::activeShader->setVec3("firebugLightPos", lightPos);
		}
		View::ResetTransform();
		View::TranslateModel(pos);
		View::ScaleModel(glm::vec3(size / 2, size / 2, size * 2));
		View::UpdateModel();
		/*glTranslatef(pos.x, pos.y, pos.z);
		glRotatef(rotation.x, 1, 0, 0);
		glRotatef(rotation.y, 0, 1, 0);
		glRotatef(rotation.z, 0, 0, 1);
		glScalef(size / 2, size / 2, size * 2);*/

		Material::firebug_body.Setup();

		//glNormal3f(0, 0, 1);

		Ellipsoid::DrawSphere(8);//, false, false);
	}
};