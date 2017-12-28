#include "Skybox.h"

#include <glm/gtc/type_ptr.hpp>

CSkybox::CSkybox(void)
{ 
	//generowanie obiektu szeœcianu
	shader.LoadFromFile(GL_VERTEX_SHADER, "shadery/skybox.vert");
	shader.LoadFromFile(GL_FRAGMENT_SHADER, "shadery/skybox.frag");
	//kompilacja i konsolidacja programu shaderowego
	shader.CreateAndLinkProgram();
	shader.Use();
		//atrybuty i uniformy
		shader.AddAttribute("vVertex"); 
		shader.AddUniform("MVP");
		shader.AddUniform("cubeMap");
		//inicjalizacja sta³ego uniformu
		glUniform1i(shader("cubeMap"),0);
	shader.UnUse();
	 
	//ustawienie pól dziedziczonych
	Init();
}
 

CSkybox::~CSkybox(void)
{
	 
} 

//skybox ma 8 wierzcho³ków
int CSkybox::GetTotalVertices() {
	return 8;
}

int CSkybox::GetTotalIndices() {
	//6 œcianek z 2 trójk¹tami po 3 wierzcho³ki
	return 6*2*3;
}

GLenum CSkybox::GetPrimitiveType() {
	return GL_TRIANGLES;
}

 void CSkybox::FillVertexBuffer(GLfloat* pBuffer) {
	glm::vec3* vertices = (glm::vec3*)(pBuffer); 
	vertices[0]=glm::vec3(-0.5f,-0.5f,-0.5f);
	vertices[1]=glm::vec3( 0.5f,-0.5f,-0.5f);
	vertices[2]=glm::vec3( 0.5f, 0.5f,-0.5f);
	vertices[3]=glm::vec3(-0.5f, 0.5f,-0.5f);
	vertices[4]=glm::vec3(-0.5f,-0.5f, 0.5f);
	vertices[5]=glm::vec3( 0.5f,-0.5f, 0.5f);
	vertices[6]=glm::vec3( 0.5f, 0.5f, 0.5f);
	vertices[7]=glm::vec3(-0.5f, 0.5f, 0.5f); 
}

void CSkybox::FillIndexBuffer(GLuint* pBuffer) {
	 
	//wype³nianie tablicy indeksów
	GLuint* id=pBuffer; 

	//œciana dolna
	*id++ = 0; 	*id++ = 4; 	*id++ = 5;
	*id++ = 5; 	*id++ = 1; 	*id++ = 0; 
	
	//œciana górna
	*id++ = 3; 	*id++ = 6; 	*id++ = 7;
	*id++ = 3; 	*id++ = 2; 	*id++ = 6;

	//œciana przednia
	*id++ = 7; 	*id++ = 6; 	*id++ = 4;
	*id++ = 6; 	*id++ = 5; 	*id++ = 4;

	//œciana tylna
	*id++ = 2; 	*id++ = 3; 	*id++ = 1;
	*id++ = 3; 	*id++ = 0; 	*id++ = 1;

	//œciana lewa
	*id++ = 3; 	*id++ = 7; 	*id++ = 0;
	*id++ = 7; 	*id++ = 4; 	*id++ = 0;

	//œciana prawa
	*id++ = 6; 	*id++ = 2; 	*id++ = 5;
	*id++ = 2; 	*id++ = 1; 	*id++ = 5;
}