#version 330 core

layout(location=0) out vec4 vFragColor; //wyjście shadera fragmentów
 
//uniformy
uniform sampler2D textureMap;	//mapa tekstury dla siatki
uniform float useDefault;		//jeśli ma być użyty kolor domyślny

//wejście z shadera wierzchołków
smooth in vec4 diffuse;		//ostateczny przytłumiony kolor światła rozproszonego   
smooth in vec2 vUVout;		//interpolowane współrzędne tekstury

void main()
{   
	//interpolacja między kolorem rozproszenia+mapa tekstury a kolorem rozproszenia 
	//w oparciu o wartość useDefault. Jeśli 0, zwracany jest kolor próbki tekstury kolorem rozproszenia,
	//a w przeciwnym razie - tylko kolor rozproszenia
	vFragColor = mix(texture(textureMap, vUVout)*diffuse, diffuse, useDefault);
}