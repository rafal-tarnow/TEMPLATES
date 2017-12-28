#version 330 core

layout(location=0) smooth out vec4 vFragColor;	//wyjście shadera fragmentów
uniform vec4 vColor;							//uniform koloru do przypisania fragmentowi

void main()
{ 		
	//przypisz fragmentowi kolor z uniformu
	vFragColor = vColor;  
}