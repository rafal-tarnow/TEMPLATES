#version 330 core
  
layout(location=0) in vec2 vVertex; //wspó³rzêdne wierzcho³ka w przestrzeni obiektu

//wyjœcie shadera wierzcho³ków
smooth out vec2 vUV;	//wspó³rzêdne tekstury
void main()
{    
	//po³o¿enie w przestrzeni przyciêcia
	gl_Position = vec4(vVertex*2.0-1,0,1);	 

	//wspó³rzêdne w przestrzeni obiektu jako wspó³rzêdne tekstury
	vUV = vVertex;
}