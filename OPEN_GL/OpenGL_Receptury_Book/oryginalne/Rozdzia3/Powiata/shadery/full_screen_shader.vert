#version 330 core
  
layout(location=0) in vec2 vVertex; //po�o�enie wierzcho�ka w przestrzeni obiektu

//wyj�cie shadera wierzcho�k�w
smooth out vec2 vUV;			//wsp�rz�dne tekstury 2D dla shadera fragment�w

void main()
{   	
	//wyznaczanie po�o�enia w przestrzeni przyci�cia na podstawie po�o�enia w przestrzeni w obiektu
	gl_Position = vec4(vVertex.xy*2-1.0,0,1);	 

	//wyznaczanie wsp�rz�dnych tekstury na podstawie po�o�enia wierzcho�ka w przestrzenie obiektu
	vUV = vVertex;
}