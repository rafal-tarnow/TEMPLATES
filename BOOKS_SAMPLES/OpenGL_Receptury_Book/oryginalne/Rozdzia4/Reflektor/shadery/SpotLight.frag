#version 330 core

layout(location=0) out vec4 vFragColor;	//wyj�cie shadera fragment�w

//uniformy
uniform vec3 light_position;	//po�o�enie �wiat�a w przestrzeni oka
uniform vec3 spot_direction;	//kierunek reflektora w przestrzeni oka
uniform float spot_cutoff;		//k�t odci�cia
uniform float spot_exponent;	//wyk�adnik t�umienia k�towego
uniform vec3 diffuse_color;		//kolor �wiat�a rozpraszanego

//dane wej�ciowe z shadera wierzcho�k�w
smooth in vec3 vEyeSpaceNormal;	//interpolowany wektor normalny w przestrzeni oka      
smooth in vec3 vEyeSpacePosition;	//interpolowane po�o�enie wierzcho�ka w przestrzeni oka

//sta�e shadera
const float k0 = 1.0;	//wygaszanie sta�e
const float k1 = 0.0;	//wygaszanie liniowe
const float k2 = 0.0;	//wygaszanie kwadratowe
 
void main() { 
	//wyznaczanie wektora �wiat�a
	vec3 L = (light_position.xyz-vEyeSpacePosition);
	//Wyznaczanie odleg�o�ci do �r�d�a �wiat�a
	float d = length(L);
	//normalizacja wektora �wiat�a
	L = normalize(L);
	//normalizacja kierunku reflektora
	vec3 D = normalize(spot_direction);

	//wyznaczanie k�ta mi�dzy kierunkami reflektora i �wiat�a
	vec3 V = -L;
	float diffuse = 1;	  
	float spotEffect = dot(V,D);
	
	//je�li k�t ten jest wi�kszy od k�ta odci�cia, cieniujemy powierzchni�
	if(spotEffect > spot_cutoff) {
		//obliczanie sk�adowej rozproszenia
		diffuse = max(0, dot(vEyeSpaceNormal, L));	
		//k�towe wygaszanie �wiat�a 
		spotEffect = pow(spotEffect, spot_exponent);
		//zanikanie �wiat�a wraz z odleg�o�ci�
		float attenuationAmount = spotEffect/(k0 + (k1*d) + (k2*d*d));
		diffuse *=  attenuationAmount;
	   //wyprowadzenie iloczynu sk�adowej rozproszenia i koloru rozproszenia 
	   //jako kolor bie��cego fragmentu
		vFragColor = diffuse*vec4(diffuse_color,1);	 
	}  
	else {
        vFragColor =  vec4(0,0,0,0);
    } 
}