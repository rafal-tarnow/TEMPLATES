#version 330 core

layout(location=0) out vec4 vFragColor; //wyjście shadera fragmentów

//wejście z shadera wierzchołków
smooth in vec4 vSmoothColor;	//liniowo interpolowany kolor cząsteczki


void main()
{
	//gładki kolor cząsteczki jako wyjściowy kolor fragmentu
	vFragColor = vSmoothColor; 
}