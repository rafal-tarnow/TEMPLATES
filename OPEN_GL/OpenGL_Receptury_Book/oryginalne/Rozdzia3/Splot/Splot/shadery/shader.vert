#version 330 core
  
layout(location=0) in vec2 vVertex; //po³o¿enie wierzcho³ka w przestrzeni obiektu

//Wyjœcie shadera wierzcho³ków
smooth out vec2 vUV;	//wspó³rzêdne teksturowe potrzebne do próbkowania tekstury w shaderze fragmentów

void main()
{    
	//wyprowadzenie po³o¿enia w przestrzeni przyciêcia
	gl_Position = vec4(vVertex*2.0-1,0,1);	 

	//ustawienie wejœciowego po³o¿enia w przestrzeni obiektu jako wspó³rzêdnych teksturowych
	vUV = vVertex;
}