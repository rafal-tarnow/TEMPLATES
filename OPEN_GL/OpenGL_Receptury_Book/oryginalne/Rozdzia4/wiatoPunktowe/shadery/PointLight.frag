#version 330 core

layout(location=0) out vec4 vFragColor;	//wyjœcie shadera fragmentów
 
//uniformy
uniform mat4 MV;				//macierz modelu i widoku
uniform vec3 light_position;	//po³o¿enie œwiat³a w przestrzeni obiektu
uniform vec3 diffuse_color;		//kolor œwiat³a rozpraszanego

//dane wejœciowe z shadera wierzcho³ków
smooth in vec3 vEyeSpaceNormal;	//interpolowany wektor normalny w przestrzeni oka      
smooth in vec3 vEyeSpacePosition;	//interpolowane po³o¿enie wierzcho³ka w przestrzeni oka

//sta³e shadera
const float k0 = 1.0;	//wygaszanie sta³e
const float k1 = 0.0;	//wygaszanie liniowe
const float k2 = 0.0;	//wygaszanie kwadratowe

void main() { 
	//mno¿enie po³o¿enia œwiat³a z przestrzeni obiektu przez macierz modelu i widoku 
	//w celu wyznaczenia po³o¿enia swiat³a w przestrzeni oka
	vec3 vEyeSpaceLightPosition = (MV*vec4(light_position,1)).xyz;
	//wyznaczanie wektora œwiat³a
	vec3 L = (vEyeSpaceLightPosition-vEyeSpacePosition);
	//Wyznaczanie odleg³oœci do Ÿród³a œwiat³a
	float d = length(L);
	//normalizacja wektora œwiat³a
	L = normalize(L);
	//obliczanie sk³adowej rozproszenia
	float diffuse = max(0, dot(vEyeSpaceNormal, L));	
	//wprowadzenie wygaszania
	float attenuationAmount = 1.0/(k0 + (k1*d) + (k2*d*d));
	diffuse *= attenuationAmount;
	//wyprowadzenie iloczynu sk³adowej rozproszenia i koloru rozproszenia 
	//jako kolor bie¿¹cego fragmentu
	vFragColor = diffuse*vec4(diffuse_color,1);	 
}