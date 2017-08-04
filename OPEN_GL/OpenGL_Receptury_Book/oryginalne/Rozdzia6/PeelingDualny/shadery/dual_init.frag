#version 330 core

layout(location = 0) out vec4 vFragColor; //wyjście shadera fragmentów
  
void main()
{
	//ustawienie koloru fragmentu jako -głębia i głębia
	//w kanałach czerwieni i zieleni. To w połączeniu z mieszaniem min/max
	//umożliwi jednoczesny peeling z przodu i z tyłu
	vFragColor.xy = vec2(-gl_FragCoord.z, gl_FragCoord.z);
}