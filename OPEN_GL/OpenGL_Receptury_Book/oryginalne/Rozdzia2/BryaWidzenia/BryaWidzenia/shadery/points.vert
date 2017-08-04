#version 330 core
  
layout(location = 0) in vec3 vVertex;   //po³o¿enie wierzcho³ka w przestrzeni obiektu

//uniform 
uniform float t;	//bie¿¹cy czas

//sta³a shaderowa
const float PI = 3.141562;

void main()
{   
	//przemieszczanie wierzcho³ka zgodnie z przebiegiem fali sinusoidalnej
	gl_Position = vec4(vVertex,1) + vec4(0,sin(vVertex.x *2*PI+t),0,0);
}