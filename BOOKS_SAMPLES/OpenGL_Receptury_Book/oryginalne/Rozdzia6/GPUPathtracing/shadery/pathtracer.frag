#version 330 core

layout(location = 0) out vec4 vFragColor; //wyjście shadera fragmentów


//struktury dla promienia, prostopadłościanu i kamery
struct Ray { vec3 origin, dir;} eyeRay; 
struct Box { vec3 min, max; };
struct Camera {
   vec3 U,V,W; 
   float d;
}cam;


//wejście z shadera wierzchołków
smooth in vec2 vUV;					//interpolowane współrzędne tekstury

//uniformy shadera
uniform mat4 invMVP;					//inverse of combined modelview projection matrix
uniform vec4 backgroundColor;			//kolor tła
uniform vec3 eyePos; 					//położenie oka w przestrzenie obiektu
uniform sampler2D vertex_positions;		//wierzchołki siatki
uniform isampler2D triangles_list;		//trójkąty siatki
uniform sampler2DArray textureMaps;		//wszystkie tekstury siatki
uniform vec3 light_position;			//położenie światła w przestrzenie obiektu
uniform Box aabb;	 					//prostopadłościan otaczający scenę 
uniform float VERTEX_TEXTURE_SIZE; 		//rozmiar tekstury wierzchołka
uniform float TRIANGLE_TEXTURE_SIZE; 	//rozmiar tekstury trójkąta 
uniform float time;						//czas bieżący

//stałe shadera
const int MAX_BOUNCES = 3;	//maksymalna liczba odbić dla każdego promienia

//funkcja, która ma wykrywać trafienie promienia w prostopadłościan
//zwraca wektor vec2, w którym współrzędna x zawiera wartość parametru t dla najbliższego trafienia
						//a współrzędna y zawiera wartość parametru t dla dalszego trafienia
vec2 intersectCube(vec3 origin, vec3 ray, Box cube) {		
	vec3   tMin = (cube.min - origin) / ray;		
	vec3   tMax = (cube.max - origin) / ray;		
	vec3     t1 = min(tMin, tMax);		
	vec3     t2 = max(tMin, tMax);
	float tNear = max(max(t1.x, t1.y), t1.z);
	float  tFar = min(min(t2.x, t2.y), t2.z);
	return vec2(tNear, tFar);	
}

//kierunek z położenia 2D i kamery
vec3 get_direction(vec2 p, Camera c) {
   return normalize(p.x * c.U + p.y * c.V + c.d * c.W);   
}

//generuje promień oka dla danego położenia 2D i kamery
void setup_camera(vec2 uv) {
 
  eyeRay.origin = eyePos; 
    
  cam.U = (invMVP*vec4(1,0,0,0)).xyz; 
  cam.V = (invMVP*vec4(0,1,0,0)).xyz; 
  cam.W = (invMVP*vec4(0,0,1,0)).xyz; 
  cam.d = 1;    
  
  eyeRay.dir = get_direction(uv , cam); 
  eyeRay.dir += cam.U*uv.x;
  eyeRay.dir += cam.V*uv.y;  
}

//procedura wykrywania trafienia w trójkąt. Normalna jest zwracana w  
//referencyjnym argumencie
//
//Wartością zwracaną jest vec4 z 
//x -> t wartość w punkcie trafienia.
///y -> u współrzędna tekstury
//z -> v współrzędna tekstury
//w ->identyfikator mapy tekstury 
vec4 intersectTriangle(vec3 origin, vec3 dir, int index, out vec3 normal ) {
	 
	ivec4 list_pos = texture(triangles_list, vec2((index+0.5)/TRIANGLE_TEXTURE_SIZE, 0.5));
	if((index+1) % 2 !=0 ) { 
		list_pos.xyz = list_pos.zxy;
	}  
	vec3 v0 = texture(vertex_positions, vec2((list_pos.z + 0.5 )/VERTEX_TEXTURE_SIZE, 0.5)).xyz;
	vec3 v1 = texture(vertex_positions, vec2((list_pos.y + 0.5 )/VERTEX_TEXTURE_SIZE, 0.5)).xyz;
	vec3 v2 = texture(vertex_positions, vec2((list_pos.x + 0.5 )/VERTEX_TEXTURE_SIZE, 0.5)).xyz;
	  
	vec3 e1 = v1-v0;
	vec3 e2 = v2-v0;

	vec3 tvec = origin - v0;  
	
	vec3 pvec = cross(dir, e2);  
	float  det  = dot(e1, pvec);   

	float inv_det = 1.0/ det;  

	float u = dot(tvec, pvec) * inv_det;  

	if (u < 0.0 || u > 1.0)  
		return vec4(-1,0,0,0);  

	vec3 qvec = cross(tvec, e1);  

	float v = dot(dir, qvec) * inv_det;  

	if (v < 0.0 || (u + v) > 1.0)  
		return vec4(-1,0,0,0);  

	float t = dot(e2, qvec) * inv_det;
	if((index+1) % 2 ==0 ) {
		v = 1-v; 
	} else {
		u = 1-u;
	} 
	
	normal = normalize(cross(e2,e1));
	return vec4(t,u,v,list_pos.w);
}

//generator liczb pseudolosowych
float random(vec3 scale, float seed) {		
	return fract(sin(dot(gl_FragCoord.xyz + seed, scale)) * 43758.5453 + seed);	
}	

