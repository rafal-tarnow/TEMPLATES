#version 330 core

layout(location = 0) out vec4 vFragColor; //wyjście shadera fragmentów

smooth in vec3 outNormal;	//wejście z shadera wierzchołków 
							//interpolowane przez rasteryzer
 
void main()
{
	//wyprowadzenie normalnej jako koloru dla bieżącego fragmentu
	vFragColor = vec4(outNormal,1);
}