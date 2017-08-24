#version 330 core
  
layout(location=0) in vec3 vVertex;		//po�o�enie wierzcho�ka w przestrzeni obiektu

//kolor z shadera wierzcho�k�w
smooth out vec3 vColor;					//przekazywanie koloru do shadera fragment�w
//uniform 
uniform mat4 MVP;  //po��czona macierz modelu, widoku i rzutowania

void main()
{ 	 
	//po�o�enie w przestrzeni przyci�cia
	gl_Position = MVP*vec4(vVertex.xyz,1);

	//pobieranie koloru z po�o�enia wierzcho�ka w przestrzeni obiektu przez dodanie przesuni�cia
    vColor = vVertex+0.5;
}