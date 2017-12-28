#version 330 core

layout(location = 0) out vec4 vFragColor;	//wyjście shadera fragmentów

smooth in vec3 vUV;				//współrzędne tekstury 3D pobrane z shadera wierzchołków 
								//interpolowane przez rasteryzer

//uniformy
uniform sampler3D volume;		//dane wolumetryczne
uniform vec3 camPos;		//camera position
uniform vec3 step_size;	//ray step size 

//stałe
const int MAX_SAMPLES = 300;	//maksymalna liczba próbek dla każdego kroku
const vec3 texMin = vec3(0);	//minimalne współrzędne tekstury
const vec3 texMax = vec3(1);	//maksymalne współrzędne tekstury

void main()
{ 
	//współrzędne tekstury 3D potrzebne do próbkowania danych wolumetrycznych
	vec3 dataPos = vUV;

	//wyznaczanie kierunku promienia
	//wyznaczanie położenia wierzchołka w przestrzeni obiektu przez odjęcie 0.5 od 
	//współrzędnych tekstury 3D a potem odjęcie położenia kamey
	//i znormalizowanie wyniku
	vec3 geomDir = normalize((vUV-vec3(0.5)) - camPos); 

	//mnożenie kierunku promienia przez rozmiar kroku w celu wyznaczenia
	//rzeczywistego kroku, jaki promień powinien wykonać
	vec3 dirStep = geomDir * step_size; 
	 
	//wskaźnik końca pętli
	bool stop = false; 

	//dla wszystkich próbek na drodze promienia
	for (int i = 0; i < MAX_SAMPLES; i++) {
		//przesuń promień o rzeczywisty krok
		dataPos = dataPos + dirStep;
		
		
		//Stałe texMin i texMax mają wartości, odpowiednio, vec3(-1,-1,-1)
		//i vec3(1,1,1). Aby określić, czy promień 
		//opuścił obszar danych, używamy funkcji sign, która  
		//zwraca -1, jeśli jej argument ma wartość mniejszą od zera,
		//0, jeśli jest on równy zero, i 1, jeśli jest większy od zera. Zatem dla położeń skrajnych 
		//wywołania tej funkcji w postaci sign(dataPos-texMin) i sign (texMax-dataPos) 
		//dadzą vec3(1,1,1). 
		//Jeśli wymnożymy skalarnie dwa takie wektory, otrzymamy wartość 3. 
		//A zatem, jeśli promień będzie w obszarze danych, iloczyn skalarny da wartość mniejszą niż 3.
		// Jeśli wyjdzie więcej, to będzie oznaczało,
		//że promień jest już poza obszarem danych
		stop = dot(sign(dataPos-texMin),sign(texMax-dataPos)) < 3.0;

		//jeśli znacznik końca ma wartość TRUE, kończymy marsz promienia
		if (stop) 
			break;
		
		//pobieranie danych z czerwonego kanału tekstury wolumetrycznej
		float sample = texture(volume, dataPos).r;	
		
		//wyznaczanie przezroczystości fragmentu
		//używamy tutaj schematu kompozycyjnego przód-tył, w którym bieżąca próbka
		//jest mnożona przez zakumulowaną wartość składowej alfa, a wynik 
		//jest odejmowany od bieżącej próbki, w celu wyznaczenia składowej alfa z poprzednich kroków
		//Następnie wartość ta jest mnożona przez kolor bieżącej próbki i dodawana 
		//do zakumulowanego koloru fragmentu. //Składowa alfa z poprzednich kroków jest następnie 
		//dodawana do składowej alfa zakumulowanego koloru fragmentu.
		float prev_alpha = sample - (sample * vFragColor.a);
		vFragColor.rgb = prev_alpha * vec3(sample) + vFragColor.rgb; 
		vFragColor.a += prev_alpha; 
			
		//wcześniejsze zakończenie marszu promienia
		//jeśli aktualna wartość składowej alfa jest w pełni nasycona
		//kończymy pętlę
		if( vFragColor.a>0.99)
			break;
	} 
}