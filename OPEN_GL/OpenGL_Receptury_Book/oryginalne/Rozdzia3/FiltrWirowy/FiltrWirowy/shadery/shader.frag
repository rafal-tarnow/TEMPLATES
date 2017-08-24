#version 330 core
 
layout(location=0) out vec4 vFragColor;	//wyjœcie shadera fragmentów

//wejœcie z shadera wierzcho³ków
smooth in vec2 vUV;						//wspó³rzêdne tekstury 2D

//uniformy shadera
uniform sampler2D textureMap;			//obraz do deformacji
uniform float twirl_amount;				//moc filtra

void main()
{
	//przesuniêcie pocz¹tku wspó³rzêdnych tekstury na œrodek obrazu
	vec2 uv = vUV-0.5;
	
	//obliczanie k¹ta na podstawie przesuniêtych wspó³rzêdnych kartezjañskich
   float angle = atan(uv.y, uv.x);

   //obliczanie promieniana podstawie przesuniêtych wspó³rzêdnych kartezjañskich
   float radius = length(uv);

   //zwiêkszanie k¹ta o iloczym promienia i mocy filtra
   angle+= radius*twirl_amount; 

   //powrót do wspó³rzêdnych kartezjañskich
   vec2 shifted = radius* vec2(cos(angle), sin(angle));

   //przesuniêcie pocz¹tku wspó³rzêdnych tekstury do po³o¿enia pierwotnego
   vFragColor = texture(textureMap, (shifted+0.5)); 
}