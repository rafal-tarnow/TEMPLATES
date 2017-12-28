#version 330 core
 
layout(location=0) out vec4 vFragColor;	//wyjœcie shadera fragmentów

//dane z shadera wierzcho³ków
smooth in vec2 vUV;						//wspó³rzêdne tekstury 2D

//uniform shadera
uniform sampler2D textureMap;			//obraz filtrowany


//j¹dro wyostrzaj¹ce 3x3  
const float kernel[]=float[9] (-1,-1,-1,
							   -1,8,-1,
							   -1,-1,-1);
 

//j¹dro wyg³adzaj¹ce 3x3 
/*
const float kernel[]=float[9] (1,1,1,
								1,1,1,
								1,1,1);
*/
 
//j¹dro rozmycia gaussowskiego 3x3  
/*
const float kernel[]=float[9] (0,1,0,
								1,5,1,
								0,1,0);
*/ 

 
//j¹dra wyt³aczaj¹ce 
/*
const float kernel[]=float[9] (-4,-4, 0,		//wyt³aczanie w kierunku NW 
								 -4, 12, 0,
								  0, 0, 0);
 
*/
/*
const float kernel[]=float[9] ( 0,-4,-4,		//wyt³aczanie w kierunku NE 
								  0,12,-4,
								  0, 0, 0);
*/
/*
const float kernel[]=float[9] (0, 0, 0,			//wyt³aczanie w kierunku SE 
								 0, 12,-4,
								 0,-4,-4);
*/
/*
const float kernel[]=float[9] (  0, 0, 0,		//wyt³aczanie w kierunku SW 
								  -4, 12, 0,
								  -4,-4, 0);
*/

void main()
{ 
	//wyznaczanie odwrotnoœci rozmiaru tekstury
	vec2 delta = 1.0/textureSize(textureMap,0);
	vec4 color = vec4(0);
	int  index = 8;

	//¿eby wykonaæ splot, pobieramy próbki obrazu z otoczenia bie¿¹cego piksela
	//i sumujemy iloczyny wspó³czynników j¹dra splotu z odpowiadaj¹cymi im próbkami obrazu. 
	//Otrzyman¹ sumê dzielimy przez liczbê pikseli w analizowanym s¹siedztwie
	// i wynik dodajemy do koloru bie¿¹cego fragmentu. W przypadku operacji wyg³adzania
	//dodawanie to jest pomijane, a uzyskany wynik splotu jest bezposrednio wstawiany zamiast 
	//koloru bie¿¹cego fragmentu.

	//dla j¹der wyostrzaj¹cych i wyt³aczaj¹cych	 
	for(int j=-1;j<=1;j++) {
		for(int i=-1;i<=1;i++) {				
			color += kernel[index--]*texture(textureMap, vUV+ (vec2(i,j)*delta));
		}
	}
	color/=9.0;
	vFragColor =  color + texture(textureMap, vUV); 
	
	 
	/*
	//dla j¹der wyg³adzaj¹cych	
	for(int j=-1;j<=1;j++) {
		for(int i=-1;i<=1;i++) {				
			color += kernel[index--]*texture(textureMap, vUV+ (vec2(i,j)*delta));
		}
	}
	color/=9.0;
    vFragColor =  color;
	*/ 
}