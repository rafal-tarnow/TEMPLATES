#version 330 core

layout(location=0) out vec4 vFragColor; //wyj�cie shadera fragment�w
 
smooth in vec4 color; //interpolowany kolor z shadera wierzcho�k�w

void main() { 
	//ustawienie koloru pobranego z shadera wierzcho�k�w jako koloru wyj�ciowego
	vFragColor = color;	 
}