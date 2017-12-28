#version 330 core

uniform sampler2DRect depthBlenderTex;	//wyjście mieszania głębi
uniform sampler2DRect frontBlenderTex;	//wyjście mieszania przedniego
uniform sampler2DRect backBlenderTex;	//wyjście mieszania tylnego

layout(location = 0) out vec4 vFragColor; //wyjście shadera fragmentów

void main()
{
	//pobieranie kolorów z tekstur mieszania przedniego i tylnego
	vec4 frontColor = texture(frontBlenderTex, gl_FragCoord.xy);
	vec3 backColor = texture(backBlenderTex, gl_FragCoord.xy).rgb; 

	//przód + tył
	//łączenie rezultatów mieszania przedniego i tylnego
	vFragColor.rgb = frontColor.rgb + backColor * frontColor.a;
	
	//mieszanie przednie
	//vFragColor.rgb = frontColor + vec3(alphaMultiplier);
	
	//mieszanie tylne
	//vFragColor.rgb = backColor;
}