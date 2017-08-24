#version 330 core
  
layout (location = 0 ) in vec3 vVertex; //po³o¿enie wierzcho³ka w przestrzeni obiektu

uniform mat4 MVP;	//po³¹czona macierz modelu, widoku i rzutowania

//wyjœcie shadera
smooth out vec4 clipSpacePos;	//po³o¿enie w przestrzeni przyciêcia
void main()
{ 	 
	//mno¿enie po³o¿enia w przestrzeni obiektu przez po³¹czon¹ macierz MVP 
	//w celu uzyskania po³o¿enia w przestrzeni przyciêcia
	gl_Position = MVP*vec4(vVertex.xyz,1);

	//wyprowadzenie po³o¿enia w przestrzeni przyciêcia
	clipSpacePos = gl_Position;
}