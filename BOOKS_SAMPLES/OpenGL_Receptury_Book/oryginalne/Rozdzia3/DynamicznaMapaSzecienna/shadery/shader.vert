#version 330 core
  
layout(location=0) in vec3 vVertex; //po�o�enie w przestrzeni obiektu
//uniform
uniform mat4 MVP;	//po��czona macierz modelu, widoku i rzutowania

void main()
{ 
	//mno�enie po�o�enia w przestrzeni obiektu przez po��czon� macierz MVP w celu uzyskania po�o�enia w przestrzeni przyci�cia 
	gl_Position = MVP*vec4(vVertex.xyz,1);
}