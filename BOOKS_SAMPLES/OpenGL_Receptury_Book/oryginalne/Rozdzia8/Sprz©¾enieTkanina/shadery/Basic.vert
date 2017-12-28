#version 330 core
precision highp float;

#extension EXT_gpu_shader4 : require

layout(location=0) in vec4 position_mass;	//xyz -> położenie, w -> masa

//uniformy
uniform mat4 MV;				//macierz modelu i widoku
uniform mat4 MVP;				//połączona macierz modelu, widoku i rzutowania
uniform float pointSize;		//rozmiar punktu w przestrzeni ekranu
uniform int selected_index;		//indeks wskazanego wierzchołka
uniform in vec4 vColor;			//uniform koloru
smooth out vec4 oColor;			//wyjście do shadera fragmentów
							 
void main() 
{  	
	vec4 vert = vec4(position_mass.xyz,1);
	//wyznacz położenie w przestrzeni oka	
	vec3 pos_eye = (MV * vert).xyz;
	//oblicz rozmiar punktu 
    gl_PointSize = max(1.0, pointSize / (1.0 - pos_eye.z));
	//wyznacz położenie w przestrzeni przycięcia
	gl_Position = MVP*vec4(position_mass.xyz, 1.0);		
	//jeśli wierzchołek jest zaznaczony, przypisz mu kolor przeciwny 
	if(selected_index == gl_VertexID)
	   oColor = 1-vColor;
	else
	   oColor = vColor;
}