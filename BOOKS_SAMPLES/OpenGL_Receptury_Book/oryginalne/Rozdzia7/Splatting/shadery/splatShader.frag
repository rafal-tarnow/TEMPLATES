#version 330 core

layout(location = 0) out vec4 vFragColor;	//wyjście shadera fragmentów

smooth in vec3 outNormal;	//normalna wierzchołka w przestrzeni oka 
							//interpolowane współrzędne tekstury pobrane z shadera wierzchołków
  
//stałe
const vec3 L = vec3(0,0,1);	//wektor światła
const vec3 V = L;			//wektor widoku
const vec4 diffuse_color = vec4(0.75,0.5,0.5,1);	//kolor rozproszony
const vec4 specular_color = vec4(1);				//kolor odblasku

void main()
{ 
	//normalizacja normalnej w przestrzeni oka
	vec3 N;
	N = normalize(outNormal);
	//pobierz współrzędne punktu i sprawdź, czy bieżący fragment jest poza
	//okręgiem
	vec2 P = gl_PointCoord*2.0 - vec2(1.0);  
	float mag = dot(P.xy,P.xy); 

	//jeśli tak, odrzuć fragment Przez to placek zostanie wyrenderowany jako okrągły punkt
	if (mag > 1) 
		discard;   

	//wyznaczanie składowch rozproszenia i odblasku.
	float diffuse = max(0, dot(N,L));	
    vec3 halfVec = normalize(L+V);
	float specular = pow(max(0, dot(halfVec,N)),400);	

	//ustal ostateczny kolor fragmentu, łącząc składowe rozproszenia i odblasku.
	vFragColor =  (specular*specular_color) + (diffuse*diffuse_color);
}	