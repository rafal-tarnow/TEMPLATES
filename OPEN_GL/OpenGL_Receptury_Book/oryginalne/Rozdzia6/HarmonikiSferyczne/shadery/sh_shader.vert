#version 330 core
 
layout(location = 0) in vec3 vVertex;	//położenie wierzchołka w przestrzeni obiektu
layout(location = 1) in vec3 vNormal;	//normalna w przestrzeni obiektu
layout(location = 2) in vec2 vUV;		//współrzędne tekstury
 
//wyjścia do shadera fragmentów
smooth out vec2 vUVout;			//współrzędne tekstury
smooth out vec4 diffuse;		//kolor rozproszony

//uniformy shadera
uniform mat4 P;					//projection matrix
uniform mat4 MV;				//macierz modelu i widoku
uniform mat3 N;					//macierz normalna
uniform vec3 light_position;	// położenie światła w przestrzeni obiektu

//stałe shadera 
//C1 i C2 są współczynnikami SH uzyskanymi przez projekcję 
//bazy harmonik sferycznych
const float C1 = 0.429043;
const float C2 = 0.511664;
const float C3 = 0.743125;
const float C4 = 0.886227;
const float C5 = 0.247708;
const float PI = 3.1415926535897932384626433832795;


//bazowe składniki próbnika HDR Campus sunset 
const vec3 L00 = vec3(.79,.94, .98);
const vec3 L1m1 = vec3(.44, .56, .70);
const vec3 L10 = vec3(-.10, -.18, -.27);
const vec3 L11 = vec3(.45, .38, .20);
const vec3 L2m2 = vec3(.18, .14, .05);
const vec3 L2m1 = vec3(-.14, -.22, -.31);
const vec3 L20 = vec3(-.39, -.40, -.36);
const vec3 L21 = vec3(.09, .07, .04);
const vec3 L22 = vec3(.67, .67, .52);
const vec3 scaleFactor = vec3(0.39/ (0.79+0.39), 0.40/(.94+0.40), 0.31/(0.98+0.31));

//stałe tłumienia
const float k0 = 1.0;	//tłumienie stałe
const float k1 = 0.0;	//tłumienie liniowe
const float k2 = 0.0;	//tłumienie kwadratowe

void main()
{
	//kopiowanie współrzędnych tekstury do zmiennej wyjściowej
	vUVout=vUV; 
	//normalizacja wektora normalnego
	vec3 tmpN = normalize(N*vNormal);  

	//wyznaczanie położenia wierzchołka i światła w przestrzeni oka
	vec4 vEyeSpaceLightPosition = (MV*vec4(light_position,1));
	vec4 vEyeSpacePosition = MV*vec4(vVertex,1);

	//wyznaczanie wektora światła
	vec3 L = (vEyeSpaceLightPosition.xyz-vEyeSpacePosition.xyz);

	//oddalenie światła
	float d = length(L);

	//normalizacja wektora światła
	L = normalize(L);

	//wyznaczanie składowej rozpraszania
	float nDotL = max(0, dot(L,tmpN));

	//wyznaczanie składowej rozproszenia za pomocą wielomianowej interpolacji  
	//współczynników SH z normalną w przestrzeni oka
	vec3 diff = C1 * L22 * (tmpN.x * tmpN.x - tmpN.y * tmpN.y) + 
				   C3 * L20 * tmpN.z * tmpN.z + 
				   C4 * L00 - 
				   C5 * L20 + 
				   2.0 * C1 * L2m2 * tmpN.x * tmpN.y + 
				   2.0 * C1 * L21 * tmpN.x * tmpN.z + 
				   2.0 * C1 * L2m1 * tmpN.y * tmpN.z + 
				   2.0 * C2 * L11 * tmpN.x +
				   2.0 * C2 * L1m1 * tmpN.y +
				   2.0 * C2 * L10 * tmpN.z;
	diff *= scaleFactor;//brings it to 0 - 1 range

	//ostateczna składowa rozproszenia jest sumą składowej rozproszenia
	//światła scenicznego i składowej obliczonej przy użyciu harmonik sferycznych
	diffuse = vec4(diff+nDotL, 1);
	
	//wprowadzanie tłumienia
	float attenuationAmount = 1.0/(k0 + (k1*d) + (k2*d*d));
	diffuse *= attenuationAmount;
    
	//wyznaczanie położenia wierzchołka w przestrzeni przycięcia
	gl_Position = P*(vEyeSpacePosition);
}