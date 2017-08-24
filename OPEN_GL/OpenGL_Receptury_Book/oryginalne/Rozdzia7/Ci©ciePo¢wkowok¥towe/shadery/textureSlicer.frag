#version 330 core

layout(location = 0) out vec4 vFragColor;	//wyjście shadera fragmentów


smooth in vec3 vUV;	//współrzędne tekstury 3D pobrane z shadera wierzchołków 
					//interpolowane przez rasteryzer

//uniformy
uniform sampler3D volume;	//dane wolumetryczne
uniform vec4 color;			//stały kolor, przez który należy wymnożyć  
							//dane wolumetryczne	

void main()
{
	//tutaj próbkujemy dane wolumetryczne, posługując się współrzędnymi tekstury pobranymi z shadera wierzchołków
	//Ponieważ w czasie tworzenia tekstury nadaliśmy jej wewnętrzny format GL_RED,
	//możemy teraz pobierać próbki danych wolumetrycznych z kanału czerwonego  Tutaj ustawiamy wszystkie cztery 
	//składowe jednakowo, aby uzyskać odcień szarości i mnożymy
	//ten odcień szarości przez kolor zadany w uniformie, aby uzyskać ostateczny kolor fragmentu
	vFragColor = texture(volume, vUV).rrrr * color ;	 
}
