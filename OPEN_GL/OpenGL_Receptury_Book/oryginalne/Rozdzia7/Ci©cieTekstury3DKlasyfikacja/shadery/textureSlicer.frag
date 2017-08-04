#version 330 core

layout(location = 0) out vec4 vFragColor;	//wyjście shadera fragmentów

smooth in vec3 vUV;			//współrzędne tekstury 3D pobrane z shadera wierzchołków i interpolowane przez rasteryzer

//uniformy
uniform sampler3D volume;	//dane wolumetryczne
uniform sampler1D lut;		//tekstura (tablica LUT) funkcji przejścia

void main()
{
    //tutaj próbkujemy dane wolumetryczne, posługując się współrzędnymi tekstury pobranymi z shadera wierzchołków
	//Ponieważ w czasie tworzenia tekstury nadaliśmy jej wewnętrzny format GL_RED,
	//możemy teraz pobierać próbki danych wolumetrycznych z kanału czerwonego  Następnie bierzemy wartość  
	//pobraną z danych wolumetrycznych i wyszukujemy odpowiedni dla niej kolor w teksturze funkcji przejścia 
	vFragColor = texture(lut, texture(volume, vUV).r);
}