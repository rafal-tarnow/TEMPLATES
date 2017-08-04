#version 330 core
  
layout(location=0) in vec3 vVertex;	//po�o�enie wierzcho�ka w przestrzeni obiektu

//wyj�cie shadera wierzcho�k�w
smooth out vec2 vUV;	//przekazywanie wsp�rz�dnych tekstury dla shadera fragment�w
 
//uniform
uniform mat4 MVP;		//po��czona macierz modelu, widoku i rzutowania

void main()
{ 	 
	//mno�enie po�o�enia wierzcho�ka przez po��czon� macierz modelu, widoku i rzutowania 
	gl_Position = MVP * vec4(vVertex.xyz,1); 

	//wyznaczanie wsp�rz�dnych tekstury na podstawie po�o�enia wierzcho�ka w przestrzenie obiektu
	vUV = vec2( (vVertex.x+1), vVertex.y) *0.5; 
}