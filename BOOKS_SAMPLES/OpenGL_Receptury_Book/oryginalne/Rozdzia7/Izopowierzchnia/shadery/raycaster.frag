#version 330 core

layout(location = 0) out vec4 vFragColor;	//wyjście shadera fragmentów

smooth in vec3 vUV;				//współrzędne tekstury 3D pobrane z shadera wierzchołków 
								//interpolowane przez rasteryzer

//uniformy
uniform sampler3D volume;			//dane wolumetryczne
uniform vec3 camPos;			//położenie kamery 
uniform vec3 step_size;		//długość kroku promienia 

//stałe
const int MAX_SAMPLES = 300;		//maksymalna liczba próbek dla każdego kroku
const vec3 texMin = vec3(0);		//minimalne współrzędne tekstury
const vec3 texMax = vec3(1);		//maksymalne współrzędne tekstury
const float DELTA = 0.01;			//rozmiar kroku w obliczeniach gradientu
const float isoValue = 40/255.0;	//wartość na izopowierzchni

//funkcja uściślająca położenie miejsca przecięcia z izopowierzchnią o wartości iso
//przy zadanych początkowych granicach przedziału (left i right)
vec3 Bisection(vec3 left, vec3 right , float iso)
{ 
	//4 przebiegi pętli
	for(int i=0;i<4;i++)
	{ 
		//wyznaczanie środka przedziału
		vec3 midpoint = (right + left) * 0.5;
		//pobieranie próbki w środku przedziału
		float cM = texture(volume, midpoint).x ;
		//sprawdzanie, czy wartość w środku przedziału jest mniejsza od wartości iso
		if(cM < iso)
			//jeśli tak, lewy koniec przedziału przesuwamy na środek
			left = midpoint;
		else
			//jeśli nie, na środek przesuwamy prawy koniec przedziału 
			right = midpoint; 
	}
	//ostatecznie zwracane jest położenie środka przedziału
	return vec3(right + left) * 0.5;
}

//funkcja obliczająca gradient pola danych w obszarze wolumetrycznym
//funkcja wykorzystuje metodę aproksymowania za pomocą różnic skończonych 
//
vec3 GetGradient(vec3 uvw) 
{
	vec3 s1, s2;  

	//wyznaczanie centralnego ilorazu różnicowego 
	s1.x = texture(volume, uvw-vec3(DELTA,0.0,0.0)).x ;
	s2.x = texture(volume, uvw+vec3(DELTA,0.0,0.0)).x ;

	s1.y = texture(volume, uvw-vec3(0.0,DELTA,0.0)).x ;
	s2.y = texture(volume, uvw+vec3(0.0,DELTA,0.0)).x ;

	s1.z = texture(volume, uvw-vec3(0.0,0.0,DELTA)).x ;
	s2.z = texture(volume, uvw+vec3(0.0,0.0,DELTA)).x ;
	 
	return normalize((s1-s2)/2.0); 
}

//funkcja obliczająca cieniowanie Phonga na podstawie wektora światła (L)
//normalnej (N), wektora widoku (V), siły odblasku (specPower) i
//koloru rozproszenia (diffuseColor) //Najpierw wyznaczana jest składowa ozpraszania
//następnie obliczany jest wektor połówkowy potrzebny do wyznaczenia składowej odblaskowej Na koniec
//obie składowe są sumowane
vec4 PhongLighting(vec3 L, vec3 N, vec3 V, float specPower, vec3 diffuseColor)
{
	float diffuse = max(dot(L,N),0.0);
	vec3 halfVec = normalize(L+V);
	float specular = pow(max(0.00001,dot(halfVec,N)),specPower);	
	return vec4((diffuse*diffuseColor + specular),1.0);
}

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
		float sample = texture(volume, dataPos).r;			//próbka bieżąca
		float sample2 = texture(volume, dataPos+dirStep).r;	//próbka następna

		//Jeśli renderujemy izopowierzchnię, nie stosujemy mieszania, 
		//lecz znajdujemy miejsca przecięcia promienia z izopowierzchnią 
		//przez testowanie dwóch kolejno pobranych próbek 
		if( (sample -isoValue) < 0  && (sample2-isoValue) >= 0.0)  {
			//jeśli między próbkami jest punkt przecięcia, uściślamy jego położenie 
			//stosując metodę bisekcji
			vec3 xN = dataPos;
			vec3 xF = dataPos+dirStep;	
			vec3 tc = Bisection(xN, xF, isoValue);	
	
			//To zwraca pierwszą napotkaną izopowierzchnię
			//vFragColor = make_float4(xN,1);
          	
			//Aby uzyskać izopowierzchnię cieniowaną, najpierw wyznaczamy normalną
			// w punkcie przecięcia 
			vec3 N = GetGradient(tc);					

			//Wektor widoku jest po prostu przeciwny do  
			//kierunku promienia
			vec3 V = -geomDir;

			//Wektor widoku będzie również wektorem światła, co będzie symulowało  
			//światło czołówki
			vec3 L =  V;

			//Na koniec wywołujemy funkcję PhongLighing, aby wyznaczyć ostateczny kolor
			//ze składowymi rozproszenia i odblasku. Spróbuj wywołać to, 
			//vFragColor =  PhongLighting(L,N,V,250,  tc); //aby uzyskać kolorową
			//izopowierzchnię
			vFragColor =  PhongLighting(L,N,V,250, vec3(0.5));	
			break;
		} 
	} 
}