#version 330 core

layout(location=0) out vec4 vFragColor;			//wyjœcie shadera fragmentów

//uniformy
uniform samplerCube cubeMap;	//sampler tekstury szeœciennej
uniform vec3 eyePosition;		//po³o¿enie oka w przestrzeni obiektu
//input from the vertex shader
smooth in vec3 position;		//po³o¿enie w przestrzeni obiektu
smooth in vec3 normal;			//wektor normalny w przestrzeni obiektu


void main() { 
	//normalizacja wektora normalnego
	vec3 N = normalize(normal);

	//wyznaczanie znormalizowanego wektora patrzenia na podstawie
	//po³o¿eñ wierzcho³ka i kamery w przestrzeni obiektu
	vec3 V = normalize(position-eyePosition);

	//odbijanie wektora patrzenia wzglêdem wektora normalnego i na podstawie tego odbicia 
	//wybieranie próbki z tekstury szeœciennej  
	vFragColor = texture(cubeMap, reflect(V,N));	 
}