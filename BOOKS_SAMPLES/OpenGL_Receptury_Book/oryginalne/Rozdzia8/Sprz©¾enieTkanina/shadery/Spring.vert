#version 330 core
#extension EXT_gpu_shader4 : require

layout( location = 0 )  in vec4 position_mass;	//xyz -> położenie, w -> masa
layout( location = 1 )  in vec4 prev_position;	//xyz -> położenie poprzednie, w -> masa
 
uniform mat4 MVP;								//połączona macierz modelu, widoku i rzutowania
uniform samplerBuffer tex_position_mass;		//bufor teksturowy dla położenia bieżącego
uniform samplerBuffer tex_prev_position_mass;	//bufor teksturowy dla położenia poprzedniego
uniform vec2  inv_cloth_size;					//rozmiar pojedynczej łaty w przestrzeni świata
uniform vec2  step;								//przyrost rozmiaru tekstury
uniform int texsize_x;							//rozmiar tekstury położeń
uniform int texsize_y; 

//stałe czasu, sprężystości i tłumienia
uniform float dt, ksStr, ksShr, ksBnd, 
				 kdStr, kdShr, kdBnd, DEFAULT_DAMPING;
  
//siła grawitacji
uniform vec3 gravity;
 
//wyjścia shadera 
out vec4 out_position_mass;
out vec4 out_prev_position;
 
//zwraca położenie danego wierzchołka oraz wartości jego siły sprężystości i tłumienia
ivec2 getNextNeighbor(int n, out float ks, out float kd) { 
   //strukturalne siły sprężystości (od najbliższych sąsiadów)
   //        o
   //        |
   //     o--m--o
   //        |
   //        o
   if(n<4) {
       ks = ksStr;
       kd = kdStr;
   }
	if (n == 0) return ivec2( 1, 0);
	if (n == 1) return ivec2( 0, -1);
	if (n == 2) return ivec2(-1, 0);
	if (n == 3) return ivec2( 0, 1);
	
	//siły ścinające (od sąsiadów diagonalnych)
	//     o  o  o
	//      \   /
	//     o  m  o
	//      /   \
	//     o  o  o
	if(n<8) {
       ks = ksShr;
       kd = kdShr;
   }
	if (n == 4) return ivec2( 1,  -1);
	if (n == 5) return ivec2( -1, -1);	
	if (n == 6) return ivec2(-1,  1);
	if (n == 7) return ivec2( 1,  1);
	
	//siły gnące (od sąsiadów oddalonych o 1 węzeł)
	//
	//o   o   o   o   o
	//        | 
	//o   o   |   o   o
	//        |   
	//o-------m-------o
	//        |  
	//o   o   |   o   o
	//        |
	//o   o   o   o   o 
	if(n<12) {
       ks = ksBnd;
       kd = kdBnd;
   }
	if (n == 8) return ivec2( 2, 0);
	if (n == 9) return ivec2( 0, -2);
	if (n ==10) return ivec2(-2, 0);
	if (n ==11) return ivec2( 0, 2);
}

void main() 
{  
	//pobierz atrybuty bieżącej cząstki
	float m = position_mass.w;
	vec3 pos = position_mass.xyz;
    vec3 pos_old = prev_position.xyz;	
	vec3 vel = (pos - pos_old) / dt;
	float ks=0, kd=0;

	//wyznacz indeks bieżącej cząstki
	int index = gl_VertexID;
	int ix = index % texsize_x;
	int iy = index / texsize_x;

	// jeśli jest to wierzchołek narożny, przypisz mu masę równą 0, aby go unieruchomić
	if(index ==0 || index == (texsize_x-1))	 
		m = 0;

    //oblicz siły zewnętrzne, czyli grawitację i opory ruchu
	// i dodaj je do siły wypadkowej F
	vec3 F = gravity*m + (DEFAULT_DAMPING*vel);
	
	//dla wszystkich sąsiadów bieżącego wierzchołka
	for(int k=0;k<12;k++) {
		//pobierz współrzędne sąsiada
		ivec2 coord = getNextNeighbor(k, ks, kd);
		int j = coord.x;
		int i = coord.y;		

		//sprawdź indeksy pod kątem wzajemnych powiązań obu wierzchołków
		if (((iy + i) < 0) || ((iy + i) > (texsize_y-1)))
			continue;

		if (((ix + j) < 0) || ((ix + j) > (texsize_x-1)))
			continue;

		//wyznacz indeks liniowy
		int index_neigh = (iy + i) * texsize_x + ix + j;

		//pobierz położenia bieżące i poprzednie z bufora teksturowego
		vec3 p2 = texelFetchBuffer(tex_position_mass, index_neigh).xyz;
		vec3 p2_last = texelFetchBuffer(tex_prev_position_mass, index_neigh).xyz;
		
		//pobierz współrzędne bezwględne
		vec2 coord_neigh = vec2(ix + j, iy + i)*step;

		//określ odległość spoczynkową
		float rest_length = length(coord*inv_cloth_size);

		//oblicz prędkość oraz zmiany położenia i prędkości
		vec3 v2 = (p2- p2_last)/dt;
		vec3 deltaP = pos - p2;	
		vec3 deltaV = vel - v2;	 
		float dist = length(deltaP);
				
		//na podstawie odległości spoczynkowej, oraz zmian położenia i prędkości wyznacz wewnętrzną siłę sprężystości
		float   leftTerm = -ks * (dist-rest_length);
		float  rightTerm = kd * (dot(deltaV, deltaP)/dist);		
		vec3 springForce = (leftTerm + rightTerm)* normalize(deltaP);

		//dodaj wewnętrzną siłę do wypadkowej F
		F +=  springForce;	
	}

	//oblicz przyspieszenie
    vec3 acc = vec3(0);
	if(m!=0)
	   acc = F/m; 

	//wyznacz nowe położenie, stosując metodę Verleta
	vec3 tmp = pos; 
	pos = pos * 2.0 - pos_old + acc* dt * dt;
	pos_old = tmp;

	//sprawdź kolizję z podłożem
	pos.y=max(0, pos.y); 
 
	//ustaw wyjścia shadera i położenie wierzchołka w przestrzeni przycięcia
	out_position_mass = vec4(pos, m);	
	out_prev_position = vec4(pos_old,m);			  
	gl_Position = MVP*vec4(pos, 1);
	

	/*
	//Debug
	out_position_mass = vec4(ix,iy,0,1);
	out_prev_position = out_position_mass ;
	gl_Position = MVP*vec4(pos,1.0);
	*/
}