//zwraca losowy kierunek
vec3 uniformlyRandomDirection(float seed) {		
	float u = random(vec3(12.9898, 78.233, 151.7182), seed);		
	float v = random(vec3(63.7264, 10.873, 623.6736), seed);		
	float z = 1.0 - 2.0 * u;		
	float r = sqrt(1.0 - z * z);	
	float angle = 6.283185307179586 * v;	
	return vec3(r * cos(angle), r * sin(angle), z);	
}	

//zwraca losowy wektor
vec3 uniformlyRandomVector(float seed) 
{		
	return uniformlyRandomDirection(seed) *  (random(vec3(36.7539, 50.3658, 306.2759), seed));	
}

//funkcja sprawdzająca, czy promień trafił w obiekt
//jeśli tak, zwraca 0.5, a jeśli nie, zwraca 1 To przyciemnia kolor
//symulując cień
float shadow(vec3 origin, vec3 dir ) {
	vec3 tmp;
	for(int i=0;i<int(TRIANGLE_TEXTURE_SIZE);i++) 
	{
		vec4 res = intersectTriangle(origin, dir, i, tmp); 
		if(res.x>0 ) { 
		   return 0.5;   
		}
	}
	return 1.0;
}

//funkcja śledząca promień wychodzący ze źródła światła
vec3 pathtrace(vec3 origin, vec3 ray, vec3 light, float t) {		

	//ustaw zmienną akumulacyjną na 0
	//ustaw też maskę koloru na 1 i kolor powierzchni na kolor tła
	vec3 colorMask = vec3(1.0);		
	vec3 accumulatedColor = vec3(0.0);
	vec3 surfaceColor=vec3(backgroundColor.xyz);
	
	float diffuse = 1;
	//dla wszystkich odbić
	for(int bounce = 0; bounce < MAX_BOUNCES; bounce++) {			
		//sprawdź, czy promień trafia w prostopadłościan otaczający scenę
		vec2 tNearFar = intersectCube(origin, ray,  aabb);
		
		//kontynuuj, jeśli nie trafił
		if(  tNearFar.x > tNearFar.y)  
		   continue; 
		
		//jeśli wcześniej nie było bliższego trafienia, zapisz parametr t
		if(tNearFar.y<t)
			t =   tNearFar.y+1;					
		
		vec3 N;
		vec4 val=vec4(t,0,0,0); 

		//sprawdź wszystkie trójkąty na okoliczność trafienia przez promień
		for(int i=0;i<int(TRIANGLE_TEXTURE_SIZE);i++) 
		{
			vec3 normal;
			vec4 res = intersectTriangle(origin, ray, i, normal); 
			//jeśli trafienie mało miejsce, zapisz współrzędne i normalną
		 	if(res.x>0.001 && res.x <  val.x) { 
			   val = res;   
			   N = normal;
		    }
		}
		   
		//jeśli trafienie jest prawidłowe
		if(  val.x < t) {			  	
			//oblicz kolor powierzchni 
			surfaceColor = mix(texture(textureMaps, val.yzw), vec4(1), (val.w==255) ).xyz; 
			
			//przenieś początek promienia do punktu trafienia
			//and get a new random ray direction 
			vec3 hit = origin + ray * val.x;	
			origin = hit;	
			ray = uniformlyRandomDirection(time + float(bounce));	
			
			//zmień położenie światła, aby zminimalizować artefakty
			vec3  jitteredLight  =  light + ray;
			vec3 L = normalize(jitteredLight - hit);			
			
			//wyznacz składowej rozpraszania
			diffuse = max(0.0, dot(L, N ));			 		
			colorMask *= surfaceColor;			

			//sprawdź cienie
			float inShadow = shadow(hit+ N*0.0001, L);

			//akumuluj kolor
			accumulatedColor += colorMask * diffuse * inShadow; 
			t = val.x;
		}  			
	}		
	//jeśli zakumulowany kolor jest równy 0, to znaczy, że promień nie trafił w żadną geometrię
	//zwróć iloczyn składowej rozproszenia i koloru powierzchni
	if(accumulatedColor == vec3(0))
		return surfaceColor*diffuse;
	else
		//w przeciwnym razie podziel zakumulowany kolor przez maksymalną liczbę odbić
		return accumulatedColor/float(MAX_BOUNCES-1);	
}	

void main()
{ 
	//ustaw maksymalną wartość t
	float t = 10000;  
	
	//ustaw kolor fragmentu zgodny z kolorem tła 
	vFragColor = backgroundColor;

	//ustaw kamerę wg współrzędnych tekstury
	setup_camera(vUV);
	
	//sprawdź, czy promień trafia w prostopadłościan otaczający scenę 
	vec2 tNearFar = intersectCube(eyeRay.origin, eyeRay.dir,  aabb);

	//jeśli trafienie jest prawidłowe
	if(tNearFar.x<tNearFar.y  ) {
		t = tNearFar.y+1; //przesuń bliskie trafienie, aby uniknąć artefaktów
		  		 
		//śledź promień wychodzący ze źródła światła w losowo wybranym kierunku		
		vec3 light = light_position + uniformlyRandomVector(time);

		//tutaj wykonaj śledzenie ścieżki 
		vFragColor = vec4(pathtrace(eyeRay.origin, eyeRay.dir, light, t),1);		 
	} 
}

