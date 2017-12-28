#version 330 core
 
layout (location=0) out vec4 vFragColor;	//wyjœcie shadera fragmentów
//wejœcie z shadera wierzcho³ków
smooth in vec2 vUV;			//interpolowane wspó³rzêdne tekstury 2D

//uniform
uniform sampler2D textureMap;	//mapa tekstury

void main()
{	
   	vec4 color = vec4(0);
	//wyznaczanie odwrotnoœci rozmiaru tekstury
	vec2 delta = 1.0/textureSize(textureMap,0);

	//pêtla po najbli¿szym otoczeniu
	for(int j=-3;j<=3;j++) {
		for(int i=-3;i<=3;i++) {	
			//sumowanie próbek z s¹siedztwa
			color += texture(textureMap, vUV+ (vec2(i,j)*delta));
		}
	}
	//dzielenie przez liczbê próbek
	color/=49.0;
	//zwracanie wyliczonego koloru
    vFragColor =  color;
}