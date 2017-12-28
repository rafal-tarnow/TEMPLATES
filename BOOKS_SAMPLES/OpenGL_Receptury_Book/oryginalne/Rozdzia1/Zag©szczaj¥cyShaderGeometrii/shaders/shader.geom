#version 330 core
layout (triangles) in;
layout (triangle_strip, max_vertices=256) out; 
 
//uniforms
uniform int sub_divisions;		 //liczba podzia³ów
uniform mat4 MVP;				 //po³¹czona macierz modelu, widoku i rzutowania
void main()
{
	//pobranie po³o¿enia wierzcho³ka w przestrzeni obiektu
	vec4 v0 = gl_in[0].gl_Position;
	vec4 v1 = gl_in[1].gl_Position;
	vec4 v2 = gl_in[2].gl_Position; 

	//okreœlenie wielkoœci ka¿dego podzia³u
	float dx = abs(v0.x-v2.x)/sub_divisions;
	float dz = abs(v0.z-v1.z)/sub_divisions;

	float x=v0.x;
	float z=v0.z;

	//przebieganie przez wszystkie podzia³y i emitowanie wierzcho³ków 
	// po wymno¿eniu po³o¿enia wierzco³ka w przestrzeni obiektu 
	//przez po³¹czon¹ macierz modelu, widoku i rzutowania.  
	//Najperw przesuwamy siê wzd³u¿ osi x a¿ do napotkania krawêdzi,  
	//a wtedy przywracamy pocz¹tkow¹ wartoœæ zmiennej x i zwiêkszamy  
	//wartoœæ zmiennej z.
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