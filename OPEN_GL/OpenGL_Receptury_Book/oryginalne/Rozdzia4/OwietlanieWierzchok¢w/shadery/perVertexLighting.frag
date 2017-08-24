#version 330 core

layout(location=0) out vec4 vFragColor; //wyjœcie shadera fragmentów
 
smooth in vec4 color; //interpolowany kolor z shadera wierzcho³ków

void main() { 
	//ustawienie koloru pobranego z shadera wierzcho³ków jako koloru wyjœciowego
	vFragColor = color;	 
}