#version 330 core
   
layout(location=0) in vec3 vVertex; //po�o�enie wierzcho�ka w przestrzeni obiektu

//uniform
uniform mat4 MVP;	//po��czona macierz modelu, widoku i rzutowania
 
//wyj�cie shadera wierzcho�k�w
smooth out vec4 color;	

const vec4 colors[8]=vec4[8](vec4(1,0,0,1), vec4(0,1,0,1), vec4(0,0,1,1),
							 vec4(1,1,0,1), vec4(0,1,1,1), vec4(1,0,1,1),
							 vec4(0.5,0.5,0.5,1),  vec4(1,1,1,1)) ;
 
void main()
{ 	 	 
	//no�enie po�o�enia wierzcho�ka przez po��czon� macierz modelu, widoku i rzutowania 
    //w celu uzyskania po�o�enia w przestrzeni przyci�cia
	gl_Position = MVP*vec4(vVertex,1);
	//pobieranie koloru cz�stki z tablicy sta�ych kolor�w z u�yciem gl_VertexID jako indeksem
	color = colors[gl_VertexID/4]; 
}