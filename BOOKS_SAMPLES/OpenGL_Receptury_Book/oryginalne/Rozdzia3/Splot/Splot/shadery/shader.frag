#version 330 core
 
layout(location=0) out vec4 vFragColor;		//wyjœcie shadera fragmentów

//wejœcie z shadera wierzcho³ków
smooth in vec2 vUV;							//wspó³rzêdne tekstury 2D

//uniform
uniform sampler2D textureMap;				//obraz do wyœwietlenia
 
void main()
{	 
	//próbkowanie tekstury w podanych wspó³rzêdnych teksturowych, aby uzyskaæ kolor fragmentu
    vFragColor = texture(textureMap, vUV); 
}