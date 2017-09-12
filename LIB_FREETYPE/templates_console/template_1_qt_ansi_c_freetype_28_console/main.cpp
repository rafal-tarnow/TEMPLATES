#include <iostream>
using namespace std;

#include <ft2build.h>
#include FT_FREETYPE_H

#include <bitset>

void drawGlyphToConsole(FT_Face &face){
    for(unsigned int i = 0; i < face->glyph->bitmap.rows; i++){
        for(unsigned int j = 0; j < face->glyph->bitmap.width; j++){

            int alpha_value =   (int)(face->glyph->bitmap.buffer[i*face->glyph->bitmap.width + j]) / 26;

            if(alpha_value > 0){
                cout << alpha_value;
            }else{
                cout << " ";
            }


        }
        cout << endl;
    }
}

int main()
{
    FT_Library ft;

    if (FT_Init_FreeType(&ft))
        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;

    FT_Face face;
    if (FT_New_Face(ft, "./data/font/dahot_Garfield_www_myfontfree_com.ttf", 0, &face))
        std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;

    FT_Set_Pixel_Sizes(face, 0, 32);

    if (FT_Load_Char(face, 'W', FT_LOAD_RENDER /*| FT_LOAD_TARGET_MONO*/))
        std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;

    cout << "width = " << face->glyph->metrics.width << endl;
    cout << "height = " << face->glyph->metrics.height << endl;

    cout << "bitmap.width = " << face->glyph->bitmap.width << endl;
    cout << "bitmap.rows = " << face->glyph->bitmap.rows << endl;
    cout << "bitmap.pitch = " << face->glyph->bitmap.pitch << endl;



    drawGlyphToConsole(face);





    return 0;
}



