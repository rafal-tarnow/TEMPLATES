#version 330 core

layout(location=0) out vec4 vFragColor;	//wyjœcie shadera fragmentów

//wejœcie z shadera wierzcho³ków
smooth in vec4 vSmoothColor;		//interpolowany kolor dla shadera fragmentów

void main()
{
	//interpolowany kolor jako wyjœcie shadera
	vFragColor = vSmoothColor;
}