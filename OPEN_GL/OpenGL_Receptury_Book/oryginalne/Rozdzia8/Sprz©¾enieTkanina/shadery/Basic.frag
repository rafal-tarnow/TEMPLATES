#version 330 core

layout (location=0) smooth out vec4 vFragColor;	//wyjście shadera fragmentów
smooth in vec4 oColor;							//kolor pobrany z shadera wierzchołków

void main()
{ 		
	//Równanie jest następujące sqrt(dot(gl_PointCoord-0.5,gl_PointCoord-0.5)>0.5)
	//podniesienie do kwadratu obu stron daje
	//odrzucamy fragmenty lezące poza sferą
	if(dot(gl_PointCoord-0.5,gl_PointCoord-0.5)>0.25)	
		discard;
	else
		vFragColor = oColor;  
}