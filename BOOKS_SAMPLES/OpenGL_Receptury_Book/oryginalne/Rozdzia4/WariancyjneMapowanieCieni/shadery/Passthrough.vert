#version 330 core
  
layout(location=0) in vec2 vVertex;		//po³o¿enie wierzcho³ka w przestrzeni obiektu
//wyjœcie shadera
smooth out vec2 vUV;					//wspó³rzêdne tekstury

void main()
{ 	 
	//wyznaczanie po³o¿enia w przestrzeni przyciêcia na podstawie po³o¿enia w przestrzeni obiektu
	gl_Position = vec4(vVertex*2-1.0,0,1);
	//przypisanie po³o¿enia wierzcho³ka jako wyjœciowych wspó³rzêdnch tekstury
	vUV = vVertex;
}