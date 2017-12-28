#version 330 core
  
layout (location = 0 ) in vec3 vVertex; //po�o�enie wierzcho�ka w przestrzeni obiektu

//uniformy
uniform mat4 MVP;					//po��czona macierz modelu, widoku i rzutowania
uniform ivec2 HALF_TERRAIN_SIZE;	//po��wkowy rozmiar terenu
uniform sampler2D heightMapTexture;	//tekstura mapy wysoko�ci
uniform float scale;				//skala tekstury
uniform float half_scale;			//skala po��wkowa

void main()
{   
	//pobieranie wysoko�ci dla bie��cego wierzcho�ka 
	//skalowanie wysoko�ci przy u�yciu uniform�w scale i half_scale
	float height = texture(heightMapTexture, vVertex.xz).r*scale - half_scale;

	//przesuwanie wierzcho�k�w do obszaru od -HALF_TERRAIN_SIZE do HALF_TERRAIN_SIZE
	//wzd�u� obu osi uk�adu wsp�rz�dnych
	vec2 pos = (vVertex.xz*2.0-1)*HALF_TERRAIN_SIZE;

	//mno�enie macierzy modelu, widoku i rzutowania przez przeskalowane po�ozenie i wysoko�� 
	gl_Position = MVP*vec4(pos.x, height, pos.y, 1);			
}