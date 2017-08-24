#version 330 core

layout(location=0) out vec4 vFragColor;	//wyjœcie shadera fragmentów

//uniformy
uniform mat4 MV;				//macierz modelu i widoku
uniform sampler2DShadow shadowMap;	//tekstura mapy cienia
uniform vec3 light_position;	//po³o¿enie œwiat³a w przestrzeni obiektu
uniform vec3 diffuse_color;		//kolor œwiat³a rozpraszanego
uniform bool bIsLightPass;			//znacznik przebiegu ze œwiat³em
									//jest to przebieg bez cieni

//dane wejœciowe z shadera wierzcho³ków
smooth in vec3 vEyeSpaceNormal;	//interpolowany wektor normalny w przestrzeni oka      
smooth in vec3 vEyeSpacePosition;	//interpolowane po³o¿enie wierzcho³ka w przestrzeni oka
smooth in vec4 vShadowCoords;		//interpolowane wspó³rzêdne cienia

//sta³e shadera
const float k0 = 1.0;	//wygaszanie sta³e
const float k1 = 0.0;	//wygaszanie liniowe
const float k2 = 0.0;	//wygaszanie kwadratowe

void main() { 
	//Jeœli jest to przebieg ze œwiat³em, nie generujemy cieni i po prostu wychodzimy,
	//poniewa¿ potrzebujemy jedynie danych o g³êbi, które s¹ zaoisane w odpowiednim przy³¹czu FBO
	if(bIsLightPass)
		return;
		 
	//wyznaczanie po³o¿enia swiat³a w przestrzeni oka
	vec4 vEyeSpaceLightPosition = MV*vec4(light_position,1);
	
	//wyznaczanie wektora œwiat³a
	vec3 L = (vEyeSpaceLightPosition.xyz-vEyeSpacePosition);

	//Wyznaczanie odleg³oœci do Ÿród³a œwiat³a
	float d = length(L);

	//normalizacja wektora œwiat³a
	L = normalize(L);

	//obliczanie sk³adowej rozproszenia i wprowadzenie wygaszania
	float attenuationAmount = 1.0/(k0 + (k1*d) + (k2*d*d));
	float diffuse = max(0, dot(vEyeSpaceNormal, L)) * attenuationAmount;	
	 
	//jeœli wspó³rzêdna w jest > 1, jesteœmy w po³ówce przedniej
	//i nale¿y wygenerowaæ cieñ. Jeœli usuniesz ten warunek, zobaczysz 
	//cienie po obu stronach Ÿród³a œwiat³a, gdy bêdzie ono blisko pod³o¿a  
	//Spróbuj to zrobiæ, a zobaczysz, co mam na myœli.
	if(vShadowCoords.w>1) 
	{
		//sprawdzanie tekstury mapy cienia, aby stwierdziæ, czy bie¿¹cy fragment jest w cieniu
		float shadow = textureProj(shadowMap, vShadowCoords);
		//przyciemnienie sk³adowej rozproszenia
		diffuse = mix(diffuse, diffuse*shadow, 0.5); 
	}

	//wyprowadzenie iloczynu sk³adowej rozproszenia i koloru rozproszenia 
	//jako kolor bie¿¹cego fragmentu
	vFragColor = diffuse*vec4(diffuse_color, 1);	 
}