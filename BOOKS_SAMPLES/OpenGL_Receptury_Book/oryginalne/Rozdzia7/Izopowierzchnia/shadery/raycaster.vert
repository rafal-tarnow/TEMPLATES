#version 330 core
  
layout(location = 0) in vec3 vVertex; //położenie wierzchołka w przestrzeni obiektu

//uniform
uniform mat4 MVP;   //połączona macierz modelu, widoku i rzutowania

smooth out vec3 vUV; //współrzędne tekstury 3D potrzebne do jej próbkowania w shaderze fragmentów

void main()
{  
	//wyznaczanie położenia wierzchołka w przestrzeni przycięcia 
	gl_Position = MVP*vec4(vVertex.xyz,1);

	//tworzenie współrzędnych tekstury przez dodawanie wektora (0.5,0.5,0.5) do położenia wierzchołka w przestrzeni obiektu 
	// Ponieważ sześcian jednostkowy ma środek w początku układu współrzędnych  (-0.5,-0.5,-0.5) and max: (0.5,0.5,0.5))
	//dodanie wektora (0.5,0.5,0.5) przenosi współrzędne jego wierzchołków do przedziału od (0,0,0) do  
	//(1,1,1)
	vUV = vVertex + vec3(0.5);
}