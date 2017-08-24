#version 330 core
  
layout(location = 0) in vec3 vVertex;	//po³o¿enie wierzcho³ka w przestrzeni obiektu

//uniform
uniform mat4 MVP;  //po³¹czona macierz modelu, widoku i rzutowania

//dane wyjœciowe dla shadera fragmentów
smooth out vec3 uv;	//trójwymiarowe wspó³rzêdne potrzebne do próbkowania tekstury
void main()
{ 	 	
	//po³o¿enie w przestrzeni przyciêcia
	gl_Position = MVP*vec4(vVertex,1);
	
	//wspó³rzêdne wierzcho³ka z przestrzeni obiektu jako wyjœciowe wspó³rzêdne tekstury
	uv = vVertex;
}