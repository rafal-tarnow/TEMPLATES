#include "RenderableObject.h"
#include <glm/glm.hpp>

RenderableObject::RenderableObject(void)
{
	
}


RenderableObject::~RenderableObject(void)
{
	Destroy();
}

void RenderableObject::Init() {
	//przygotowanie VAO i VBO
	glGenVertexArrays(1, &vaoID);
	glGenBuffers(1, &vboVerticesID);
	glGenBuffers(1, &vboIndicesID);

	//pobranie liczby wierzcho��w i indeks�w
	totalVertices = GetTotalVertices();
	totalIndices  = GetTotalIndices();
	primType      = GetPrimitiveType();

	//alokacja bufor�w
	glBindVertexArray(vaoID);	

		glBindBuffer (GL_ARRAY_BUFFER, vboVerticesID);
		glBufferData (GL_ARRAY_BUFFER, totalVertices * sizeof(glm::vec3), 0, GL_STATIC_DRAW);
		 
		GLfloat* pBuffer = static_cast<GLfloat*>(glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY));
			FillVertexBuffer(pBuffer);
		glUnmapBuffer(GL_ARRAY_BUFFER);

		glEnableVertexAttribArray(shader["vVertex"]);
		glVertexAttribPointer(shader["vVertex"], 3, GL_FLOAT, GL_FALSE,0,0);
		 
		  
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIndicesID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, totalIndices * sizeof(GLuint), 0, GL_STATIC_DRAW);
		
		GLuint* pIBuffer = static_cast<GLuint*>(glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY));
			FillIndexBuffer(pIBuffer);
		glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);

	glBindVertexArray(0);
}

void RenderableObject::Destroy() {
	//likwidacja programu shaderowego
	shader.DeleteShaderProgram();

	//likwidacja vao i vbo
	glDeleteBuffers(1, &vboVerticesID);
	glDeleteBuffers(1, &vboIndicesID);
	glDeleteVertexArrays(1, &vaoID);
}


void RenderableObject::Render(const GLfloat* MVP) {
	shader.Use();				
		if(MVP!=0)
			glUniformMatrix4fv(shader("MVP"), 1, GL_FALSE, MVP);
		SetCustomUniforms();
		glBindVertexArray(vaoID);
			glDrawElements(primType, totalIndices, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	shader.UnUse();
}

void RenderableObject::SetCustomUniforms() {

}

GLSLShader* RenderableObject::GetShader() {
	return &shader;
}