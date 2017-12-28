#version 330 core
 
layout(location=0) out vec4 vFragColor;	//wyj�cie shadera fragment�w

//dane z shadera wierzcho�k�w
smooth in vec2 vUV;						//wsp�rz�dne tekstury 2D

//uniform shadera
uniform sampler2D textureMap;			//obraz filtrowany


//j�dro wyostrzaj�ce 3x3  
const float kernel[]=float[9] (-1,-1,-1,
							   -1,8,-1,
							   -1,-1,-1);
 

//j�dro wyg�adzaj�ce 3x3 
/*
const float kernel[]=float[9] (1,1,1,
								1,1,1,
								1,1,1);
*/
 
//j�dro rozmycia gaussowskiego 3x3  
/*
const float kernel[]=float[9] (0,1,0,
								1,5,1,
								0,1,0);
*/ 

 
//j�dra wyt�aczaj�ce 
/*
const float kernel[]=float[9] (-4,-4, 0,		//wyt�aczanie w kierunku NW 
								 -4, 12, 0,
								  0, 0, 0);
 
*/
/*
const float kernel[]=float[9] ( 0,-4,-4,		//wyt�aczanie w kierunku NE 
								  0,12,-4,
								  0, 0, 0);
*/
/*
const float kernel[]=float[9] (0, 0, 0,			//wyt�aczanie w kierunku SE 
								 0, 12,-4,
								 0,-4,-4);
*/
/*
const float kernel[]=float[9] (  0, 0, 0,		//wyt�aczanie w kierunku SW 
								  -4, 12, 0,
								  -4,-4, 0);
*/

void main()
{ 
	//wyznaczanie odwrotno�ci rozmiaru tekstury
	vec2 delta = 1.0/textureSize(textureMap,0);
	vec4 color = vec4(0);
	int  index = 8;

	//�eby wykona� splot, pobieramy pr�bki obrazu z otoczenia bie��cego piksela
	//i sumujemy iloczyny wsp�czynnik�w j�dra splotu z odpowiadaj�cymi im pr�bkami obrazu. 
	//Otrzyman� sum� dzielimy przez liczb� pikseli w analizowanym s�siedztwie
	// i wynik dodajemy do koloru bie��cego fragmentu. W przypadku operacji wyg�adzania
	//dodawanie to jest pomijane, a uzyskany wynik splotu jest bezposrednio wstawiany zamiast 
	//koloru bie��cego fragmentu.

	//dla j�der wyostrzaj�cych i wyt�aczaj�cych	 
	for(int j=-1;j<=1;j++) {
		for(int i=-1;i<=1;i++) {				
			color += kernel[index--]*texture(textureMap, vUV+ (vec2(i,j)*delta));
		}
	}
	color/=9.0;
	vFragColor =  color + texture(textureMap, vUV); 
	
	 
	/*
	//dla j�der wyg�adzaj�cych	
	for(int j=-1;j<=1;j++) {
		for(int i=-1;i<=1;i++) {				
			color += kernel[index--]*texture(textureMap, vUV+ (vec2(i,j)*delta));
		}
	}
	color/=9.0;
    vFragColor =  color;
	*/ 
}