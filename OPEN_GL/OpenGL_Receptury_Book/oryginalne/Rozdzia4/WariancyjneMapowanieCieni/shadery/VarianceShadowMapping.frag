#version 330 core

layout(location=0) out vec4 vFragColor;	//wyj�cie shadera fragment�w

//uniformy
uniform mat4 MV;				//macierz modelu i widoku
uniform sampler2D  shadowMap;		//tekstura mapy cienia
uniform vec3 light_position;	//po�o�enie �wiat�a w przestrzeni obiektu
uniform vec3 diffuse_color;		//kolor �wiat�a rozpraszanego
									 
//dane wej�ciowe z shadera wierzcho�k�w
smooth in vec3 vEyeSpaceNormal;	        //interpolowany wektor normalny w przestrzeni oka      
smooth in vec3 vEyeSpacePosition;	//interpolowane po�o�enie wierzcho�ka w przestrzeni oka
smooth in vec4 vShadowCoords;		//interpolowane wsp�rz�dne cienia

//sta�e shadera
const float k0 = 1.0;	//wygaszanie sta�e
const float k1 = 0.0;	//wygaszanie liniowe
const float k2 = 0.0;	//wygaszanie kwadratowe

void main() {   

	//wyznaczanie po�o�enia swiat�a w przestrzeni oka
	vec4 vEyeSpaceLightPosition = (MV*vec4(light_position,1));
	
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
		
		//dzielenie wsp�rz�dnych cienia przez wsp�rz�dn� w
		vec3 uv = vShadowCoords.xyz/vShadowCoords.w;
		
		//pobieranie warto�ci g��bi
		float depth = uv.z;
		
		//odczytywanie moment�w z tekstury mapy cienia
		vec4 moments = texture(shadowMap, uv.xy); 

		//wyznaczanie wariancji z moment�w
		float E_x2 = moments.y;
		float Ex_2 = moments.x*moments.x;
		float var = E_x2-Ex_2;

		//obci�cie wariancji
		var = max(var, 0.00002);

		//odejmowanie pierwszego momentu od g��boko�ci fragmentu
		//i dzielenie wariancji przez kwadrat r�nicy
		//w celu obliczenia maksymalnego prawdopodobie�stwa, �e fragment jest w cieniu
		float mD = depth-moments.x;
		float mD_2 = mD*mD; 
		float p_max = var/(var+ mD_2); 

		//przyciemnianie sk�adowej rozproszenia, je�li bie��ca g��boko�� jest mniejsza lub r�wna
		// pierwszemu momentowi i zwracana wielko�� jest mniejsza ni� 
		//wyliczone maksymalne prawdopodobie�stwo
		diffuse *= max(p_max, (depth<=moments.x)?1.0:0.2); 
	}
	//wyprowadzenie iloczynu sk�adowej rozproszenia i koloru rozproszenia 
	//jako kolor bie��cego fragmentu
	vFragColor = diffuse*vec4(diffuse_color, 1);	 
}