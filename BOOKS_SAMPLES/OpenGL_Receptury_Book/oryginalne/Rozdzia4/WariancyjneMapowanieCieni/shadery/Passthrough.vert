#version 330 core
  
layout(location=0) in vec2 vVertex;		//po�o�enie wierzcho�ka w przestrzeni obiektu
//wyj�cie shadera
smooth out vec2 vUV;					//wsp�rz�dne tekstury

void main()
{ 	 
	//wyznaczanie po�o�enia w przestrzeni przyci�cia na podstawie po�o�enia w przestrzeni obiektu
	gl_Position = vec4(vVertex*2-1.0,0,1);
	//przypisanie po�o�enia wierzcho�ka jako wyj�ciowych wsp�rz�dnch tekstury
	vUV = vVertex;
}