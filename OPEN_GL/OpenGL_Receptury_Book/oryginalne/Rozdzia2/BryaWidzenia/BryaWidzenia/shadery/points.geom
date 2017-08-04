#version 330 core
layout (points) in;
layout (points, max_vertices=3) out; 
 
//uniformy			
uniform mat4 MVP;				//po³¹czona macierz modelu, widoku i rzutowania
uniform vec4 FrustumPlanes[6];	//œciany bry³y odcinania

//sprawdzanie po³o¿enia badanego punktu (p) wzglêdem œciany bry³y widzenia. W tym celu obliczana jest odleg³oœæ punktu od œiany
//jeœli ta odleg³oœæ jest mniejsza od 0, to znaczy, ¿e punkt le¿y po zewnêtrznej stronie œciany
bool PointInFrustum(in vec3 p) {
	for(int i=0; i < 6; i++) 
	{
		vec4 plane=FrustumPlanes[i];
		if ((dot(plane.xyz, p)+plane.w) < 0)
			return false;
	}
	return true;
}

void main()
{
	//przygotowanie wierzcho³ków do renderowania
	for(int i=0;i<gl_in.length(); i++) { 
		vec4 vInPos = gl_in[i].gl_Position;
		//convert the points from 0 to 1 range to -5 to 5 range
		vec2 tmp = (vInPos.xz*2-1.0)*5;
		//take the Y value from the vertex shafer output
		vec3 V = vec3(tmp.x, vInPos.y, tmp.y);

		//wyznaczanie po³o¿eñ w przestrzeni przyciêcia 
		gl_Position = MVP*vec4(V,1);
		
		//emitowanie wierzcho³ków zawartych w bryle widzenia	  
		if(PointInFrustum(V)) { 
			EmitVertex();		
		} 
	}
	EndPrimitive();
}