#version 330 core
  
layout(location=0) in vec3 vVertex; //po³o¿enie wierzcho³ka w przestrzeni obiektu

//uniform
uniform mat4 MVP;	//po³¹czona macierz modelu, widoku i rzutowania

void main()
{ 
	//no¿enie po³o¿enia wierzcho³ka przez po³¹czon¹ macierz modelu, widoku i rzutowania 
        //w celu uzyskania po³o¿enia w przestrzeni przyciêcia

	gl_Position = MVP*vec4(vVertex.xyz,1);
}