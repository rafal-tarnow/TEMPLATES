#include "Ezm.h"  
#include "3rdParty/pugi_xml/pugixml.hpp"
 
#include <iostream>



NVSHARE::MeshSystemContainer *msc = NULL;
NVSHARE::MeshImport *meshImportLibrary = NULL;

EzmLoader::EzmLoader() {
	//ładowanie siatek przy użyciu biblioteki MeshImport
	meshImportLibrary = NVSHARE::loadMeshImporters(".");
}

EzmLoader::~EzmLoader() {
	if(meshImportLibrary)
		meshImportLibrary->releaseMeshSystemContainer(msc);
}

#include <fstream>
#include <sstream>

std::string trim(const std::string& str,
                 const std::string& whitespace = " \t")
{
    const auto strBegin = str.find_first_not_of(whitespace);
    if (strBegin == std::string::npos)
        return ""; //bez zawartości

    const auto strEnd = str.find_last_not_of(whitespace);
    const auto strRange = strEnd - strBegin + 1;

    return str.substr(strBegin, strRange);
}
 
bool EzmLoader::Load(const string& filename, vector<Bone>& skeleton, vector<NVSHARE::MeshAnimation>& animations, vector<SubMesh>& submeshes, vector<Vertex>& vertices, vector<unsigned short>& indices, map<std::string, std::string>& materialNames, glm::vec3& min, glm::vec3& max) {
	  
	
	min=glm::vec3(1000.0f);
	max=glm::vec3(-1000.0f);
	

	pugi::xml_document doc;
	pugi::xml_parse_result res = doc.load_file(filename.c_str()); 
		
	//wczytaj materiały z pliku EZMesh, używając biblioteki pugixml
	pugi::xml_node mats = doc.child("MeshSystem").child("Materials");
	int totalMaterials = atoi(mats.attribute("count").value());
	pugi::xml_node material = mats.child("Material");
	std::cerr<<"Reading materials ...";

	//dla wszystkich materiałów 
	for(int i=0;i<totalMaterials;i++) {
		//pobierz nazwę materiału 
		std::string name = material.attribute("name").value();
		std::string metadata = material.attribute("meta_data").value();
		if(metadata.find("%") != std::string::npos) {
			int startGarbage = metadata.find_first_of("%20");
			int endGarbage = metadata.find_last_of("%20");
		
			if(endGarbage!= std::string::npos)
				metadata = metadata.erase(endGarbage-2, 3);
			if(startGarbage!= std::string::npos)
				metadata = metadata.erase(startGarbage, 3); 
		}
		int diffuseIndex = metadata.find("diffuse=");
		if( diffuseIndex != std::string::npos)
			metadata = metadata.erase(diffuseIndex, strlen("diffuse="));

		//jeśli pole metadanych nie jest puste
		int len = metadata.length();
		if(len>0) 
		{
			//dodawanie materiału do mapy materiałów 
			string fullName="";
			 
			 
			//pozyskiwanie tylko nazwy pliku
			int index = metadata.find_last_of("\\");

			if(index == string::npos) {
				fullName.append(metadata);
			} else {
				std::string fileNameOnly = metadata.substr(index+1, metadata.length());
				fullName.append(fileNameOnly);
			}
			//sprawdzanie czy taki materiał już istnieje,			 
			bool exists = true;
			if(materialNames.find(name)==materialNames.end() ) 
				 exists = false;
			 
			if(!exists)
				materialNames[name] = (fullName);
		
		} else {
			//w przeciwnym razie po prostu zapisz nazwę materiału jako pusty łańcuch
			bool exists = true;
			if(materialNames.find(name)==materialNames.end() ) 
				 exists = false;
			 
			if(!exists)
				materialNames[name] = "";
		
		}
		//pobierz następny węzeł materiału
		material = material.next_sibling("Material");
	}
	std::cerr<<" Done."<<std::endl;

	//wczytaj informacje siatkowe
	if ( meshImportLibrary )
	{
		ifstream infile(filename.c_str(), std::ios::in);
		//określ rozmiar pliku w bajtach
		long beg=0,end=0;
		infile.seekg(0, std::ios::beg); 
		beg = (long)infile.tellg();     
		infile.seekg(0, std::ios::end);
		end = (long)infile.tellg();
		infile.seekg(0, std::ios::beg); 
		long fileSize = (end - beg);

		//Utwórz bufor na tyle duży, aby pomieścił cały plik
		string buffer(std::istreambuf_iterator<char>(infile), (std::istreambuf_iterator<char>()));		 
		infile.close();

		msc = meshImportLibrary->createMeshSystemContainer(filename.c_str(),buffer.c_str(),buffer.length(),0);
		if ( msc ) {
			//pobierz system siatkowy i wyznacz prostopadłościan otaczający siatkę 
			NVSHARE::MeshSystem *ms = meshImportLibrary->getMeshSystem(msc);	
			min = glm::vec3(ms->mAABB.mMin[0], ms->mAABB.mMin[1], ms->mAABB.mMin[2]);
			max = glm::vec3(ms->mAABB.mMax[0], ms->mAABB.mMax[1], ms->mAABB.mMax[2]);
					
			//określ, czy górę siatki wskazuje oś Y, czy oś Z
			float dy = fabs(max.y-min.y);
			float dz = fabs(max.z-min.z);
			bool bYup = (dy>dz);
			if(!bYup) {
				float tmp =  min.y;
				min.y =  min.z;
				min.z = -tmp;

				tmp = max.y;
				max.y = max.z;
				max.z = -tmp;
			}

			//wczytaj szkielety
			std::cerr<<"Reading skeletons ...";
			if(ms->mSkeletonCount>0) {
				NVSHARE::MeshSkeleton* pSkel = ms->mSkeletons[0];
				Bone b;
				//dla każdej kości określ jej nazwę, orientację, skalę  
				//i położenie
				for(int i=0;i<pSkel->GetBoneCount();i++) {
					const NVSHARE::MeshBone pBone = pSkel->mBones[i];
					const int s = strlen(pBone.mName);
								
					b.name = new char[s+1];
					memset(b.name, 0, sizeof(char)*(s+1));
					strncpy_s(b.name,sizeof(char)*(s+1), pBone.mName, s);
					b.orientation = glm::quat( pBone.mOrientation[3],
											   pBone.mOrientation[0],
											   pBone.mOrientation[1],
											   pBone.mOrientation[2]);
					b.position = glm::vec3( pBone.mPosition[0],
												pBone.mPosition[1],
												pBone.mPosition[2]);

					b.scale  = glm::vec3(pBone.mScale[0], pBone.mScale[1], pBone.mScale[2]);

					//obsługa przypadku z osią Z w górę
					if(!bYup) {
						float tmp = b.position.y;
						b.position.y = b.position.z;
						b.position.z = -tmp;

						tmp = b.orientation.y;
						b.orientation.y = b.orientation.z;
						b.orientation.z = -tmp;

						tmp = b.scale.y;
						b.scale.y = b.scale.z;
						b.scale.z = -tmp;
					}			

					//wyznacz nową transformację lokalną
					glm::mat4 S = glm::scale(glm::mat4(1), b.scale);								
					glm::mat4 R = glm::toMat4(b.orientation);							
					glm::mat4 T = glm::translate(glm::mat4(1), b.position);												
					b.xform = T*R*S; 				
					
					//zapisz indeks kości nadrzędnej
					b.parent = pBone.mParentIndex;														

					//dodaj kość do wektora szkieletu
					skeleton.push_back(b);
				}
			}
			std::cerr<<" Gotowe. "<<std::endl;
			
			std::cerr<<"Wczytuję animacje ... ";
			//pobierz wszystkie animacje i umieść je w wektorze animacji
			for(size_t i=0;i<ms->mAnimationCount;i++) {
				NVSHARE::MeshAnimation* pAnim = (ms->mAnimations[i]);						 
				animations.push_back(*pAnim);
			}
			std::cerr<<" Gotowe. "<<std::endl;

			std::cerr<<"Wczytuje siatki i atrybuty wierzcholkow ...";
			int totalVertices = 0, lastSubMeshIndex = 0;

			//dla wszystkich siatek 
			for(size_t i=0;i<ms->mMeshCount;i++) {
				//pobierz siatkę
				NVSHARE::Mesh* pMesh = ms->mMeshes[i];
				 
				//dla każdego wierzchołka siatki pobierz jego położenie, normalną, współrzędne tekstury
				//wagi wiązania i indeksy wiązania, a następnie zapisz to wszystko w wektorze wierzchołków
				for(size_t j=0;j<pMesh->mVertexCount;j++) {
					Vertex v;
					v.pos.x = pMesh->mVertices[j].mPos[0];
					if(bYup) {
						v.pos.y = pMesh->mVertices[j].mPos[1];
						v.pos.z = pMesh->mVertices[j].mPos[2];
					} else {
						v.pos.y = pMesh->mVertices[j].mPos[2];
						v.pos.z = -pMesh->mVertices[j].mPos[1];
					}
							
					v.normal.x = pMesh->mVertices[j].mNormal[0];
					if(bYup) {
						v.normal.y = pMesh->mVertices[j].mNormal[1];
						v.normal.z = pMesh->mVertices[j].mNormal[2];
					} else {
						v.normal.y = pMesh->mVertices[j].mNormal[2];
						v.normal.z = -pMesh->mVertices[j].mNormal[1];
					}
					v.uv.x = pMesh->mVertices[j].mTexel1[0];
					v.uv.y = pMesh->mVertices[j].mTexel1[1];

					v.blendWeights.x = pMesh->mVertices[j].mWeight[0];
					v.blendWeights.y = pMesh->mVertices[j].mWeight[1];
					v.blendWeights.z = pMesh->mVertices[j].mWeight[2];
					v.blendWeights.w = pMesh->mVertices[j].mWeight[3];
					v.blendIndices[0] = pMesh->mVertices[j].mBone[0];
					v.blendIndices[1] = pMesh->mVertices[j].mBone[1];
					v.blendIndices[2] = pMesh->mVertices[j].mBone[2];
					v.blendIndices[3] = pMesh->mVertices[j].mBone[3];

					vertices.push_back(v);
				}

				//dla wszystkich siatek składowych
				for(size_t j=0;j<pMesh->mSubMeshCount;j++) {
					SubMesh s;
					NVSHARE::SubMesh* pSubMesh = pMesh->mSubMeshes[j];

					//pobierz nazwę materiału siatki składowej 					
					s.materialName = pSubMesh->mMaterialName;

					//pobierz indeksy siatki składowej
					s.indices.resize(pSubMesh->mTriCount * 3);
					memcpy(&(s.indices[0]), pSubMesh->mIndices, sizeof(unsigned int) *  pSubMesh->mTriCount * 3);				

					//zapisz siatkę składową w wektorze
					submeshes.push_back(s);
				}

				//dodawanie całkowitej liczby wierzchołków w celu przesunięcia indeksów siatki
				//aby wszystkie siatki mogły być połączone 
				//w jeden wektor
				if(totalVertices!=0) {
					for(size_t m=lastSubMeshIndex;m<submeshes.size();m++) {
						for(size_t n=0;n<submeshes[m].indices.size();n++) {
							submeshes[m].indices[n] += totalVertices;	
						}
					}
				}

				//powiększ ogólną liczbę wierzchołków i dołącz indeks siatki składowej 
				//do indeksów wszystkich siatek
				totalVertices += pMesh->mVertexCount;
				lastSubMeshIndex = submeshes.size();
			}
			std::cerr<<" Gotowe. "<<std::endl;
		}
	}
	return true;
}


