#version 330 core

layout(location=0) out vec4 vFragColor;	//wyjœcie shadera fragmentów

//uniformy
uniform vec3 light_position;	//po³o¿enie œwiat³a w przestrzeni oka
uniform vec3 spot_direction;	//kierunek reflektora w przestrzeni oka
uniform float spot_cutoff;		//k¹t odciêcia
uniform float spot_exponent;	//wyk³adnik t³umienia k¹towego
uniform vec3 diffuse_color;		//kolor œwiat³a rozpraszanego

//dane wejœciowe z shadera wierzcho³ków
smooth in vec3 vEyeSpaceNormal;	//interpolowany wektor normalny w przestrzeni oka      
smooth in vec3 vEyeSpacePosition;	//interpolowane po³o¿enie wierzcho³ka w przestrzeni oka

//sta³e shadera
const float k0 = 1.0;	//wygaszanie sta³e
const float k1 = 0.0;	//wygaszanie liniowe
const float k2 = 0.0;	//wygaszanie kwadratowe
 
void main() { 
	//wyznaczanie wektora œwiat³a
	vec3 L = (light_position.xyz-vEyeSpacePosition);
	//Wyznaczanie odleg³oœci do Ÿród³a œwiat³a
	float d = length(L);
	//normalizacja wektora œwiat³a
	L = normalize(L);
	//normalizacja kierunku reflektora
	vec3 D = normalize(spot_direction);

	//wyznaczanie k¹ta miêdzy kierunkami reflektora i œwiat³a
	vec3 V = -L;
	float diffuse = 1;	  
	float spotEffect = dot(V,D);
	
	//jeœli k¹t ten jest wiêkszy od k¹ta odciêcia, cieniujemy powierzchniê
	if(spotEffect > spot_cutoff) {
		//obliczanie sk³adowej rozproszenia
		diffuse = max(0, dot(vEyeSpaceNormal, L));	
		//k¹towe wygaszanie œwiat³a 
		spotEffect = pow(spotEffect, spot_exponent);
		//zanikanie œwiat³a wraz z odleg³oœci¹
		float attenuationAmount = spotEffect/(k0 + (k1*d) + (k2*d*d));
		diffuse *=  attenuationAmount;
	   //wyprowadzenie iloczynu sk³adowej rozproszenia i koloru rozproszenia 
	   //jako kolor bie¿¹cego fragmentu
		vFragColor = diffuse*vec4(diffuse_color,1);	 
	}  
	else {
        vFragColor =  vec4(0,0,0,0);
    } 
}