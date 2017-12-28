#version 330 core

layout(location=0) out vec4 vFragColor;		//wyjœciowy kolor fragmentu

//wejœcie z shadera wierzcho³ków
smooth in vec2 vUV;				//interpolowane wspó³rzêdne tekstury 2D

//uniform
uniform sampler2D textureMap;	//mapa tekstury do wyœwietlenia

void main()
{ 
	//wyznaczanie koloru wyjœciowego z mapy tekstury przy u¿yciu wejœciowych wspó³rzêdnych tekstury
	vFragColor = texture(textureMap, vUV);
}