#version 330 core
  
smooth out vec4 vSmoothColor;	//wyjście do shadera fragmentów

//uniformy shadera
uniform mat4 MVP;				//połączona macierz modelu, widoku i rzutowania
uniform float time;				//czas bieżący
 
//atrybuty cząsteczek
const vec3 a = vec3(0,2,0);		//przyspieszenie cząsteczek
//vec3 g = vec3(0,-9.8,0);		//jeśli chcesz uwzględnić grawitację 

const float rate = 1/500.0;		//tempo emisji
const float life = 2;			//czas życia cząsteczki

//stałe
const float PI = 3.14159;
const float TWO_PI = 2*PI;

//kolory mapy kolorów
const vec3 RED = vec3(1,0,0);
const vec3 GREEN = vec3(0,1,0);
const vec3 YELLOW = vec3(1,1,0); 

//generator liczb pseudolosowych
float rand(vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

//kierunek pseudolosowy
vec3 uniformRadomDir(vec2 v, out vec2 r) {
	r.x = rand(v.xy);
	r.y = rand(v.yx);
	float theta = mix(0.0, PI / 6.0, r.x);
	float phi = mix(0.0, TWO_PI, r.y);
	return vec3(sin(theta) * cos(phi), cos(theta), sin(theta) * sin(phi));
}

void main()
{
	//zmienne lokalne
	vec3 pos=vec3(0);

	//wyznaczanie czasu cząsteczki przez mnożenie identyfikatora wierzchołka przez
	//tempo emisji
	float t = gl_VertexID*rate;
	//na początku alpha = 1 (cząsteczka jest w pełni widoczna)
	float alpha = 1;
	
	if(time>t) {
		float dt = mod((time-t), life);
		vec2 xy = vec2(gl_VertexID,t); 
		vec2 rdm=vec2(0);
		//emiter punktowy
		//pos = ((uniformRadomDir(xy) + 0.5*(a+g)*dt)*dt); //z grawitacją 	   
	   
		pos = ((uniformRadomDir(xy, rdm) + 0.5*a*dt)*dt);       //efekt ognia dla emitera punktowego
	   
		/*
	    pos = ( uniformRadomDir(xy, rdm) + 0.5*a*dt)*dt;       //efekt ognia dla emitera czworokątnego
	    vec2 rect = (rdm*2.0 - 1.0) ;
	    pos += vec3(rect.x, 0, rect.y) ;
	    */
	    
	    /*
	    pos = ( uniformRadomDir(xy, rdm) + 0.5*a*dt)*dt;       //efekt ognia dla emitera kolistego
	    vec2 rect = (rdm*2.0 - 1.0);
	    float dotP = dot(rect, rect);
	    
	    if(dotP<1)
	        pos += vec3(rect.x, 0, rect.y) ;
	    */
	    
		alpha = 1.0 - (dt/life);	  
	}
   
	//liniowe przejścia między kolorami czerwonym i żółtym
	vSmoothColor = vec4(mix(RED,YELLOW,alpha),alpha);
	//wyznaczanie położenia w przestrzeni oka
	gl_Position = MVP*vec4(pos,1);
}
