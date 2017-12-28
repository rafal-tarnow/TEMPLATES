#version 330 core
precision highp float;

#extension EXT_gpu_shader4 : require

layout( location = 0 )  in vec4 position;           //xyz -> położenie, w -> prędkość
layout( location = 1 )  in vec4 prev_position;      //xyz -> położenie poprzednie, w -> życie
layout( location = 2 )  in vec4 direction;			//xyz-> kierunek, w = 0
 
uniform mat4 MVP; //połączona macierz modelu, widoku i rzutowania
 
uniform float time; //czas bieżący
   
//stałe
const float PI = 3.14159;
const float TWO_PI = 2*PI;
const float PI_BY_2 = PI*0.5;
const float PI_BY_4 = PI_BY_2*0.5;
 
//wyjścia shadera 
out vec4 out_position;
out vec4 out_prev_position;
out vec4 out_direction;

const float DAMPING_COEFFICIENT =  0.9995;			//współczynnik określający opory ruchu
const vec3 emitterForce = vec3(0.0f,-0.001f, 0.0f);	//domyślny kierunek siły emitera
const vec4 collidor = vec4(0,1,0,0);				//płaszczyzna stanowiąca przeszkodę
const vec3 emitterPos = vec3(0);					//bieżące położenie emitera

//parametry orientacji emitera
float emitterYaw = (0.0f);
float emitterPitch	= PI_BY_2;
float emitterSpeed = 0.05f;
int emitterLife = 60;

//modyfikacje dla poszczególnych cząsteczek
float emitterYawVar	= TWO_PI;
float emitterPitchVar = PI_BY_4;
float emitterSpeedVar = 0.01f;
int   emitterLifeVar = 15;

//dla generatora liczb pseudolosowych
const float UINT_MAX = 4294967295.0;

//funkcja haszująca dla generatora liczb pseudolosowych
uint randhash(uint seed)
{
    uint i=(seed^12345391u)*2654435769u;
    i^=(i<<6u)^(i>>26u);
    i*=2654435769u;
    i+=(i<<5u)^(i>>12u);
    return i;
}
//zwraca liczbę pseudolosową z zakresu od 0 do 1
float randhashf(uint seed, float b)
{
    return float(b * randhash(seed)) / UINT_MAX;
}

//funkcja ta na podstawie wartości pochylenia i odchylenia zwraca wektor kierunku
//na sferze jednostkowej
void RotationToDirection(float pitch, float yaw, out vec3 direction)
{
	direction.x = -sin(yaw) * cos(pitch);
	direction.y = sin(pitch);
	direction.z = cos(pitch) * cos(yaw);
}

 
void main() 
{    
	//zapisz bieżące i poprzednie położenia cząsteczki
	vec3 prevPos = prev_position.xyz;
	int life = int(prev_position.w);
	vec3 pos  = position.xyz;

	//zapisz także bieżącą prędkość i kierunek ruchu cząsteczki
	float speed = position.w;
	vec3 dir = direction.xyz; 

	//jeśli życie cząsteczki >0, symulację można kontynuować
	if(life > 0) {
		prevPos = pos;
		pos += dir*speed; 
		if(dot(pos+emitterPos, collidor.xyz)+ collidor.w <0) {			 
			dir = reflect(dir, collidor.xyz);
			speed *= DAMPING_COEFFICIENT;
		}  
		dir += emitterForce;
		life--;
	
	//w przeciwnym razie generujemy nową cząsteczkę, podając sumę identyfikatora czątski + bieżącego czasu jako zarodek 
	//nowej cząstce przypisujemy losową wartość życia z ograniczeniem w postaci emiterowej wariacji życia
	//podobnie postępujemy z pozostałymi parametrami cząstki, prędkością, kierunkiem itd.
	}  else { 
		uint seed =   uint(time + gl_VertexID); 
		life = emitterLife + int(randhashf(seed++, emitterLifeVar));    
        float yaw = emitterYaw + (randhashf(seed++, emitterYawVar ));
		float pitch = emitterPitch + randhashf(seed++, emitterPitchVar);
		RotationToDirection(pitch, yaw, dir);		 
		float nspeed = emitterSpeed + (randhashf(seed++, emitterSpeedVar ));
		dir *= nspeed;  
		pos = emitterPos;
		prevPos = emitterPos; 
		speed = 1;
	}  

	//na koniec wyprowadzamy dane wyjściowe i wyznaczamy zmienną gl_Position 
	out_position = vec4(pos, speed);	
	out_prev_position = vec4(prevPos, life);		
	out_direction = vec4(dir, 0);
	gl_Position = MVP*vec4(pos, 1); 
}