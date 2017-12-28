#version 330 core
  
layout(location=0) in vec3 vVertex;		//po�o�enie wierzcho�ka w przestrzeni obiektu
layout(location=1) in vec3 vNormal;		//wektor normalny wierzcho�ka
 
//uniformy  
uniform mat4 MVP;				//po��czona macierz modelu, widoku i rzutowania
uniform mat4 MV;				//macierz modelu i widoku
uniform mat3 N;					//macierz normalna
uniform vec3 light_position;	//po�o�enie �wiat�a w przestrzeni obiektu
uniform vec3 diffuse_color;		//kolor �wiat�a rozpraszanego przez obiekt
uniform vec3 specular_color;	//kolor �wiat�a odbijanego przez obiekt
uniform float shininess;		//jasno�� odblasku
//dane wyj�ciowe dla shadera fragment�w
smooth out vec4 color;    //wyj�ciowy kolor �waiat�a rozpraszanego dla shadera fragment�w

//sta�a shaderowa
const vec3 vEyeSpaceCameraPosition = vec3(0,0,0); //obserwator znajduje si� w punkcie vec3(0,0,0) przestrzeni oka

void main()
{ 	
	//mno�enie po�o�enia �wiat�a w przestrzeni obiektu przez macierz modelu i widoku 
	//w celu wyznaczenia po�o�enia swiat�a w przestrzeni oka
	vec4 vEyeSpaceLightPosition = MV*vec4(light_position,1);

	//mno�enie po�o�enia wierzcho�ka w przestrzeni obiektu przez macierz modelu i widoku 
	//w celu wyznaczenia po�o�enia wierzcho�ka w przestrzeni oka
	vec4 vEyeSpacePosition = MV*vec4(vVertex,1); 

	//mno�enie wektora normalnego z przestrzeni obiektu przez macierz normaln� 
	//w celu wyznaczenia wektora normalnego w przestrzeni oka
	vec3 vEyeSpaceNormal   = normalize(N*vNormal);

	//wyznaczanie wektora �wiat�a
	vec3 L = normalize(vEyeSpaceLightPosition.xyz-vEyeSpacePosition.xyz);
	//wyznaczanie wektora widoku
	vec3 V = normalize(vEyeSpaceCameraPosition.xyz-vEyeSpacePosition.xyz);
	//wyznaczanie wektora po�redniego mi�dzy wektorami �wiat�a i widoku
	vec3 H = normalize(L+V);

	//obliczanie sk�adowych rozproszenia i odblasku
	float diffuse = max(0, dot(vEyeSpaceNormal, L));
	float specular = max(0, pow(dot(vEyeSpaceNormal, H), shininess));

	//obliczanie koloru wyj�ciowego przez sumowanie sk�adnik�w rozproszenia i odblasku
	color = diffuse*vec4(diffuse_color,1) + specular*vec4(specular_color, 1);

	//mno�enie po�o�enia wierzcho�ka w przestrzeni obiektu przez po��czon� macierz modelu, widoku i rzutowania
	//w celu wyznaczenia po�o�enia wierzcho�ka w przestrzeni przyci�cia
    gl_Position = MVP*vec4(vVertex,1); 
}
 