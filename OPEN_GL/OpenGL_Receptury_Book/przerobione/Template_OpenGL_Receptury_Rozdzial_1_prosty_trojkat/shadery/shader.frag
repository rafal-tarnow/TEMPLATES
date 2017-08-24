#version 330 core

layout(location=0) out vec4 vFragColor;	//wyj�cie shadera fragment�w

//wej�cie z shadera wierzcho�k�w
smooth in vec4 vSmoothColor;		//interpolowany kolor dla shadera fragment�w

void main()
{
	//interpolowany kolor jako wyj�cie shadera
	vFragColor = vSmoothColor;
}