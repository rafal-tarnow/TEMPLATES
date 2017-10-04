#version 330 core

layout(location = 0) out vec4 vFragColor;	//wyjściowy kolor fragmentu

//wejście z shadera wierzchołków
smooth in vec2 vUV;				//interpolowane współrzędne tekstury 2D

//uniform
uniform sampler2D textureMap;	//mapa tekstury


void main()
{
	//wyszukiwanie koloru w mapie tekstury na podstawie współrzędnych interpolowanych 
	vFragColor = texture(textureMap, vUV).rrrr;
}
