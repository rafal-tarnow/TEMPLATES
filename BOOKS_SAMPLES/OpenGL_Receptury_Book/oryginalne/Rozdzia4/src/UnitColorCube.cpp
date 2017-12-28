#include "UnitColorCube.h"
#include <glm/glm.hpp>

CUnitColorCube::CUnitColorCube(void)
{
	//generowanie obiektu kostki
	shader.LoadFromFile(GL_VERTEX_SHADER, "shadery/cube_shader.vert");
	shader.LoadFromFile(GL_FRAGMENT_SHADER, "shadery/cube_shader.frag");
	shader.CreateAndLinkProgram();
	shader.Use();
		shader.AddAttribute("vVertex");
		shader.AddAttribute("vColor");
		shader.AddUniform("MVP");
	shader.UnUse();
	/*
	glm::vec3* vertices=new glm::vec3[8];
	vertices[0]=glm::vec3(-0.5f,-0.5f,-0.5f);
	vertices[1]=glm::vec3( 0.5f,-0.5f,-0.5f);
	vertices[2]=glm::vec3( 0.5f, 0.5f,-0.5f);
	vertices[3]=glm::vec3(-0.5f, 0.5f,-0.5f);
	vertices[4]=glm::vec3(-0.5f,-0.5f, 0.5f);
	vertices[5]=glm::vec3( 0.5f,-0.5f, 0.5f);
	vertices[6]=glm::vec3( 0.5f, 0.5f, 0.5f);
	vertices[7]=glm::vec3(-0.5f, 0.5f, 0.5f);
	
	total_indices = 6*2*3;
	GLushort* indices = new GLushort[total_indices];
	 
	int count = 0;
	
	//wype�nianie tablicy indeks�w
	GLushort* id=&indices[0]; 

	//�ciana dolna
	*id++ = 0; 	*id++ = 5; 	*id++ = 4;
	*id++ = 5; 	*id++ = 0; 	*id++ = 1; 
	
	//�ciana g�rna
	*id++ = 3; 	*id++ = 7; 	*id++ = 6;
	*id++ = 3; 	*id++ = 6; 	*id++ = 2;

	//�ciana przednia
	*id++ = 7; 	*id++ = 4; 	*id++ = 6;
	*id++ = 6; 	*id++ = 4; 	*id++ = 5;

	//�ciana tylna
	*id++ = 2; 	*id++ = 1; 	*id++ = 3;
	*id++ = 3; 	*id++ = 1; 	*id++ = 0;

	//�ciana lewa 
	*id++ = 3; 	*id++ = 0; 	*id++ = 7;
	*id++ = 7; 	*id++ = 0; 	*id++ = 4;

	//�ciana prawa 
	*id++ = 6; 	*id++ = 5; 	*id++ = 2;
	*id++ = 2; 	*id++ = 5; 	*id++ = 1;
	
	//vao i vbo 
	glGenVertexArrays(1, &vaoID);
	glGenBuffers(1, &vboVerticesID);
	glGenBuffers(1, &vboIndicesID);

	glBindVertexArray(vaoID);	

		glBindBuffer (GL_ARRAY_BUFFER, vboVerticesID);
		glBufferData (GL_ARRAY_BUFFER, 8*sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);
		 
		glEnableVertexAttribArray(shader["vVertex"]);
		glVertexAttribPointer(shader["vVertex"], 3, GL_FLOAT, GL_FALSE,0,0);
		  
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIndicesID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort)*total_indices, &indices[0], GL_STATIC_DRAW);
		 
	glBindVertexArray(0);
	delete [] indices;
	delete [] vertices;
	*/
	Init();
}


CUnitColorCube::~CUnitColorCube(void)
{
	/*
	//likwidacja shadera
	shader.DeleteShaderProgram();

	//likwidacja VBO i VAO
	glDeleteBuffers(1, &vboVerticesID);
	glDeleteBuffers(1, &vboIndicesID);
	glDeleteVertexArrays(1, &vaoID);
	*/
} 

int CUnitColorCube::GetTotalVertices() {
	return 8;
}

int CUnitColorCube::GetTotalIndices() {
	//6 faces with 2 triangles each with 3 vertices
	return 6*2*3;
}

GLenum CUnitColorCube::GetPrimitiveType() {
	return GL_TRIANGLES;
}

 void CUnitColorCube::FillVertexBuffer(GLfloat* pBuffer) {
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

void CUnitColorCube::FillIndexBuffer(GLuint* pBuffer) {
	int i=0;
	//wype�nianie tablicy indeks�w
	GLuint* id=pBuffer; 

	//�ciana dolna
	*id++ = 0; 	*id++ = 5; 	*id++ = 4;
	*id++ = 5; 	*id++ = 0; 	*id++ = 1; 
	
	//�ciana g�rna
	*id++ = 3; 	*id++ = 7; 	*id++ = 6;
	*id++ = 3; 	*id++ = 6; 	*id++ = 2;

	//�ciana przednia
	*id++ = 7; 	*id++ = 4; 	*id++ = 6;
	*id++ = 6; 	*id++ = 4; 	*id++ = 5;

	//�ciana tylna
	*id++ = 2; 	*id++ = 1; 	*id++ = 3;
	*id++ = 3; 	*id++ = 1; 	*id++ = 0;

	//�ciana lewa
	*id++ = 3; 	*id++ = 0; 	*id++ = 7;
	*id++ = 7; 	*id++ = 0; 	*id++ = 4;

	//�ciana prawa
	*id++ = 6; 	*id++ = 5; 	*id++ = 2;
	*id++ = 2; 	*id++ = 5; 	*id++ = 1;
}