#version 330 core
  
layout(location=0) in vec3 vVertex;	//po³o¿enie wierzcho³ka w przestrzeni obiektu

//wyjœcie shadera wierzcho³ków
smooth out vec2 vUV;	//przekazywanie wspó³rzêdnych tekstury dla shadera fragmentów
 
//uniform
uniform mat4 MVP;		//po³¹czona macierz modelu, widoku i rzutowania

void main()
{ 	 
	//mno¿enie po³o¿enia wierzcho³ka przez po³¹czon¹ macierz modelu, widoku i rzutowania 
	gl_Position = MVP * vec4(vVertex.xyz,1); 

	//wyznaczanie wspó³rzêdnych tekstury na podstawie po³o¿enia wierzcho³ka w przestrzenie obiektu
	vUV = vec2( (vVertex.x+1), vVertex.y) *0.5; 
}