#version 330 core
precision highp float;

layout (location=0) in vec4 position;	//położenie cząstki w przestrzenie obiektu

uniform mat4 MVP;						//połączona macierz modelu, widoku i rzutowania

smooth out vec4 color;					//wyjściowy kolor dla shadera fragmentów 

//kolory mapy kolorów
const vec3 RED = vec3(1,0,0);
const vec3 GREEN = vec3(0,1,0);
const vec3 YELLOW = vec3(1,1,0); 

void main() 
{  
	//wyznacz położenie wierzchołka w przestrzeni przycięcia
	gl_Position = MVP*vec4(position.xyz, 1.0);	
	//wyznacz wartość zmiennej t do interpolowania barw z mapy kolorów 
	//uzależnij ją od odległości od centrum. Jeśli wyjdzie 
	//więcej niż 3, przypisz cząsteczce kolor żółty
	float t =  1.0- length(position.xyz)/3.0;	
	color = vec4(mix(YELLOW, RED, t), t);
}