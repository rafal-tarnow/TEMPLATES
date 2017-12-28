#version 330 core

layout(location=0) out vec4 vFragColor;	//wyjœcie shadera fragmentów
uniform mat4 MV;				//macierz modelu i widoku
uniform vec3 light_direction;	//kierunek œwiat³a w przestrzeni obiektu
uniform vec3 diffuse_color;		//kolor œwiat³a rozpraszanego

//dane wejœciowe z shadera wierzcho³ków
smooth in vec3 vEyeSpaceNormal;	//interpolowany wektor normalny w przestrzeni oka      

void main() { 
	//mno¿enie kierunku œwiat³a z przestrzeni obiektu przez macierz modelu i widoku 
	//w celu wyznaczenia kierunku swiat³a w przestrzeni oka
	vec4 vEyeSpaceLightDirection = MV*vec4(light_direction,0);
	//normalizacja kierunku œwiat³a w celu uzyskania wektora œwiat³a 
	vec3 L = normalize(vEyeSpaceLightDirection.xyz); 
	//obliczanie sk³adowej rozproszenia
	float diffuse = max(0, dot(vEyeSpaceNormal, L));	 
	//wyprowadzenie iloczynu sk³adowej rozproszenia i koloru rozproszenia 
	//jako koloru fragmentu
	vFragColor =  diffuse*vec4(diffuse_color,1);	 
}