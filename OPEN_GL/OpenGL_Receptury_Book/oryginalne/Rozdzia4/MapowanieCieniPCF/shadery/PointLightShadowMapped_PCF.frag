#version 330 core

layout(location=0) out vec4 vFragColor;	//wyj�cie shadera fragment�w

//uniformy
uniform mat4 MV;			//macierz modelu i widoku
uniform sampler2DShadow shadowMap;	//tekstura mapy cienia
uniform vec3 light_position;	        //po�o�enie �wiat�a w przestrzeni obiektu
uniform vec3 diffuse_color;		//kolor �wiat�a rozpraszanego
uniform bool bIsLightPass;		//znacznik przebiegu ze �wiat�em
									//jest to przebieg bez cieni

//dane wej�ciowe z shadera wierzcho�k�w
smooth in vec3 vEyeSpaceNormal;	        //interpolowany wektor normalny w przestrzeni oka      
smooth in vec3 vEyeSpacePosition;	//interpolowane po�o�enie wierzcho�ka w przestrzeni oka
smooth in vec4 vShadowCoords;		//interpolowane wsp�rz�dne cienia

//sta�e shadera
const float k0 = 1.0;	//wygaszanie sta�e
const float k1 = 0.0;	//wygaszanie liniowe
const float k2 = 0.0;	//wygaszanie kwadratowe
 
//usu� znak komentarza, aby w��czy� odpowiedni tryb PCF
//#define STRATIFIED_3x3
//#define STRATIFIED_4x4 
#define RANDOM_SAMPLING

#ifdef RANDOM_SAMPLING
//generator liczb pseudolosowych
float random(vec4 seed) {
	float dot_product = dot(seed, vec4(12.9898,78.233,45.164,94.673));
    return fract(sin(dot_product) * 43758.5453);
}
#endif

void main() { 
	//Je�li jest to przebieg ze �wiat�em, nie generujemy cieni i po prostu wychodzimy,
	//poniewa� potrzebujemy jedynie danych o g��bi, kt�re s� zaoisane w odpowiednim przy��czu FBO
	if(bIsLightPass)
		return;

	//wyznaczanie po�o�enia swiat�a w przestrzeni oka
	vec4 vEyeSpaceLightPosition = MV*vec4(light_position,1);

	//wyznaczanie wektora �wiat�a
	vec3 L = (vEyeSpaceLightPosition.xyz-vEyeSpacePosition);

	//Wyznaczanie odleg�o�ci do �r�d�a �wiat�a
	float d = length(L);

	//normalizacja wektora �wiat�a
	L = normalize(L);

	//obliczanie sk�adowej rozproszenia i wprowadzenie wygaszania
	float attenuationAmount = 1.0/(k0 + (k1*d) + (k2*d*d));
	float diffuse = max(0, dot(vEyeSpaceNormal, L)) * attenuationAmount;	
	 
	//je�li wsp�rz�dna w jest > 1, jeste�my w po��wce przedniej
	//i nale�y wygenerowa� cie�. Je�li usuniesz ten warunek, zobaczysz 
	//cienie po obu stronach �r�d�a �wiat�a, gdy b�dzie ono blisko pod�o�a  
	//Spr�buj to zrobi�, a zobaczysz, co mam na my�li.
	if(vShadowCoords.w>1) {

		//Dla PCF bierzemy kilka pr�bek mapy cienia 
		//i u�redniamy ich warto�ci. Wynik jest potem u�ywany do 
		//ustalenia jasno�ci fragmentu. Do pobierania pr�bek s�u�y
		//funkcja textureProjOffset, kt�ra przyjmuje przesuni�cie z bie��cego
		//punktu mapy cienia i zwraca warto�� pobranej pr�bki.

		float sum = 0;
		float shadow = 1;

		//s�siedztwo 3x3 
		#ifdef STRATIFIED_3x3
		sum += textureProjOffset(shadowMap, vShadowCoords, ivec2(-2,-2));
		sum += textureProjOffset(shadowMap, vShadowCoords, ivec2(-2, 0));
		sum += textureProjOffset(shadowMap, vShadowCoords, ivec2(-2, 2));

		sum += textureProjOffset(shadowMap, vShadowCoords, ivec2( 0,-2));
		sum += textureProjOffset(shadowMap, vShadowCoords, ivec2( 0, 0));
		sum += textureProjOffset(shadowMap, vShadowCoords, ivec2( 0, 2));

		sum += textureProjOffset(shadowMap, vShadowCoords, ivec2( 2,-2));
		sum += textureProjOffset(shadowMap, vShadowCoords, ivec2( 2, 0));
		sum += textureProjOffset(shadowMap, vShadowCoords, ivec2( 2, 2));
		shadow = sum/9.0;
		#endif
		
		//s�siedztwo 4x4 
		#ifdef STRATIFIED_4x4
		sum += textureProjOffset(shadowMap, vShadowCoords, ivec2(-2,-2));
		sum += textureProjOffset(shadowMap, vShadowCoords, ivec2(-1,-2));
		sum += textureProjOffset(shadowMap, vShadowCoords, ivec2( 1,-2));
		sum += textureProjOffset(shadowMap, vShadowCoords, ivec2( 2,-2));

		sum += textureProjOffset(shadowMap, vShadowCoords, ivec2(-2,-1));
		sum += textureProjOffset(shadowMap, vShadowCoords, ivec2(-1,-1));
		sum += textureProjOffset(shadowMap, vShadowCoords, ivec2( 1,-1));
		sum += textureProjOffset(shadowMap, vShadowCoords, ivec2( 2,-1));

		sum += textureProjOffset(shadowMap, vShadowCoords, ivec2(-2, 1));
		sum += textureProjOffset(shadowMap, vShadowCoords, ivec2(-1, 1));
		sum += textureProjOffset(shadowMap, vShadowCoords, ivec2( 1, 1));
		sum += textureProjOffset(shadowMap, vShadowCoords, ivec2( 2, 1));
 
		sum += textureProjOffset(shadowMap, vShadowCoords, ivec2(-2, 2));
		sum += textureProjOffset(shadowMap, vShadowCoords, ivec2(-1, 2));
		sum += textureProjOffset(shadowMap, vShadowCoords, ivec2( 1, 2));
		sum += textureProjOffset(shadowMap, vShadowCoords, ivec2( 2, 2));

		shadow = sum/16.0;
		#endif

		#ifdef RANDOM_SAMPLING	 
		for(int i=0;i<16;i++) {
			float indexA = (random(vec4(gl_FragCoord.xyx, i))*0.25);
			float indexB = (random(vec4(gl_FragCoord.yxy, i))*0.25); 
			sum += textureProj(shadowMap, vShadowCoords+vec4(indexA, indexB, 0, 0));
		}
		shadow = sum/16.0;
		#endif

		//przyciemnienie (rozja�nienie) fragmentu na podstawie warto�ci zmiennej shadow
		diffuse = mix(diffuse, diffuse*shadow, 0.5); 
	}
	//wyprowadzenie iloczynu sk�adowej rozproszenia i koloru rozproszenia 
	//jako kolor bie��cego fragmentu
	vFragColor = diffuse*vec4(diffuse_color, 1);	 
}