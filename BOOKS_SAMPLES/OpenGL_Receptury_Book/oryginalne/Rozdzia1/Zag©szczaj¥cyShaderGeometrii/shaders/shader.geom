#version 330 core
layout (triangles) in;
layout (triangle_strip, max_vertices=256) out; 
 
//uniforms
uniform int sub_divisions;		 //liczba podzia��w
uniform mat4 MVP;				 //po��czona macierz modelu, widoku i rzutowania
void main()
{
	//pobranie po�o�enia wierzcho�ka w przestrzeni obiektu
	vec4 v0 = gl_in[0].gl_Position;
	vec4 v1 = gl_in[1].gl_Position;
	vec4 v2 = gl_in[2].gl_Position; 

	//okre�lenie wielko�ci ka�dego podzia�u
	float dx = abs(v0.x-v2.x)/sub_divisions;
	float dz = abs(v0.z-v1.z)/sub_divisions;

	float x=v0.x;
	float z=v0.z;

	//przebieganie przez wszystkie podzia�y i emitowanie wierzcho�k�w 
	// po wymno�eniu po�o�enia wierzco�ka w przestrzeni obiektu 
	//przez po��czon� macierz modelu, widoku i rzutowania.  
	//Najperw przesuwamy si� wzd�u� osi x a� do napotkania kraw�dzi,  
	//a wtedy przywracamy pocz�tkow� warto�� zmiennej x i zwi�kszamy  
	//warto�� zmiennej z.
	for(int j=0;j<sub_divisions*sub_divisions;j++) { 		 
		gl_Position =  MVP * vec4(x,0,z,1);        EmitVertex();		
		gl_Position =  MVP * vec4(x,0,z+dz,1);     EmitVertex();				  
		gl_Position =  MVP * vec4(x+dx,0,z,1);     EmitVertex(); 
		gl_Position =  MVP * vec4(x+dx,0,z+dz,1);  EmitVertex();
		EndPrimitive();	 
		x+=dx;

		if((j+1) %sub_divisions == 0) {
		   x=v0.x;
		   z+=dz;
		}	
	}	
}