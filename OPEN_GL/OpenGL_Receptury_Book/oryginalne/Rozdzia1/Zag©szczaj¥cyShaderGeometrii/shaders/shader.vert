#version 330 core
  
layout(location=0) in vec3 vVertex;		 //po³o¿enie wierzcho³ka w przestrzeni obiektu
 
void main()
{    
	//ustawienie po³o¿enia w przestrzeni obiektu
	gl_Position =  vec4(vVertex, 1);			
}