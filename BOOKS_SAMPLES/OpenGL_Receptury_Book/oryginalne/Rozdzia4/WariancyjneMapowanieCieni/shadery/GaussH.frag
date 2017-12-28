#version 330 core
 
layout(location=0) out vec4 vFragColor;//wyjœcie shadera

smooth in vec2 vUV;	//interpolowane wejœciowe wspó³rzêdne tekstury 

//uniform
uniform sampler2D textureMap;	//tekstura do wyg³adzenia

//sta³e wartoœci j¹dra dla wyg³adzania gaussowskiego
const float kernel[]=float[21] (0.000272337,  0.00089296, 0.002583865, 0.00659813,  0.014869116,
								0.029570767, 0.051898313, 0.080381679, 0.109868729, 0.132526984, 
								0.14107424,  0.132526984, 0.109868729, 0.080381679, 0.051898313, 
								0.029570767, 0.014869116, 0.00659813,  0.002583865, 0.00089296, 0.000272337);
 
void main()
{ 
	//odwrotnoœæ rozmiaru tekstury
	vec2 delta = 1.0/textureSize(textureMap,0);
	vec4 color = vec4(0);
	int  index = 20;
	 
	//mno¿enie wartoœci j¹dra przez kolor pobrany z wejœciowej tekstury w najbli¿szym otoczeniu  
	for(int i=-10;i<=10;i++) {				
		color += kernel[index--]*texture(textureMap, vUV + (vec2(i*delta.x,0)));
	}
	  
	//przefiltrowany kolor jako kolor bie¿¹cego fragmentu
    vFragColor =  vec4(color.xy,0,0);	
}