#version 330 core

layout(location=0) out vec4 vFragColor;	//wyjœcie shadera fragmentów
 
//uniformy
uniform mat4 MV;				//macierz modelu i widoku
uniform vec3 light_position;	//po³o¿enie œwiat³a w przestrzeni obiektu
uniform vec3 diffuse_color;		//kolor œwiat³a rozpraszanego przez obiekt
uniform vec3 specular_color;	//kolor œwiat³a odbijanego przez powierzchniê obiektu
uniform float shininess;		//jasnoœæ odblasku 

//dane wejœciowe z shadera wierzcho³ków
smooth in vec3 vEyeSpaceNormal;		//interpolowany wektor normalny w przestrzeni oka
smooth in vec3 vEyeSpacePosition;	//interpolowany wektor po³o¿enia w przestrzeni oka

//sta³a shaderowa
const vec3 vEyeSpaceCameraPosition = vec3(0,0,0);

void main() { 
	//mno¿enie po³o¿enia œwiat³a w przestrzeni obiektu przez macierz modelu i widoku 
	//w celu wyznaczenia po³o¿enia swiat³a w przestrzeni oka
	vec3 vEyeSpaceLightPosition = (MV * vec4(light_position,1)).xyz;

	//normalizacja wektora normalnego w przestrzeni oka
	vec3 N = normalize(vEyeSpaceNormal);	
	//wyznaczanie wektora œwiat³a i jego normalizacja
	vec3 L = normalize(vEyeSpaceLightPosition-vEyeSpacePosition);
	//wyznaczanie wektora widoku i jego normalizacja
	vec3 V = normalize(vEyeSpaceCameraPosition.xyz-vEyeSpacePosition.xyz);
	//wyznaczanie wektora poœredniego miêdzy wektorami œwiat³a i widoku
	vec3 H = normalize(L+V);
	//obliczanie sk³adowej rozproszenia
	float diffuse = max(0, dot(N, L));
	//obliczanie sk³adowej odblasku
	float specular = max(0, pow(dot(N, H), shininess));

	//wyprowadzenie sumy sk³adowych koloru
	//jako ostatecznego koloru fragmentu 
	vFragColor = diffuse*vec4(diffuse_color,1) + specular*vec4(specular_color, 1);
}