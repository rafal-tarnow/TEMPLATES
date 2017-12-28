#version 330 core
  
layout(location=0) in vec3 vVertex;		//Po�o�enie wierzcho�ka 
layout(location=1) in vec3 vNormal;  	//wektor normalny wierzcho�ka
 
//uniformy 
uniform mat4 MVP;		//Po��czona macierz modelu, widoku i rzutowania
uniform mat4 MV;		//macierz modelu i widoku
uniform mat4 M;		//macierz modelu
uniform mat3 N;			//macierz normalna
uniform mat4 S;		//shadow matrix

//dane wyj�ciowe dla shadera fragment�w
smooth out vec3 vEyeSpaceNormal;    //wektor normalny wierzcho�ka w przestrzeni oka
smooth out vec3 vEyeSpacePosition;	//Po�o�enie wierzcho�ka w przestrzeni oka
smooth out vec4 vShadowCoords;			//wsp�rz�dne cienia
void main()
{ 	
	//mno�enie po�o�enia wierzcho�ka w przestrzeni obiektu przez macierz modelu i widoku 
	//w celu wyznaczenia po�o�enia wierzcho�ka w przestrzeni oka
	vEyeSpacePosition = (MV*vec4(vVertex,1)).xyz; 

	//mno�enie wektora normalnego z przestrzeni obiektu przez macierz normaln� 
	//w celu wyznaczenia wektora normalnego w przestrzeni oka
	vEyeSpaceNormal   = N*vNormal;

	//mno�enie po�o�enia wierzcho�ka w przestrzeni �wiata przez macierz cienia 
	//w celu wyznaczenia wsp�rz�dnych cienia
	vShadowCoords     = S*(M*vec4(vVertex,1));

	//mno�enie po�o�enia wierzcho�ka w przestrzeni obiektu przez po��czon� macierz modelu, widoku i rzutowania
	//w celu wyznaczenia po�o�enia wierzcho�ka w przestrzeni przyci�cia
    gl_Position       = MVP*vec4(vVertex,1); 
}
 