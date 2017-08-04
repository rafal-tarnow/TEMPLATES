#version 330 core

layout(location=0) out vec4 vFragColor;	//wyj�cie shadera fragment�w
 
//uniformy
uniform mat4 MV;				//macierz modelu i widoku
uniform vec3 light_position;	//po�o�enie �wiat�a w przestrzeni obiektu
uniform vec3 diffuse_color;		//kolor �wiat�a rozpraszanego

//dane wej�ciowe z shadera wierzcho�k�w
smooth in vec3 vEyeSpaceNormal;	//interpolowany wektor normalny w przestrzeni oka      
smooth in vec3 vEyeSpacePosition;	//interpolowane po�o�enie wierzcho�ka w przestrzeni oka

//sta�e shadera
const float k0 = 1.0;	//wygaszanie sta�e
const float k1 = 0.0;	//wygaszanie liniowe
const float k2 = 0.0;	//wygaszanie kwadratowe

void main() { 
	//mno�enie po�o�enia �wiat�a z przestrzeni obiektu przez macierz modelu i widoku 
	//w celu wyznaczenia po�o�enia swiat�a w przestrzeni oka
	vec3 vEyeSpaceLightPosition = (MV*vec4(light_position,1)).xyz;
	//wyznaczanie wektora �wiat�a
	vec3 L = (vEyeSpaceLightPosition-vEyeSpacePosition);
	//Wyznaczanie odleg�o�ci do �r�d�a �wiat�a
	float d = length(L);
	//normalizacja wektora �wiat�a
	L = normalize(L);
	//obliczanie sk�adowej rozproszenia
	float diffuse = max(0, dot(vEyeSpaceNormal, L));	
	//wprowadzenie wygaszania
	float attenuationAmount = 1.0/(k0 + (k1*d) + (k2*d*d));
	diffuse *= attenuationAmount;
	//wyprowadzenie iloczynu sk�adowej rozproszenia i koloru rozproszenia 
	//jako kolor bie��cego fragmentu
	vFragColor = diffuse*vec4(diffuse_color,1);	 
}