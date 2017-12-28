#version 330 core
  
layout(location=0) out vec4 vFragColor;  //wyjście shadera fragmentów
 
smooth in vec2 vUV;	//interpolowane współrzędne tekstury pobrane z shadera wierzchołków

//uniformy
uniform sampler2D textureMap; //mapa tekstury do wyświetlenia

void main()
{ 	 
	//wyprowadzenie próbki tekstury pobranej z punktu o podanych współrzędnych
	vFragColor = texture(textureMap, vUV); 
}