#version 330 core
  
layout(location=0) in vec3 vVertex;		//po³o¿enie wierzcho³ka w przestrzeni obiektu
layout(location=1) in vec3 vNormal;		//wektor normalny wierzcho³ka
 
//uniformy  
uniform mat4 MVP;				//po³¹czona macierz modelu, widoku i rzutowania
uniform mat4 MV;				//macierz modelu i widoku
uniform mat3 N;					//macierz normalna
uniform vec3 light_position;	//po³o¿enie œwiat³a w przestrzeni obiektu
uniform vec3 diffuse_color;		//kolor œwiat³a rozpraszanego przez obiekt
uniform vec3 specular_color;	//kolor œwiat³a odbijanego przez obiekt
uniform float shininess;		//jasnoœæ odblasku
//dane wyjœciowe dla shadera fragmentów
smooth out vec4 color;    //wyjœciowy kolor œwaiat³a rozpraszanego dla shadera fragmentów

//sta³a shaderowa
const vec3 vEyeSpaceCameraPosition = vec3(0,0,0); //obserwator znajduje siê w punkcie vec3(0,0,0) przestrzeni oka

void main()
{ 	
	//mno¿enie po³o¿enia œwiat³a w przestrzeni obiektu przez macierz modelu i widoku 
	//w celu wyznaczenia po³o¿enia swiat³a w przestrzeni oka
	vec4 vEyeSpaceLightPosition = MV*vec4(light_position,1);

	//mno¿enie po³o¿enia wierzcho³ka w przestrzeni obiektu przez macierz modelu i widoku 
	//w celu wyznaczenia po³o¿enia wierzcho³ka w przestrzeni oka
	vec4 vEyeSpacePosition = MV*vec4(vVertex,1); 

	//mno¿enie wektora normalnego z przestrzeni obiektu przez macierz normaln¹ 
	//w celu wyznaczenia wektora normalnego w przestrzeni oka
	vec3 vEyeSpaceNormal   = normalize(N*vNormal);

	//wyznaczanie wektora œwiat³a
	vec3 L = normalize(vEyeSpaceLightPosition.xyz-vEyeSpacePosition.xyz);
	//wyznaczanie wektora widoku
	vec3 V = normalize(vEyeSpaceCameraPosition.xyz-vEyeSpacePosition.xyz);
	//wyznaczanie wektora poœredniego miêdzy wektorami œwiat³a i widoku
	vec3 H = normalize(L+V);

	//obliczanie sk³adowych rozproszenia i odblasku
	float diffuse = max(0, dot(vEyeSpaceNormal, L));
	float specular = max(0, pow(dot(vEyeSpaceNormal, H), shininess));

	//obliczanie koloru wyjœciowego przez sumowanie sk³adników rozproszenia i odblasku
	color = diffuse*vec4(diffuse_color,1) + specular*vec4(specular_color, 1);

	//mno¿enie po³o¿enia wierzcho³ka w przestrzeni obiektu przez po³¹czon¹ macierz modelu, widoku i rzutowania
	//w celu wyznaczenia po³o¿enia wierzcho³ka w przestrzeni przyciêcia
    gl_Position = MVP*vec4(vVertex,1); 
}
 