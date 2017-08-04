#version 330 core
 
layout(location = 0) in vec3 vVertex;	 //położenie wierzchołka
layout(location = 1) in vec3 vNormal;	 //normalna wierzchołka
layout(location = 2) in vec2 vUV;		 //współrzędne uv wierzchołka
 
//uniformowe macierze rzutowania oraz modelu i widoku a także macierz normalna
uniform mat4 P; 
uniform mat4 MV;
uniform mat3 N;
uniform vec3 light_position; //położenie światła w przestrzenie obiektu

//wyjście do shadera fragmentów
smooth out vec2 vUVout;			//współrzędne tekstury
smooth out vec4 diffuse;		//kolor rozproszony

//stałe shadera
const float k0 = 1.0;	//tłumienie stałe
const float k1 = 0.0;	//tłumienie liniowe
const float k2 = 0.0;	//tłumienie kwadratowe

void main()
{
	//wyprowadzenie współrzędnych tekstury
	vUVout=vUV; 

	//mnożenie wektora normalnego w przestrzeni obiektu przez macierz normalną w celu 
	//normalnej w przestrzeni oka
	vec3 tmpN = normalize(N*vNormal);  

	//mnożenie położenia światła w przestrzeni obiektu przez macierz modelu i widoku 
	//w celu wyznaczania położenia światła w przestrzeni oka  
	vec4 vEyeSpaceLightPosition = (MV*vec4(light_position,1));

	//mnożenie położenia wierzchołka w przestrzeni obiektu przez macierz modelu i widoku 
	//w celu uzyskania położenia w przestrzeni oka  
	vec4 vEyeSpacePosition = MV*vec4(vVertex,1);

	//wyznaczanie wektora światła
	vec3 L = (vEyeSpaceLightPosition.xyz-vEyeSpacePosition.xyz);

	//wyznaczanie odległości od źródła światła
	float d = length(L);

	//normalizacja wektora światła
	L = normalize(L);

	//obliczenia związane z tłumieniem światła
	float attenuationAmount = 1.0/(k0 + (k1*d) + (k2*d*d));
	float nDotL = max(0, dot(L,tmpN)) * attenuationAmount; 
	diffuse = vec4(nDotL);
    
	//mnożenie położenia w przestrzenie oka przez macierz rzutowania w celu uzyskania
	//położenia w przestrzeni przycięcia
	gl_Position = P*vEyeSpacePosition;
}