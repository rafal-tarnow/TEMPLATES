#version 330 core
 
layout(location = 0) in vec3 vVertex;	//Po³o¿enie wierzcho³ka w przestrzeni obiektu
layout(location = 1) in vec3 vColor;	//kolor wierzcho³ka

//wyjœcie shadera wierzcho³ków
smooth out vec4 vSmoothColor;		//wyg³adzany kolor dla shadera fragmentów

//uniform
uniform mat4 MVP;	//po³¹czona macierz modelu, widoku i rzutowania
void main()
{
	//przypisanie koloru wierzcho³ka do zmiennej varying vSmoothColor 
   vSmoothColor = vec4(vColor,1);

   //wyznaczanie po³o¿enia w przestrzeni przyciêcia przez mno¿enie macierzy MVP wspó³rzêdne  
   //wierzcho³ka w przestrzeni obiektu
   gl_Position = MVP*vec4(vVertex,1);
}