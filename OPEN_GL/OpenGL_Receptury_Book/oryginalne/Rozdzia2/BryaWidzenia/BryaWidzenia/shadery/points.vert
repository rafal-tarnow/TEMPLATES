#version 330 core
  
layout(location = 0) in vec3 vVertex;   //po�o�enie wierzcho�ka w przestrzeni obiektu

//uniform 
uniform float t;	//bie��cy czas

//sta�a shaderowa
const float PI = 3.141562;

void main()
{   
	//przemieszczanie wierzcho�ka zgodnie z przebiegiem fali sinusoidalnej
	gl_Position = vec4(vVertex,1) + vec4(0,sin(vVertex.x *2*PI+t),0,0);
}