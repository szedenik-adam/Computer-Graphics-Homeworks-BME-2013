
#include "Texture.h"
#include "View.h"

// virtual
void Material::Select() {
	/*glDisable(GL_TEXTURE_2D);
	glMaterialfv(GL_FRONT, GL_AMBIENT, ka);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, kd);
	glMaterialfv(GL_FRONT, GL_SPECULAR, ks);
	glMaterialf(GL_FRONT, GL_SHININESS, 5.0f);*/

	View::activeShader->setVec3("material.ambient", ka);
	View::activeShader->setVec3("material.diffuse", kd);
	View::activeShader->setVec3("material.specular", ks);
	View::activeShader->setFloat("material_shininess", 5.0f);
}

// virtual
void Texture::Select() {
	View::activeShader->setVec3("material.ambient", 0,0,0);
	//View::activeShader->setInt("texture0", texture);
	//glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);

	/*glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	if (colors_enabled) {
		glMaterialfv(GL_FRONT, GL_AMBIENT, colors.ka);
		glMaterialfv(GL_FRONT, GL_DIFFUSE, colors.kd);
		glMaterialfv(GL_FRONT, GL_SPECULAR, colors.ks);
		glMaterialf(GL_FRONT, GL_SHININESS, 5.0f);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	}
	else { glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE); }*/
}

void MaterialBase::Setup()
{
	if (this != currentMaterial || !allowCaching)
	{
		currentMaterial = this;
		this->Select();
	}
}
