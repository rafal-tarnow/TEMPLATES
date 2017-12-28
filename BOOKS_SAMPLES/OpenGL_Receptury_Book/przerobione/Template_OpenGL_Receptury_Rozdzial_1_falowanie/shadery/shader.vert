#version 330 core
  
layout(location=0) in vec3 vVertex;		//położenie wierzchołka w przestrzeni obiektu

//uniformy
uniform mat4 MVP;		//połączona macierz rzutowania
uniform float mtime;		//upływający czas

//stałe shadera
const float amplitude = 0.125;
const float frequency = 4;
const float PI = 3.14159;

void main()
{ 
	//obliczanie odległości bieżącego wierzchołka od środka siatki
	float distance = length(vVertex);  
	//obliczanie wartości funkcji falowej w zależności od odległości, częstotliwości, amplitudy i czasu
	float y = amplitude*sin(-PI*distance*frequency+mtime);
	//mnożenie nowego położenia porzez macierz MVP w celu uzyskania położenia w przestrzeni przycięcia
	gl_Position = MVP*vec4(vVertex.x, y, vVertex.z,1);
}
