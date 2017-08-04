#version 330 core
precision highp float;

#extension EXT_gpu_shader4 : require

layout( location = 0 )  in vec4 position_mass; //xyz -> położenie, w -> masa
layout( location = 1 )  in vec4 prev_position; //xyz -> położenie poprzednie, w -> masa
 
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
 
uniform mat4  ellipsoid_xform;		//transformacje elipsoidy
uniform mat4  inv_ellipsoid;		//odwrotne transformacje elipsoidy
uniform vec4  ellipsoid;			//(środek jako xyz, promień jako w) dla elipsoidy

//siła grawitacji
uniform vec3 gravity;
 
//wyjścia shadera 
out vec4 out_position_mass;		
out vec4 out_prev_position;
 

const vec3 center = vec3(0,0,0);
const float radius = 1;

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

//funkcja wykrywająca kolizję wierzchołka x ze sferą
void sphereCollision(inout vec3 x, vec4 sphere)
{
	vec3 delta = x - sphere.xyz;
	float dist = length(delta);
	if (dist < sphere.w) {
		x = sphere.xyz + delta*(sphere.w / dist);
	}
}

//funkcja wykrywająca kolizję wierzchołka x z płaszczyzną
void planeCollision(inout vec3 x,  vec4 plane) {
	 float dist = dot(plane.xyz,x)+ plane.w;
	 if(dist<0) {
		 x += plane.xyz*-dist;
	 }
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
	//rozwiń indeks liniowy do dwuwymiarowego
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

	//kolizję z podłożem zastąpiła kolizja z płaszczyzną 
	//pos.y=max(0, pos.y);

	//kolizja z płaszczyzną 
	//planeCollision(pos, vec4(0,1,0,0));

	//kolizja ze sferą
	//sphereCollision(pos, vec4(0,2,0,1)) ;

	//kolizja z prostopadłościanem
	///boxCollision(pos, pos_old, vec3(0,0,0), vec3(1,1,1));

	
	//kolizja z elipsoidą	
	//bring the current position to the ellipsoid object space
	vec4 x0 = inv_ellipsoid*vec4(pos,1); 

	//oblicz odległość od środka elipsoidy
	vec3 delta0 = x0.xyz-ellipsoid.xyz;
	float dist2 = dot(delta0, delta0);
	//jeśli odległość jest mniejsza od 1, mamy kolizję
	if(dist2<1) {  
		//wyznacz głębokośc penetracji 
		delta0 = (ellipsoid.w - dist2) * delta0 / dist2;

		//przetransformuj delta do pierwotnej przestrzeni
		vec3 delta;
		vec3 transformInv = vec3(ellipsoid_xform[0].x, ellipsoid_xform[1].x, ellipsoid_xform[2].x);
		transformInv /= dot(transformInv, transformInv);

		delta.x = dot(delta0, transformInv);
		transformInv = vec3(ellipsoid_xform[0].y, ellipsoid_xform[1].y, ellipsoid_xform[2].y);
		transformInv /= dot(transformInv, transformInv);

		delta.y = dot(delta0, transformInv);
		transformInv = vec3(ellipsoid_xform[0].z, ellipsoid_xform[1].z, ellipsoid_xform[2].z);
		transformInv /= dot(transformInv, transformInv);

		delta.z = dot(delta0, transformInv); 
		//zmień bieżące położenie o wartość delta
		pos +=  delta ; 

		//ustaw poprzednie położenie jako bieżące, przez co wypadkowa prędkość wyniesie 0
		pos_old = pos; //to zostało dodane, aby wypadkowa prędkość w następnej iteracji była równa zero
					   //usunięcie tego spowoduje, że kolidujące punkty będą ciągle podskakiwały
	}
	
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