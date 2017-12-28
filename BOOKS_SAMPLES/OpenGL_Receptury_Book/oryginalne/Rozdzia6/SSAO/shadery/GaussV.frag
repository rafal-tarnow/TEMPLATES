#version 330 core
 
layout(location=0)  out vec4 vFragColor;	 //wyjście shadera fragmentów
smooth in vec2 vUV;		//input interpolated texture coordinate

uniform sampler2D textureMap; //obraz wejściowy do rozmycia

//stałe wartości jądra wygładzania gaussowskiego
const float kernel[]=float[21] (0.000272337,  0.00089296, 0.002583865, 0.00659813,  0.014869116,
								0.029570767, 0.051898313, 0.080381679, 0.109868729, 0.132526984, 
								0.14107424,  0.132526984, 0.109868729, 0.080381679, 0.051898313, 
								0.029570767, 0.014869116, 0.00659813,  0.002583865, 0.00089296, 0.000272337);
 

 
void main()
{ 
	//wyznaczanie odwrotności rozmiaru tekstury
	vec2 delta = 1.0/textureSize(textureMap,0);
	vec4 color = vec4(0);
	int  index = 20;
	 
	//przejdź przez wszystkich sąsiadów i pomnóż wartość jądra przez kolor pobrany 
	//z obrazu wejściowego
	for(int i=-10;i<=10;i++) {				
		color += kernel[index--]*texture(textureMap, vUV + (vec2(0,i*delta.y)));
	} 

	//wyprowadź przefiltrowany kolor jako wyjściowy kolor fragmentu
    vFragColor =  color;	
}