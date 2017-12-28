#version 330 core

layout(location=0) out vec4 vFragColor;	//wyj�cie shadera fragment�w

//kolor wej�ciowy 
smooth in vec4 color;

void main() { 
	//wyznaczanie przesuni�tych wsp�rz�dnych cz�stki
	vec2 pos = gl_PointCoord-0.5;
	//odrzucanie fragment�w spoza zadanego promienia
	if(dot(pos,pos)>0.25) 
		discard;
	else
		//dla pozosta�ych fragment�w zwracanie pobranego koloru
		vFragColor = color;  
}