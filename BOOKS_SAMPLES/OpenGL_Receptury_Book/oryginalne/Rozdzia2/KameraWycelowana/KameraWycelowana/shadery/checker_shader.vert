#version 330 core
  
layout(location = 0) in vec3 vVertex;	//po³o¿enie w przestrzeni obiektu

//uniform
uniform mat4 MVP;	//macierz modelu, widoku i rzutowania

//wyjœcie shadera wierzcho³ków
smooth out vec2 vUV;	//wspó³rzêdne tekstury 2D

void main()
{  
	//mno¿enie po³o¿enia w przestrzeni obiektu przez macierz MVP w celu uzyskania po³o¿enia w przestrzeni przyciêcia 
	gl_Position = MVP*vec4(vVertex.xyz,1);

	//wspó³rzêdne x i z wierzcho³ka jako wspó³rzêdne tekstury 2D
	vUV =   (vVertex.xz); 
}