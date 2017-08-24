#version 330 core

layout(location = 0) out vec4 vFragColor;	//wyjście shadera fragmentów

//uniformy
uniform vec4 vColor;						//stały kolor 
uniform sampler2DRect  depthTexture;		//tekstura głębi 

void main()
{
	//odczytywanie głębi z tekstury głębi
	float frontDepth = texture(depthTexture, gl_FragCoord.xy).r;

	//porównanie głębi bieżącego fragmentu z głębią zapisaną w teksturze głębi
	//jeśli jest mniejsza, fragment jest odrzucany
	if(gl_FragCoord.z <= frontDepth)
		discard;
	
	//w przeciwnym razie następuje przypisanie fragmentowi koloru zapisanego w uniformie vColor
	vFragColor = vColor;
}