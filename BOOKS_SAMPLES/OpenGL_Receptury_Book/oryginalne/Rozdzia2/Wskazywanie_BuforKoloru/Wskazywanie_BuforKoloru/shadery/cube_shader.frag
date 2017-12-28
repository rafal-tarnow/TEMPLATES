#version 330 core

layout(location = 0) out vec4 vFragColor;	//wyjœcie shadera fragmentów

//uniform
uniform vec3 vColor; //sta³y kolor

void main()
{
	//sta³y kolor jako wyjœcie shadera
	vFragColor = vec4(vColor.xyz,1);
}