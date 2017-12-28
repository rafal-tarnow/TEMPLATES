#version 330 core
 
layout (location=0) out vec4 vFragColor;	//wyj�cie shadera fragment�w
//wej�cie z shadera wierzcho�k�w
smooth in vec2 vUV;			//interpolowane wsp�rz�dne tekstury 2D

//uniform
uniform sampler2D textureMap;	//mapa tekstury

void main()
{	
   	vec4 color = vec4(0);
	//wyznaczanie odwrotno�ci rozmiaru tekstury
	vec2 delta = 1.0/textureSize(textureMap,0);

	//p�tla po najbli�szym otoczeniu
	for(int j=-3;j<=3;j++) {
		for(int i=-3;i<=3;i++) {	
			//sumowanie pr�bek z s�siedztwa
			color += texture(textureMap, vUV+ (vec2(i,j)*delta));
		}
	}
	//dzielenie przez liczb� pr�bek
	color/=49.0;
	//zwracanie wyliczonego koloru
    vFragColor =  color;
}