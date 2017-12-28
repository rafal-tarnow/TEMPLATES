#version 330 core

layout(location=0) out vec4 vFragColor;	//wyjście shadera fragmentów

smooth in vec3 vEyeSpaceNormal;			//normalna w przestrzeni oka pobrana z shadera wierzchołków
 
void main()
{ 
	//wyprowadzenie normalnej w przestrzeni oka po ograniczeniu do zakresu 0-1
	vFragColor = vec4(normalize(vEyeSpaceNormal)*0.5 + 0.5, 1); 
}