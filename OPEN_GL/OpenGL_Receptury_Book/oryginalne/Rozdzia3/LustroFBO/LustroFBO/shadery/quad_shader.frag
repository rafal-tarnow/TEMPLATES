#version 330 core

layout(location=0) out vec4 vFragColor;		//wyj�ciowy kolor fragmentu

//wej�cie z shadera wierzcho�k�w
smooth in vec2 vUV;				//interpolowane wsp�rz�dne tekstury 2D

//uniform
uniform sampler2D textureMap;	//mapa tekstury do wy�wietlenia

void main()
{ 
	//wyznaczanie koloru wyj�ciowego z mapy tekstury przy u�yciu wej�ciowych wsp�rz�dnych tekstury
	vFragColor = texture(textureMap, vUV);
}