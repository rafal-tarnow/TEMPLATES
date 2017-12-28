#version 330 core

layout(location = 0) out vec4 vFragColor;	//wyjœcie shadera fragmentów

//uniform koloru
uniform vec3 vColor;

void main()
{
	//przypisanie zadanego koloru do bie¿¹cego fragmentu
	vFragColor = vec4(vColor.xyz,1);
}