#pragma once
#include <GL/freeglut.h>
#include <string.h>
#include <glm/glm.hpp>
#include <vector>

//struktura do przechowywania normalnych i położeń wierzchołków
struct Vertex {
	glm::vec3 pos, normal;
};

//klasa TetrahedraMarcher
class TetrahedraMarcher
{
public:
	//konstruktor i destruktor
	TetrahedraMarcher(void);
	~TetrahedraMarcher(void);

	//funkcja ustalająca wymiary obszaru wolumetrycznego
	void SetVolumeDimensions(const int xdim, const int ydim, const int zdim);
	
	//funkcja do ustalania liczby próbkowanych wokseli
	//more voxels will give a higher density mesh
	void SetNumSamplingVoxels(const int x, const int y, const int z);
	
	//ustalenie wartości określającej izopowierzchnię
	void SetIsosurfaceValue(const GLubyte value);
	
	//wczytanie danych wolumetrycznych
	bool LoadVolume(const std::string& filename);
	
	//przemierzanie obszaru wolumetrycznego
	void MarchVolume();
	
	//ustalanie całkowitej liczby wygenerowanych wierzchołków
	size_t GetTotalVertices();
	
	//pobieranie wskaźnika do bufora wierzchołków
	Vertex* GetVertexPointer();

protected:
	//funkcja próbkująca; pobiera współrzędne x, y i z, a zwraca wartość wolumetryczną 
	//istniejącą we wskazanym miejscu
	GLubyte SampleVolume(const int x, const int y, const int z);
	
	//wyznaczanie normalnej w podanym miejscu metodą aproksymowania za pomocą różnic skończonych
	glm::vec3 GetNormal(const int x, const int y, const int z);
	
	//próbkowanie woksela w podanym miejscu
	void SampleVoxel(const int x, const int y, const int z, glm::vec3 scale);
	
	//zwraca przesunięcie między dwiema wartościami próbek
	float GetOffset(const GLubyte v1, const GLubyte v2);

	//wymiary obszaru wolumetrycznego i ich odwracanie
	int XDIM, YDIM, ZDIM;
	glm::vec3 invDim;

	//odległości między próbkami wyrażone w wokselach
	int X_SAMPLING_DIST;
	int Y_SAMPLING_DIST;
	int Z_SAMPLING_DIST;

	//wskaźnik do danych wolumetrycznych
	GLubyte* pVolume;
	
	//poszukiwana stała wartość (izowartość)
	GLubyte isoValue; 
	
	//wektor do przechowywania normalnych i położeń wierzchołków
	std::vector<Vertex> vertices; 
};

