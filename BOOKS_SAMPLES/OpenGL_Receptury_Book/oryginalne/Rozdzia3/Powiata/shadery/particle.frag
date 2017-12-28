#version 330 core

layout(location=0) out vec4 vFragColor;	//wyjœcie shadera fragmentów

//kolor wejœciowy 
smooth in vec4 color;

void main() { 
	//wyznaczanie przesuniêtych wspó³rzêdnych cz¹stki
	vec2 pos = gl_PointCoord-0.5;
	//odrzucanie fragmentów spoza zadanego promienia
	if(dot(pos,pos)>0.25) 
		discard;
	else
		//dla pozosta³ych fragmentów zwracanie pobranego koloru
		vFragColor = color;  
}