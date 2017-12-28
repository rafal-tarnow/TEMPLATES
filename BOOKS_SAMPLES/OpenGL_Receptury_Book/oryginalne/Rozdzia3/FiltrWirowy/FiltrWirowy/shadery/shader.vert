#version 330 core
  
layout(location=0) in vec2 vVertex; //wsp�rz�dne wierzcho�ka w przestrzeni obiektu

//wyj�cie shadera wierzcho�k�w
smooth out vec2 vUV;	//wsp�rz�dne tekstury
void main()
{    
	//po�o�enie w przestrzeni przyci�cia
	gl_Position = vec4(vVertex*2.0-1,0,1);	 

	//wsp�rz�dne w przestrzeni obiektu jako wsp�rz�dne tekstury
	vUV = vVertex;
}