#version 330 core
  
layout(location=0) in vec3 vVertex;		 //po�o�enie wierzcho�ka w przestrzeni obiektu
 
void main()
{    
	//ustawienie po�o�enia w przestrzeni obiektu
	gl_Position =  vec4(vVertex, 1);			
}