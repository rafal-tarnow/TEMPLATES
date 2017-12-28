#version 330 core
layout (triangles) in;
layout (triangle_strip, max_vertices=256) out; 
 
//uniforms 
uniform int sub_divisions;	//liczba podziałów
uniform mat4 PV;			//połączona macierz widoku i rzutowania

void main()
{
	//pobranie położenia wierzchołka w przestrzeni obiektu
	vec4 v0 = gl_in[0].gl_Position;
	vec4 v1 = gl_in[1].gl_Position;
	vec4 v2 = gl_in[2].gl_Position; 

	//określenie wielkości każdego podziału 
	float dx = abs(v0.x-v2.x)/sub_divisions;
	float dz = abs(v0.z-v1.z)/sub_divisions;

	float x=v0.x;
	float z=v0.z;

	//przebieganie przez wszystkie podziały i emitowanie wierzchołków
	//po wymnożeniu położenia wierzcołka w przestrzeni obiektu
	//przez połączoną macierz modelu, widoku i rzutowania.
	//Najperw przesuwamy się wzdłuż osi x aż do napotkania krawędzi, 
	//a wtedy przywracamy początkową wartość zmiennej x i zwiększamy wartość zmiennej z.
	for(int j=0;j<sub_divisions*sub_divisions;j++) {		
		gl_Position =  PV * vec4(x,0,z,1);        EmitVertex();		
		gl_Position =  PV * vec4(x,0,z+dz,1);     EmitVertex();				  
		gl_Position =  PV * vec4(x+dx,0,z,1);     EmitVertex(); 
		gl_Position =  PV * vec4(x+dx,0,z+dz,1);  EmitVertex();
		EndPrimitive();	 
		x+=dx;

		if((j+1) %sub_divisions == 0) {
		   x=v0.x;
		   z+=dz;
		}	
	}	
}
