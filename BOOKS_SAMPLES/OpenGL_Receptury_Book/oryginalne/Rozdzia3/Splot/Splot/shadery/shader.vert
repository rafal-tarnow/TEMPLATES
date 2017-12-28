#version 330 core
  
layout(location=0) in vec2 vVertex; //po�o�enie wierzcho�ka w przestrzeni obiektu

//Wyj�cie shadera wierzcho�k�w
smooth out vec2 vUV;	//wsp�rz�dne teksturowe potrzebne do pr�bkowania tekstury w shaderze fragment�w

void main()
{    
	//wyprowadzenie po�o�enia w przestrzeni przyci�cia
	gl_Position = vec4(vVertex*2.0-1,0,1);	 

	//ustawienie wej�ciowego po�o�enia w przestrzeni obiektu jako wsp�rz�dnych teksturowych
	vUV = vVertex;
}