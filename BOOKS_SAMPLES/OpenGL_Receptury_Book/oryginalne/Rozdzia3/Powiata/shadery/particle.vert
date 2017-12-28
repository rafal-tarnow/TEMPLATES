#version 330 core
   
layout(location=0) in vec3 vVertex; //po³o¿enie wierzcho³ka w przestrzeni obiektu

//uniform
uniform mat4 MVP;	//po³¹czona macierz modelu, widoku i rzutowania
 
//wyjœcie shadera wierzcho³ków
smooth out vec4 color;	

const vec4 colors[8]=vec4[8](vec4(1,0,0,1), vec4(0,1,0,1), vec4(0,0,1,1),
							 vec4(1,1,0,1), vec4(0,1,1,1), vec4(1,0,1,1),
							 vec4(0.5,0.5,0.5,1),  vec4(1,1,1,1)) ;
 
void main()
{ 	 	 
	//no¿enie po³o¿enia wierzcho³ka przez po³¹czon¹ macierz modelu, widoku i rzutowania 
    //w celu uzyskania po³o¿enia w przestrzeni przyciêcia
	gl_Position = MVP*vec4(vVertex,1);
	//pobieranie koloru cz¹stki z tablicy sta³ych kolorów z u¿yciem gl_VertexID jako indeksem
	color = colors[gl_VertexID/4]; 
}