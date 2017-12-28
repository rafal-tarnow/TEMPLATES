#version 330 core

layout(location=0) out vec4 vFragColor;	//wyj�cie shadera fragment�w
 
//uniformy
uniform mat4 MV;				//macierz modelu i widoku
uniform vec3 light_position;	//po�o�enie �wiat�a w przestrzeni obiektu
uniform vec3 diffuse_color;		//kolor �wiat�a rozpraszanego przez obiekt
uniform vec3 specular_color;	//kolor �wiat�a odbijanego przez powierzchni� obiektu
uniform float shininess;		//jasno�� odblasku 

//dane wej�ciowe z shadera wierzcho�k�w
smooth in vec3 vEyeSpaceNormal;		//interpolowany wektor normalny w przestrzeni oka
smooth in vec3 vEyeSpacePosition;	//interpolowany wektor po�o�enia w przestrzeni oka

//sta�a shaderowa
const vec3 vEyeSpaceCameraPosition = vec3(0,0,0);

void main() { 
	//mno�enie po�o�enia �wiat�a w przestrzeni obiektu przez macierz modelu i widoku 
	//w celu wyznaczenia po�o�enia swiat�a w przestrzeni oka
	vec3 vEyeSpaceLightPosition = (MV * vec4(light_position,1)).xyz;

	//normalizacja wektora normalnego w przestrzeni oka
	vec3 N = normalize(vEyeSpaceNormal);	
	//wyznaczanie wektora �wiat�a i jego normalizacja
	vec3 L = normalize(vEyeSpaceLightPosition-vEyeSpacePosition);
	//wyznaczanie wektora widoku i jego normalizacja
	vec3 V = normalize(vEyeSpaceCameraPosition.xyz-vEyeSpacePosition.xyz);
	//wyznaczanie wektora po�redniego mi�dzy wektorami �wiat�a i widoku
	vec3 H = normalize(L+V);
	//obliczanie sk�adowej rozproszenia
	float diffuse = max(0, dot(N, L));
	//obliczanie sk�adowej odblasku
	float specular = max(0, pow(dot(N, H), shininess));

	//wyprowadzenie sumy sk�adowych koloru
	//jako ostatecznego koloru fragmentu 
	vFragColor = diffuse*vec4(diffuse_color,1) + specular*vec4(specular_color, 1);
}