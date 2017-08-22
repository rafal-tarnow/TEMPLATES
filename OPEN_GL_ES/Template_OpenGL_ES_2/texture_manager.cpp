#include "texture_manager.hpp"

#include <SOIL/SOIL.h>

map<string, GLuint> TextureManager::mapaTesktur;

GLuint TextureManager::getTextureId(string fileName){
    if(mapaTesktur.count(fileName) == 1){
        //jeżeli istnieje juz taka teksuta to ją zwróc
        return mapaTesktur.at(fileName);
    }else{
        //w przeciwnym wypadku utworz nowa teksture
        mapaTesktur[fileName] = SOIL_load_OGL_texture(fileName.c_str(), 4,0,SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y);

        glBindTexture(GL_TEXTURE_2D, mapaTesktur.at(fileName));

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // Set wrapping mode
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glBindTexture(GL_TEXTURE_2D, 0);

        return mapaTesktur.at(fileName);
    }
    return 0;
}


void TextureManager::deleteAllTextures(){
    //TODO zrobic usuwanie tekstur
      //glDeleteTextures(1)
}
