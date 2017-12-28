#version 330 core

layout(location=0) out vec4 vFragColor; //wyjście shadera fragmentów
smooth in vec2 vUV;	//interpolowane współrzędne tekstury pobrane z shadera wierzchołków

uniform sampler2D textureMap;	//uniform z mapą tekstury

void main()
{ 
	//wyprowadzenie próbki tekstury pobranej z punktu o podanych współrzędnych
	vFragColor =  texture(textureMap, vUV);
}