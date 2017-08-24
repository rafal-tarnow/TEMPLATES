#version 330 core
  
layout (location = 0 ) in vec3 vVertex; //po³o¿enie wierzcho³ka w przestrzeni obiektu

//uniformy
uniform mat4 MVP;					//po³¹czona macierz modelu, widoku i rzutowania
uniform ivec2 HALF_TERRAIN_SIZE;	//po³ówkowy rozmiar terenu
uniform sampler2D heightMapTexture;	//tekstura mapy wysokoœci
uniform float scale;				//skala tekstury
uniform float half_scale;			//skala po³ówkowa

void main()
{   
	//pobieranie wysokoœci dla bie¿¹cego wierzcho³ka 
	//skalowanie wysokoœci przy u¿yciu uniformów scale i half_scale
	float height = texture(heightMapTexture, vVertex.xz).r*scale - half_scale;

	//przesuwanie wierzcho³ków do obszaru od -HALF_TERRAIN_SIZE do HALF_TERRAIN_SIZE
	//wzd³u¿ obu osi uk³adu wspó³rzêdnych
	vec2 pos = (vVertex.xz*2.0-1)*HALF_TERRAIN_SIZE;

	//mno¿enie macierzy modelu, widoku i rzutowania przez przeskalowane po³ozenie i wysokoœæ 
	gl_Position = MVP*vec4(pos.x, height, pos.y, 1);			
}