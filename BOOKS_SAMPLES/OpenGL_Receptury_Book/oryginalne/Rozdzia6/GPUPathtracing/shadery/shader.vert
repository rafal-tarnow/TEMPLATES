#version 330 core
 
layout(location = 0) in vec3 vVertex;	//położenie wierzchołka
layout(location = 1) in vec3 vNormal;	//normalna wierzchołka
layout(location = 2) in vec2 vUV;		//współrzędne uv wierzchołka
 
//uniformowe macierze rzutowania oraz modelu i widoku a także macierz normalna
uniform mat4 P; 
uniform mat4 MV;
uniform mat3 N;

//wyjście do shadera fragmentów
smooth out vec2 vUVout;						//współrzędne tekstury
smooth out vec3 vEyeSpaceNormal;    		//normalne w przestrzeni oka
smooth out vec3 vEyeSpacePosition;			//położenia w przestrzeni oka

void main()
{
	//wyprowadzenie współrzędnych tekstury
	vUVout=vUV; 

	//mnożenie położenia wierzchołka w przestrzeni obiektu przez macierz modelu i widoku 
	//w celu uzyskania położenia w przestrzeni oka  
	vEyeSpacePosition = (MV*vec4(vVertex,1)).xyz; 

	//mnożenie wektora normalnego w przestrzeni obiektu przez macierz normalną w celu 
	//normalnej w przestrzeni oka
	vEyeSpaceNormal   = N*vNormal;

	//mnożenie położenia w przestrzenie oka przez macierz rzutowania w celu uzyskania
	//położenia w przestrzeni przycięcia
	gl_Position = P*vec4(vEyeSpacePosition,1);
}