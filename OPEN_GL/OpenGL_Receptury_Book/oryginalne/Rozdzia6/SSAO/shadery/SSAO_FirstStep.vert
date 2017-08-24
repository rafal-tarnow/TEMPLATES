#version 330 core
 
layout(location = 0) in vec3 vVertex;	//położenie wierzchołka w przestrzeni obiektu
layout(location = 1) in vec3 vNormal;	//normalna w przestrzeni obiektu
 
//uniformy 
uniform mat4 MVP;	//połączona macierz modelu, widoku i rzutowania
uniform mat3 N;		//macierz normalna

smooth out vec3 vEyeSpaceNormal;   //normalna w przestrzeni oka  

void main()
{     
	//wyznaczanie normalnej w przestrzeni oka jako iloczynu normalnej w przestrzeni obiektu
	//i macierzy normalnej
	vEyeSpaceNormal = N*vNormal;  
	//wyznaczanie położenia wierzchołka w przestrzeni przycięcia
	gl_Position = MVP*vec4(vVertex,1); 
}