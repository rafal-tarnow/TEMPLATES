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
smooth out vec2 vUVout;					//współrzędne tekstury
smooth out vec3 vEyeSpaceNormal;		//normalne w przestrzeni oka
smooth out vec3 vEyeSpacePosition;		//położenia w przestrzeni oka

void main()
{
	//inicjalizacja zmiennych lokalnych
	vec4 blendVertex=vec4(0);
	vec3 blendNormal=vec3(0);
	vec4 vVertex4 = vec4(vVertex,1);

	//pobierz pierwszy indeks
	int index = viBlendIndices.x;
	
	//pobierz macierz kości dla pierwszego indeksu 
	//i pomnóż ją przez dany wierzchołek oraz wagę wiązania kości
	//to samo zrób dla normalnej
	blendVertex = (Bones[index] * vVertex4) *  vBlendWeights.x;
    blendNormal = (Bones[index] * vec4(vNormal, 0.0)).xyz *  vBlendWeights.x;
   	 
	//pobierz macierz kości dla drugiego indeksu 
	//i pomnóż ją przez dany wierzchołek oraz wagę wiązania kości, ale dodaj do poprzedniej wartości 
	//blendedVertex  
	//to samo zrób dla normalnej (i też dodaj do poprzedniej wartości blendNormal)
	index = viBlendIndices.y;        
	blendVertex = ((Bones[index] * vVertex4) * vBlendWeights.y) + blendVertex;
    blendNormal = (Bones[index] * vec4(vNormal, 0.0)).xyz * vBlendWeights.y  + blendNormal;

	//pobierz macierz kości dla trzeciego indeksu 
	//i pomnóż ją przez dany wierzchołek oraz wagę wiązania kości i też dodaj do poprzedniej wartości 
	//blendedVertex  
	//to samo zrób dla normalnej (i też dodaj do poprzedniej wartości blendNormal)
	index = viBlendIndices.z;        
	blendVertex = ((Bones[index] * vVertex4) *  vBlendWeights.z)  + blendVertex;
    blendNormal = (Bones[index] * vec4(vNormal, 0.0)).xyz *  vBlendWeights.z  + blendNormal;

	//pobierz macierz kości dla czwartego indeksu 
	//i pomnóż ją przez dany wierzchołek oraz wagę wiązania kości i też dodaj do poprzedniej wartości 
	//blendedVertex  
	//to samo zrób dla normalnej (i też dodaj do poprzedniej wartości blendNormal)
	index = viBlendIndices.w;        
	blendVertex = ((Bones[index] * vVertex4) *  vBlendWeights.w)   + blendVertex;
    blendNormal = (Bones[index] * vec4(vNormal, 0.0)).xyz *  vBlendWeights.w  + blendNormal;

	//na koniec pomnóż blendVertex przez macierz modelu i widoku, aby uzyskać położenie w przestrzeni oka
    vEyeSpacePosition = (MV*blendVertex).xyz; 

	//pomnóż blendNormal przez macierz normalną w celu uzyskania normalnej w przestrzeni oka
    vEyeSpaceNormal   = normalize(N*blendNormal);  
	 
	//wyprowadzenie współrzędnych tekstury
	vUVout=vUV; 

	//wyznacz położenie w przestrzeni przycięcia jako iloczyn położenia w przestrzenie oka i macierzy rzutowania 
    gl_Position = P*vec4(vEyeSpacePosition,1);
}