#version 330 core
  
layout(location=0) in vec3 vVertex;		//po³o¿enie wierzcho³ka w przestrzeni obiektu

//kolor z shadera wierzcho³ków
smooth out vec3 vColor;					//przekazywanie koloru do shadera fragmentów
//uniform 
uniform mat4 MVP;  //po³¹czona macierz modelu, widoku i rzutowania

void main()
{ 	 
	//po³o¿enie w przestrzeni przyciêcia
	gl_Position = MVP*vec4(vVertex.xyz,1);

	//pobieranie koloru z po³o¿enia wierzcho³ka w przestrzeni obiektu przez dodanie przesuniêcia
    vColor = vVertex+0.5;
}