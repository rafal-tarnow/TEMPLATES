#version 330 core
  
layout(location=0) in vec3 vVertex;		//Po³o¿enie wierzcho³ka 
layout(location=1) in vec3 vNormal;  	//wektor normalny wierzcho³ka
 
//uniformy 
uniform mat4 MVP;		//Po³¹czona macierz modelu, widoku i rzutowania
uniform mat4 MV;		//macierz modelu i widoku
uniform mat3 N;			//macierz normalna

//dane wyjœciowe dla shadera fragmentów
smooth out vec3 vEyeSpaceNormal;    //wektor normalny wierzcho³ka w przestrzeni oka
smooth out vec3 vEyeSpacePosition;	//Po³o¿enie wierzcho³ka w przestrzeni oka

void main()
{ 	
	//mno¿enie po³o¿enia wierzcho³ka w przestrzeni obiektu przez macierz modelu i widoku 
	//w celu wyznaczenia po³o¿enia wierzcho³ka w przestrzeni oka
	vEyeSpacePosition = (MV*vec4(vVertex,1)).xyz; 
	
	//mno¿enie wektora normalnego z przestrzeni obiektu przez macierz normaln¹ 
	//w celu wyznaczenia wektora normalnego w przestrzeni oka
	vEyeSpaceNormal   = N*vNormal;

	//mno¿enie po³o¿enia wierzcho³ka w przestrzeni obiektu przez po³¹czon¹ macierz modelu, widoku i rzutowania
	//w celu wyznaczenia po³o¿enia wierzcho³ka w przestrzeni przyciêcia
    gl_Position = MVP*vec4(vVertex,1);  
}
 