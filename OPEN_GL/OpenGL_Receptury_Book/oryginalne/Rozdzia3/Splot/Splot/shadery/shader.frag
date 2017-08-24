#version 330 core
 
layout(location=0) out vec4 vFragColor;		//wyj�cie shadera fragment�w

//wej�cie z shadera wierzcho�k�w
smooth in vec2 vUV;							//wsp�rz�dne tekstury 2D

//uniform
uniform sampler2D textureMap;				//obraz do wy�wietlenia
 
void main()
{	 
	//pr�bkowanie tekstury w podanych wsp�rz�dnych teksturowych, aby uzyska� kolor fragmentu
    vFragColor = texture(textureMap, vUV); 
}