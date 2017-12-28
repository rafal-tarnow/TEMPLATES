#version 330 core
  
layout(location=0) in vec2 vVertex; //po³o¿enie wierzcho³ka w przestrzeni obiektu

//wyjœcie shadera wierzcho³ków
smooth out vec2 vUV;			//wspó³rzêdne tekstury 2D dla shadera fragmentów

void main()
{   	
	//wyznaczanie po³o¿enia w przestrzeni przyciêcia na podstawie po³o¿enia w przestrzeni w obiektu
	gl_Position = vec4(vVertex.xy*2-1.0,0,1);	 

	//wyznaczanie wspó³rzêdnych tekstury na podstawie po³o¿enia wierzcho³ka w przestrzenie obiektu
	vUV = vVertex;
}