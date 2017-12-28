#version 330 core

layout(location=0) smooth out vec4 vFragColor;	//wyjście shadera fragmentów
smooth in vec4 color;							//interpolowane współrzędne tekstury pobrane z shadera wierzchołków

void main()
{ 		
	//po prostu wyprowadź interpolowany kolor jako wyjściowy kolor fragmentu
	vFragColor = color;  	
}