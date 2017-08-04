#version 330 core
  
uniform sampler2D textureMap;		//tekstura na siatce składowej
uniform float useDefault;			//0-> siatka składowa ma teksturę, 1-> siatka składowa nie ma tekstury
uniform vec3 light_position;		//położenie światła w przestrzenie obiektu
uniform mat4 MV;					//macierz modelu i widoku (użyta tutaj w celu wyznaczania położenia światła w przestrzeni oka)

smooth in vec3 vEyeSpaceNormal;    	//normalna w przestrzeni oka pobrana z shadera wierzchołków
smooth in vec3 vEyeSpacePosition;	//położenie w przestrzeni oka pobrane z shadera wierzchołków
smooth in vec2 vUVout;				//współrzędne tekstury interpolowane przez rasteryzer

layout(location=0) out vec4 vFragColor;	//wyjście shadera fragmentów

//stałe tłumienia
const float k0 = 1.0;	//tłumienie stałe
const float k1 = 0.0;	//tłumienie liniowe
const float k2 = 0.0;	//tłumienie kwadratowe

void main()
{ 
	//wyznaczanie położenia światła w przestrzeni oka
	vec4 vEyeSpaceLightPos = MV * vec4(light_position,1);

	//wyznaczanie wektora światła
	vec3 L = (vEyeSpaceLightPos.xyz-vEyeSpacePosition);

	//wyznaczanie odległości od źródła światła
	float d = length(L);

	//normalizacja wektora światła 
	L = normalize(L);

	//wyznaczanie składowej rozpraszania
	float diffuse = max(0, dot(vEyeSpaceNormal, L));	

	//wprowadzanie tłumienia
	float attenuationAmount = 1.0/(k0 + (k1*d) + (k2*d*d));
	diffuse *= attenuationAmount;

	//wyprowadzenie wypadkowego koloru
	//za pomocą funkcji mix zmieszaj kolory rozproszenia, tekstury i czystą biel
	//tam gdzie useDefault ma wartość 1, tam funkcja mix zwróci kolor biały, a w pozostałych przypadkach  
	//zwróci kolor rozproszenia+tekstury
	vFragColor = diffuse*mix(texture(textureMap, vUVout), vec4(1), useDefault);
}