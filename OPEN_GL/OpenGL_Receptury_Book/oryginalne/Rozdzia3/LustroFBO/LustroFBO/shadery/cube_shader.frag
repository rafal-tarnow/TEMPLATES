#version 330 core

layout(location=0) out vec4 vFragColor;	//wyj�cie shadera fragment�w

//dane wej�ciowe z shadera wierzcho�k�w
smooth in vec3 vColor;			//interpolowany kolor z shadera fragment�w

void main()
{
	//interpolowany kolor z shadera wierzcho�k�w jako kolor fragmentu
	vFragColor = vec4(vColor.xyz,1);
}