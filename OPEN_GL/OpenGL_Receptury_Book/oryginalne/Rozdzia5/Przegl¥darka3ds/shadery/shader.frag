#version 330 core

layout(location=0) out vec4 vFragColor;	//wyjście shadera fragmentów

//uniformy
uniform mat4 MV;			   //macierz modelu i widoku
uniform sampler2D textureMap;  		//tekstura dla bieżącej siatki (podsiatki)
uniform float hasTexture;	  	 //jeśli ma być użyty kolor domyślny
uniform vec3 light_position;  		 //położenie światła w przestrzenie obiektu

//wejście z shadera wierzchołków
smooth in vec3 vEyeSpaceNormal;    	//normalna w przestrzeni oka pobrana z shadera wierzchołków   
smooth in vec3 vEyeSpacePosition;  	//położenie w przestrzeni oka pobrane z shadera wierzchołków
smooth in vec2 vUVout;			   //współrzędne tekstury pobrane z shadera wierzchołków

//stałe shadera
const float k0 = 1.0;		//tłumienie stałe
const float k1 = 0.0;		//tłumienie liniowe
const float k2 = 0.0;		//tłumienie kwadratowe

void main()
{ 
	//wyznaczanie położenia światła w przestrzeni oka
	vec4 vEyeSpaceLightPosition = (MV*vec4(light_position,1));
	//wyznaczanie wektora światła
	vec3 L = (vEyeSpaceLightPosition.xyz-vEyeSpacePosition);
	//wyznaczanie odległości od źródła światła
	float d = length(L);
	//normalizacja wektora światła
	L = normalize(L);
	//wyznaczanie składowej rozpraszania
	float diffuse = max(0, dot(vEyeSpaceNormal, L));	
	//wprowadzanie tłumienia
	float attenuationAmount = 1.0/(k0 + (k1*d) + (k2*d*d));
	diffuse *= attenuationAmount;
	//wyprowadzenie koloru fragmentu
	vFragColor = diffuse*mix(vec4(1), texture(textureMap, vUVout), hasTexture);
}