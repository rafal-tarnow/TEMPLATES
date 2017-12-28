#version 330 core
 
layout(location=0) out vec4 vFragColor;	//wyj�cie shadera fragment�w

//wej�cie z shadera wierzcho�k�w
smooth in vec2 vUV;						//wsp�rz�dne tekstury 2D

//uniformy shadera
uniform sampler2D textureMap;			//obraz do deformacji
uniform float twirl_amount;				//moc filtra

void main()
{
	//przesuni�cie pocz�tku wsp�rz�dnych tekstury na �rodek obrazu
	vec2 uv = vUV-0.5;
	
	//obliczanie k�ta na podstawie przesuni�tych wsp�rz�dnych kartezja�skich
   float angle = atan(uv.y, uv.x);

   //obliczanie promieniana podstawie przesuni�tych wsp�rz�dnych kartezja�skich
   float radius = length(uv);

   //zwi�kszanie k�ta o iloczym promienia i mocy filtra
   angle+= radius*twirl_amount; 

   //powr�t do wsp�rz�dnych kartezja�skich
   vec2 shifted = radius* vec2(cos(angle), sin(angle));

   //przesuni�cie pocz�tku wsp�rz�dnych tekstury do po�o�enia pierwotnego
   vFragColor = texture(textureMap, (shifted+0.5)); 
}