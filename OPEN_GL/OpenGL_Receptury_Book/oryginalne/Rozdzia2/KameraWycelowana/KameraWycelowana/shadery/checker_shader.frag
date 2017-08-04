#version 330 core

layout(location = 0) out vec4 vFragColor;	//wyjœciowy kolor fragmentu

//wejœcie z shadera wierzcho³ków
smooth in vec2 vUV;				//interpolowane wspó³rzêdne tekstury 2D

//uniform
uniform sampler2D textureMap;	//texture map


void main()
{
	//wyszukiwanie koloru w mapie tekstury na podstawie wspó³rzêdnych interpolowanych
	vFragColor = texture(textureMap, vUV).rrrr;
}