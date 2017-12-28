#version 330 core

layout(location = 0) out vec4 vFragColor;	//wyj�ciowy kolor fragmentu

//wej�cie z shadera wierzcho�k�w
smooth in vec2 vUV;				//interpolowane wsp�rz�dne tekstury 2D

//uniform
uniform sampler2D textureMap;	//texture map


void main()
{
	//wyszukiwanie koloru w mapie tekstury na podstawie wsp�rz�dnych interpolowanych
	vFragColor = texture(textureMap, vUV).rrrr;
}