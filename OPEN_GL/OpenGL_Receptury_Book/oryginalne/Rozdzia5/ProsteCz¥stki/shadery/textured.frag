#version 330 core

layout(location=0) out vec4 vFragColor; //wyjście shadera fragmentów

//wejście z shadera wierzchołków
smooth in vec4 vSmoothColor;	//liniowo interpolowany kolor cząsteczki

uniform sampler2D textureMap;	//tekstura cząsteczki 

void main()
{ 
	//wartość alfa gładkiego koloru cząsteczki wygasza kolor uzyskany
	//z tekstury 
	vFragColor = texture(textureMap, gl_PointCoord)* vSmoothColor.a;  
}