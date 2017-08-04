#version 330 core

layout(location=0) out vec4 vFragColor;	//wyj�cie shadera fragment�w
uniform mat4 MV;				//macierz modelu i widoku
uniform vec3 light_direction;	//kierunek �wiat�a w przestrzeni obiektu
uniform vec3 diffuse_color;		//kolor �wiat�a rozpraszanego

//dane wej�ciowe z shadera wierzcho�k�w
smooth in vec3 vEyeSpaceNormal;	//interpolowany wektor normalny w przestrzeni oka      

void main() { 
	//mno�enie kierunku �wiat�a z przestrzeni obiektu przez macierz modelu i widoku 
	//w celu wyznaczenia kierunku swiat�a w przestrzeni oka
	vec4 vEyeSpaceLightDirection = MV*vec4(light_direction,0);
	//normalizacja kierunku �wiat�a w celu uzyskania wektora �wiat�a 
	vec3 L = normalize(vEyeSpaceLightDirection.xyz); 
	//obliczanie sk�adowej rozproszenia
	float diffuse = max(0, dot(vEyeSpaceNormal, L));	 
	//wyprowadzenie iloczynu sk�adowej rozproszenia i koloru rozproszenia 
	//jako koloru fragmentu
	vFragColor =  diffuse*vec4(diffuse_color,1);	 
}