#version 330 core
 
layout(location = 0) in vec3 vVertex;	//Po�o�enie wierzcho�ka w przestrzeni obiektu
layout(location = 1) in vec3 vColor;	//kolor wierzcho�ka

//wyj�cie shadera wierzcho�k�w
smooth out vec4 vSmoothColor;		//wyg�adzany kolor dla shadera fragment�w

//uniform
uniform mat4 MVP;	//po��czona macierz modelu, widoku i rzutowania
void main()
{
	//przypisanie koloru wierzcho�ka do zmiennej varying vSmoothColor 
   vSmoothColor = vec4(vColor,1);

   //wyznaczanie po�o�enia w przestrzeni przyci�cia przez mno�enie macierzy MVP wsp�rz�dne  
   //wierzcho�ka w przestrzeni obiektu
   gl_Position = MVP*vec4(vVertex,1);
}