#version 330 core
  
layout(location = 0) in vec3 vVertex;	//po�o�enie wierzcho�ka w przestrzeni obiektu

//uniform
uniform mat4 MVP;  //po��czona macierz modelu, widoku i rzutowania

//dane wyj�ciowe dla shadera fragment�w
smooth out vec3 uv;	//tr�jwymiarowe wsp�rz�dne potrzebne do pr�bkowania tekstury
void main()
{ 	 	
	//po�o�enie w przestrzeni przyci�cia
	gl_Position = MVP*vec4(vVertex,1);
	
	//wsp�rz�dne wierzcho�ka z przestrzeni obiektu jako wyj�ciowe wsp�rz�dne tekstury
	uv = vVertex;
}