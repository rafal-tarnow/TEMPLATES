#version 330 core
  
layout(location = 0) in vec3 vVertex;	//po�o�enie w przestrzeni obiektu

//uniform
uniform mat4 MVP;	//macierz modelu, widoku i rzutowania

//wyj�cie shadera wierzcho�k�w
smooth out vec2 vUV;	//wsp�rz�dne tekstury 2D

void main()
{  
	//mno�enie po�o�enia w przestrzeni obiektu przez macierz MVP w celu uzyskania po�o�enia w przestrzeni przyci�cia 
	gl_Position = MVP*vec4(vVertex.xyz,1);

	//wsp�rz�dne x i z wierzcho�ka jako wsp�rz�dne tekstury 2D
	vUV =   (vVertex.xz); 
}