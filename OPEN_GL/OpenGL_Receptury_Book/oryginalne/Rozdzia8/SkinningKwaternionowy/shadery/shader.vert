#version 330 core
 
layout(location = 0) in vec3 vVertex;				//położenie wierzchołka
layout(location = 1) in vec3 vNormal;				//normalna wierzchołka
layout(location = 2) in vec2 vUV;					//współrzędne uv wierzchołka
layout(location = 3) in vec4 vBlendWeights;			//4 wagi wiązania wierzchołka
layout(location = 4) in ivec4 viBlendIndices;		//4 indeksy wiązania wierzchołka

//uniformowe macierze rzutowania oraz modelu i widoku a także macierz normalna
uniform mat4 P; 
uniform mat4 MV;
uniform mat3 N;

//wyjście do shadera fragmentów
smooth out vec2 vUVout;						//współrzędne tekstury
smooth out vec3 vEyeSpaceNormal;    		//normalne w przestrzeni oka
smooth out vec3 vEyeSpacePosition;			//położenia w przestrzeni oka

//funkcja zwracająca macierz, a pobierająca kwaterniony zwykły (Qn) i dualny (Qd)
mat4 dualQuatToMatrix(vec4 Qn, vec4 Qd)
{	
	mat4 M;
    float len2 = dot(Qn, Qn);
    float w = Qn.w, x = Qn.x, y = Qn.y, z = Qn.z;
    float t0 = Qd.w, t1 = Qd.x, t2 = Qd.y, t3 = Qd.z;
				
    M[0][0] = w*w + x*x - y*y - z*z;
	M[0][1] = 2 * x * y + 2 * w * z;
	M[0][2] = 2 * x * z - 2 * w * y;
	M[0][3] = 0;

    M[1][0] = 2 * x * y - 2 * w * z;
    M[1][1] = w * w + y * y - x * x - z * z;
	M[1][2] = 2 * y * z + 2 * w * x;
	M[1][3] = 0;

	M[2][0] = 2 * x * z + 2 * w * y;
    M[2][1] = 2 * y * z - 2 * w * x;
    M[2][2] = w * w + z * z - x * x - y * y;
	M[2][3] = 0;

    M[3][0] = -2 * t0 * x + 2 * w * t1 - 2 * t2 * z + 2 * y * t3;
    M[3][1] = -2 * t0 * y + 2 * t1 * z - 2 * x * t3 + 2 * w * t2;
    M[3][2] = -2 * t0 * z + 2 * x * t2 + 2 * w * t3 - 2 * t1 * y;
	M[3][3] = len2;

    M /= len2;

    return M;	
}

void main()
{
	//inicjalizacja zmiennych lokalnych
	vec4 blendVertex=vec4(0);
	vec3 blendNormal=vec3(0); 
	vec4 blendDQ[2];
	
	//tutaj sprawdzamy iloczyn skalarny dwóch kwaternionów
	float yc = 1.0, zc = 1.0, wc = 1.0;
    
	//jeśli iloczyn skalarny jest <0, kwaterniony są przeciwne
    if (dot(Bones[viBlendIndices.x * 2], Bones[viBlendIndices.y * 2]) < 0.0)
		yc = -1.0;
    
    if (dot(Bones[viBlendIndices.x * 2], Bones[viBlendIndices.z * 2]) < 0.0)
       	zc = -1.0;
	
    if (dot(Bones[viBlendIndices.x * 2], Bones[viBlendIndices.w * 2]) < 0.0)
		wc = -1.0;
	
    //pobierz kwaternion dualny dla pierwszego indeksu
	//i pomnóż go przez daną wagę wiązania kości
	blendDQ[0] = Bones[viBlendIndices.x * 2] * vBlendWeights.x;
    blendDQ[1] = Bones[viBlendIndices.x * 2 + 1] * vBlendWeights.x;
    
	//pobierz kwaternion dualny dla drugiego indeksu
	//i pomnóż go przez daną wagę wiązania kości i dodaj istniejącego kwaternionu dualnego
    blendDQ[0] += yc*Bones[viBlendIndices.y * 2] * vBlendWeights.y;
    blendDQ[1] += yc*Bones[viBlendIndices.y * 2 + 1] * vBlendWeights.y;
    
	//pobierz kwaternion dualny dla trzeciego indeksu
	//i pomnóż go przez daną wagę wiązania kości i dodaj istniejącego kwaternionu dualnego
    blendDQ[0] += zc*Bones[viBlendIndices.z * 2] * vBlendWeights.z;
    blendDQ[1] += zc*Bones[viBlendIndices.z * 2 + 1] * vBlendWeights.z;
    
	//pobierz kwaternion dualny dla czwartego indeksu
	//i pomnóż go przez daną wagę wiązania kości i dodaj istniejącego kwaternionu dualnego
    blendDQ[0] += wc*Bones[viBlendIndices.w * 2] * vBlendWeights.w;
    blendDQ[1] += wc*Bones[viBlendIndices.w * 2 + 1] * vBlendWeights.w;

	//utwórz macierz skinningu na podstawie kwaternionu dualnego
	mat4 skinTransform = dualQuatToMatrix(blendDQ[0], blendDQ[1]);

	//pomnóż macierz skinningu przez dany wierzchołek i normalne w celu uzyskania  
	//wierzchołka związanego z kośćmi  
    blendVertex = skinTransform*vec4(vVertex,1);
	blendNormal = (skinTransform*vec4(vNormal,0)).xyz;

	//na koniec pomnóż blendVertex przez macierz modelu i widoku, aby uzyskać położenie w przestrzeni oka
    vEyeSpacePosition = (MV*blendVertex).xyz; 

	//pomnóż blendNormal przez macierz normalną w celu uzyskania normalnej w przestrzeni oka
    vEyeSpaceNormal   = N*blendNormal;  
	 
	//wyprowadzenie współrzędnych tekstury
	vUVout=vUV; 

	//wyznacz położenie w przestrzeni przycięcia jako iloczyn położenia w przestrzenie oka i macierzy rzutowania 
    gl_Position = P*vec4(vEyeSpacePosition,1);
}