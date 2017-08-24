#version 330 core

layout(location = 0) out vec4 vFragColor0;	//wyjście do celu nr 0
layout(location = 1) out vec4 vFragColor1;	//wyjście do celu nr 1
layout(location = 2) out vec4 vFragColor2;	//wyjście do celu nr 2	
 
//uniformy
uniform vec4 vColor;		//stały kolor kostki
uniform sampler2DRect  depthBlenderTex;	//wyjście mieszania głębi
uniform sampler2DRect  frontBlenderTex;	//wyjście mieszania przedniego
uniform float alpha;	//wartość alfa fragmentu

#define MAX_DEPTH 1.0	//maksymalna wartość głębi do czyszczenia bufora głębi
 

void main()
{
	//wyznaczanie głębi bieżącego fragmentu
	float fragDepth = gl_FragCoord.z;
	//odczytywanie głębi z wyjścia mieszającego
	vec2 depthBlender = texture(depthBlenderTex, gl_FragCoord.xy).xy;
	//odczytywanie wyjścia mieszania przedniego
	vec4 forwardTemp = texture(frontBlenderTex, gl_FragCoord.xy);

	//Głębokości i 1.0-aplhaMult zawsze rosną,
	//więc możemy wykonać przejście z mieszaniem MAX
	vFragColor0.xy = depthBlender;
	
	//kolory przednie zawsze rosną (DST += SRC*ALPHA_MULT),
	//więc możemy wykonać przejście z mieszaniem MAX
	vFragColor1 = forwardTemp;
	
	//Ponieważ mieszanie nakładkowe powoduje zwiększenie lub zmniejszenie koloru,
	//nie możemy wykonać przejścia standardowego
	//w każdym przejściu tylko jeden fragment może mieć wartość koloru większą niż 0
	vFragColor2 = vec4(0.0);

	float nearestDepth = -depthBlender.x;
	float farthestDepth = depthBlender.y;
	float alphaMultiplier = 1.0 - forwardTemp.w;

	if (fragDepth < nearestDepth || fragDepth > farthestDepth) {
		//pomiń tę głębię w algorytmie peelingu
		vFragColor0.xy = vec2(-MAX_DEPTH);
		return;
	}
	
	if (fragDepth > nearestDepth && fragDepth < farthestDepth) {
		//ten fragment musi być peelingowany jeszcze raz
		vFragColor0.xy = vec2(-fragDepth, fragDepth);
		return;
	}	 
	
	//jeśli zrobiliśmy to tutaj, ten fragment znajduje się teraz na warstwie peelingu z ostatniego przejścia
	//dlatego trzeba go zasłonić, żeby nie był ponownie brany pod uwagę w peelingu
	vFragColor0.xy = vec2(-MAX_DEPTH);
	
	//jeśli głęba bieżącego jest najmniejszą głębią, mieszamy kolor 
	//do drugiego przyłącza
	if (fragDepth == nearestDepth) {
		vFragColor1.xyz += vColor.rgb * alpha * alphaMultiplier;
		vFragColor1.w = 1.0 - alphaMultiplier * (1.0 - alpha);
	} else {
		//w przeciwnym razie zapisujemy do przyłącza trzeciego
		vFragColor2 += vec4(vColor.rgb, alpha);
	}
}