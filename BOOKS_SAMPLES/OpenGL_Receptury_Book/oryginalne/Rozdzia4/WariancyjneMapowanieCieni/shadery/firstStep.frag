#version 330 core

layout(location=0) out vec4 vFragColor;		//Wyjœcie shadera fragmentów
//input from the vertex shader
smooth in vec4 clipSpacePos;	//po³o¿enie wierzcho³ka w przestrzeni przyciêcia

void main()
{
	//wyznaczanie wspó³rzêdnych jednorodnych
	vec3 pos = clipSpacePos.xyz/clipSpacePos.w;

	//przesuniêcie likwiduj¹ce artefakty
	pos.z += 0.001;	

	//przesuwanie g³êbi do przedzia³u (0; 1)
	float depth = (pos.z +1)*0.5; 

	//g³êbia jako moment pierwszego rzêdu
	float moment1 = depth;
	
	//kwadrat g³êbi jako moment drugiego rzêdu
	float moment2 = depth * depth; //zaczerpniête z g³ównej pracy na temat wariacyjnego mapowania cieni

	//z rozdzia³u 8.- GPU Gems 3
	//float dx = dFdx(depth);
	//float dy = dFdy(depth); 
	//float moment2 = depth*depth + 0.25*(dx*dx + dy*dy);

	//zapis momentów w kana³ach red i green koloru wyjœciowego
	vFragColor = vec4(moment1,moment2,0,0);
}