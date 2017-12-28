#version 330 core

uniform sampler2DRect tempTexture; //pośredni rezultat mieszania


layout(location = 0) out vec4 vFragColor; //wyjście shadera fragmentów

void main()
{
	//wyprowadzenie pośredniego rezultatu mieszania
	vFragColor = texture(tempTexture, gl_FragCoord.xy); 

	//jeśli alfa ma wartość 0, fragment jest odrzucany
	if(vFragColor.a == 0) 
		discard;
}