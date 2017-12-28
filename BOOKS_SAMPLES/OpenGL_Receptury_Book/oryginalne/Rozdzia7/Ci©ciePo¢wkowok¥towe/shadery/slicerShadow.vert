#version 330 core
  
layout(location = 0) in vec3 vVertex; //położenie wierzchołka w przestrzeni obiektu
  
//uniformy
uniform mat4 MVP;	//połączona macierz modelu, widoku i rzutowania
uniform mat4 S;		//macierz cienia

//wyjście do shadera fragmentów
smooth out vec3 vUV;		//współrzędne tekstury
smooth out vec4 vLightUVW;	//współrzędne tekstury cienia

void main()
{  
	//położenie wierzchołka w przestrzeni obiektu pomnożone przez
	//macierz cienia w celu wyznaczenia współrzędnych próbki tekstury cienia
	vLightUVW = S*vec4(vVertex,1);
	

	//położenia w przestrzeni przycięcia
	gl_Position = MVP*vec4(vVertex,1);

	//współrzędne próbki tekstury 3D
	vUV = vVertex + vec3(0.5);
}