#version 330 core

layout(location=0) out vec4 vFragColor;			//wyj�cie shadera fragment�w

//uniformy
uniform samplerCube cubeMap;	//sampler tekstury sze�ciennej
uniform vec3 eyePosition;		//po�o�enie oka w przestrzeni obiektu
//input from the vertex shader
smooth in vec3 position;		//po�o�enie w przestrzeni obiektu
smooth in vec3 normal;			//wektor normalny w przestrzeni obiektu


void main() { 
	//normalizacja wektora normalnego
	vec3 N = normalize(normal);

	//wyznaczanie znormalizowanego wektora patrzenia na podstawie
	//po�o�e� wierzcho�ka i kamery w przestrzeni obiektu
	vec3 V = normalize(position-eyePosition);

	//odbijanie wektora patrzenia wzgl�dem wektora normalnego i na podstawie tego odbicia 
	//wybieranie pr�bki z tekstury sze�ciennej  
	vFragColor = texture(cubeMap, reflect(V,N));	 
}