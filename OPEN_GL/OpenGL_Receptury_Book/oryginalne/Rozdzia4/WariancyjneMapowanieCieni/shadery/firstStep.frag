#version 330 core

layout(location=0) out vec4 vFragColor;		//Wyj�cie shadera fragment�w
//input from the vertex shader
smooth in vec4 clipSpacePos;	//po�o�enie wierzcho�ka w przestrzeni przyci�cia

void main()
{
	//wyznaczanie wsp�rz�dnych jednorodnych
	vec3 pos = clipSpacePos.xyz/clipSpacePos.w;

	//przesuni�cie likwiduj�ce artefakty
	pos.z += 0.001;	

	//przesuwanie g��bi do przedzia�u (0; 1)
	float depth = (pos.z +1)*0.5; 

	//g��bia jako moment pierwszego rz�du
	float moment1 = depth;
	
	//kwadrat g��bi jako moment drugiego rz�du
	float moment2 = depth * depth; //zaczerpni�te z g��wnej pracy na temat wariacyjnego mapowania cieni

	//z rozdzia�u 8.- GPU Gems 3
	//float dx = dFdx(depth);
	//float dy = dFdy(depth); 
	//float moment2 = depth*depth + 0.25*(dx*dx + dy*dy);

	//zapis moment�w w kana�ach red i green koloru wyj�ciowego
	vFragColor = vec4(moment1,moment2,0,0);
}