#version 330 core

layout(location = 0) out vec4 vFragColor;	//wyjście shadera fragmentów

//uniformy
uniform sampler2DRect colorTexture;	//tekstura z poprzedniego przejścia
uniform vec4 vBackgroundColor;		//kolor tła


void main()
{
	//pobieranie koloru z bufora koloru
	vec4 color = texture(colorTexture, gl_FragCoord.xy);
	//łączenie koloru odczytanego z tekstury z kolorem tła
	//przez mnożenie wartości alfa z tekstury przez kolor tła i dodawanie 
	//tego iloczynu do koloru z tekstury
	vFragColor = color + vBackgroundColor*color.a;
}