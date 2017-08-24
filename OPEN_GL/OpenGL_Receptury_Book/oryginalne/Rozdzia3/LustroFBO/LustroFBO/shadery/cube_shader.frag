#version 330 core

layout(location=0) out vec4 vFragColor;	//wyjœcie shadera fragmentów

//dane wejœciowe z shadera wierzcho³ków
smooth in vec3 vColor;			//interpolowany kolor z shadera fragmentów

void main()
{
	//interpolowany kolor z shadera wierzcho³ków jako kolor fragmentu
	vFragColor = vec4(vColor.xyz,1);
}