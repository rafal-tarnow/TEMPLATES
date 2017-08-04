#include "TetrahedraMarcher.h"
#include <fstream>
#include "Tables.h"

TetrahedraMarcher::TetrahedraMarcher(void)
{
	XDIM = 256;
	YDIM = 256;
	ZDIM = 256;
	pVolume = NULL; 

} 

TetrahedraMarcher::~TetrahedraMarcher(void)
{ 
	if(pVolume!=NULL) {
		delete [] pVolume;
		pVolume = NULL;
	}
}

void TetrahedraMarcher::SetVolumeDimensions(const int xdim, const int ydim, const int zdim) {
	XDIM = xdim;
	YDIM = ydim;
	ZDIM = zdim;
	invDim.x = 1.0f/XDIM; 
	invDim.y = 1.0f/YDIM; 
	invDim.z = 1.0f/ZDIM; 
}
void TetrahedraMarcher::SetNumSamplingVoxels(const int x, const int y, const int z) {
	X_SAMPLING_DIST = x;
	Y_SAMPLING_DIST = y;
	Z_SAMPLING_DIST = z;
}
void TetrahedraMarcher::SetIsosurfaceValue(const GLubyte value) {
	isoValue = value;
}

bool TetrahedraMarcher::LoadVolume(const std::string& filename) {
	std::ifstream infile(filename.c_str(), std::ios_base::binary); 

	if(infile.good()) {
		pVolume = new GLubyte[XDIM*YDIM*ZDIM];
		infile.read(reinterpret_cast<char*>(pVolume), XDIM*YDIM*ZDIM*sizeof(GLubyte));
		infile.close();
		return true;
	} else {
		return false;
	}
} 

void TetrahedraMarcher::SampleVoxel(const int x, const int y, const int z, glm::vec3 scale) {
	GLubyte cubeCornerValues[8];
	int flagIndex, edgeFlags, i;
	glm::vec3 edgeVertices[12];
	glm::vec3 edgeNormals[12];

	//utwórz lokalną kopię wartości występujących w narożnikach sześcianu
	for( i = 0; i < 8; i++) {
		cubeCornerValues[i] = SampleVolume( x + (int)(a2fVertexOffset[i][0]*scale.x), 
											y + (int)(a2fVertexOffset[i][1]*scale.y), 
											z + (int)(a2fVertexOffset[i][2]*scale.z));
	}

	//Zbadaj, które wierzchołki są wewnątrz powierzchni, a które nie
	//utwórz indeks znacznikowy informujący o tym, czy wartość w danym narożniku sześcianu jest mniejsza 
	//niż zadana izowartość
	flagIndex = 0;
	for( i= 0; i<8; i++)	{
		if(cubeCornerValues[i] <= isoValue) 
			flagIndex |= 1<<i;
	}

	//Sprawdź, które krawędzie są przecinane przez izopowierczhnię 
	edgeFlags = aiCubeEdgeFlags[flagIndex];
		
	//jeśli sześcian jest cały wewnątrz lub na zewnątrz izopowierzchni, żadna krawędź nie jest przecinana
	if(edgeFlags == 0) 
	{
		return;
	}

	//dla wszystkich krawędzi
	for(i = 0; i < 12; i++)
	{
		//jeśli ta krawędź jest przecinana
		if(edgeFlags & (1<<i))
		{
			//pobierz względne współrzędne jej końców 
			float offset = GetOffset(cubeCornerValues[ a2iEdgeConnection[i][0] ], cubeCornerValues[ a2iEdgeConnection[i][1] ]);

			//i na ich podstawie wyznacz położenie punktu przecięcia
			edgeVertices[i].x = x + (a2fVertexOffset[ a2iEdgeConnection[i][0] ][0]  +  offset * a2fEdgeDirection[i][0])*scale.x ;
			edgeVertices[i].y = y + (a2fVertexOffset[ a2iEdgeConnection[i][0] ][1]  +  offset * a2fEdgeDirection[i][1])*scale.y ;
			edgeVertices[i].z = z + (a2fVertexOffset[ a2iEdgeConnection[i][0] ][2]  +  offset * a2fEdgeDirection[i][2])*scale.z ;
			
			//i na podstawie położenia punktu przecięcia wyznacz normalną
			edgeNormals[i] = GetNormal( (int)edgeVertices[i].x ,  (int)edgeVertices[i].y ,  (int)edgeVertices[i].z  );
		}
	}

	//Utwórz trójkąty.   Może ich być maksymalnie 5 dla jednego sześcianu
	for(i = 0; i< 5; i++)
	{
		if(a2iTriangleConnectionTable[flagIndex][3*i] < 0)
			break;

		for(int j= 0; j< 3; j++)
		{
			int vertex = a2iTriangleConnectionTable[flagIndex][3*i+j]; 
			Vertex v;
			v.normal = (edgeNormals[vertex]); 
			v.pos = (edgeVertices[vertex])*invDim; 
			vertices.push_back(v);
		}
	} 
}

void TetrahedraMarcher::MarchVolume() {
	vertices.clear(); 
	int dx = XDIM/X_SAMPLING_DIST;
	int dy = YDIM/Y_SAMPLING_DIST;
	int dz = ZDIM/Z_SAMPLING_DIST;
	glm::vec3 scale = glm::vec3(dx,dy,dz); 
	for(int z=0;z<ZDIM;z+=dz) {
		for(int y=0;y<YDIM;y+=dy) {
			for(int x=0;x<XDIM;x+=dx) {
				SampleVoxel(x,y,z, scale);
			}
		}
	}
}
  
size_t TetrahedraMarcher::GetTotalVertices() {
	return vertices.size();
}
Vertex* TetrahedraMarcher::GetVertexPointer() {
	return  &vertices[0];
} 

GLubyte TetrahedraMarcher::SampleVolume(const int x, const int y, const int z) {
  	int index = (x+(y*XDIM)) + z*(XDIM*YDIM); 
	if(index<0)
		index = 0;
	if(index >= XDIM*YDIM*ZDIM)
		index = (XDIM*YDIM*ZDIM)-1;
	return pVolume[index];
}

glm::vec3 TetrahedraMarcher::GetNormal (const int x, const int y, const int z) { 
	glm::vec3 N;
	N.x =  (SampleVolume(x-1,y,z)-SampleVolume(x+1,y,z))*0.5f  ;
	N.y =  (SampleVolume(x,y-1,z)-SampleVolume(x,y+1,z))*0.5f ;
	N.z =  (SampleVolume(x,y,z-1)-SampleVolume(x,y,z+1))*0.5f ;
	return glm::normalize(N);
}
float TetrahedraMarcher::GetOffset(const GLubyte v1, const GLubyte v2) {
	float delta = (float)(v2-v1);
	if(delta == 0)
		return 0.5f;
	else
		return (isoValue-v1)/delta;
}

