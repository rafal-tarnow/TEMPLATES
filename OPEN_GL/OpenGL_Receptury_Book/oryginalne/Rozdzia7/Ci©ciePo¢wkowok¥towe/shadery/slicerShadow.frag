#version 330 core

layout(location = 0) out vec4 vFragColor;	//wyjście shadera fragmentów

smooth in vec3 vUV;			//interpolowane współrzędne tekstury
smooth in vec4 vLightUVW;	//interpolowane współrzędne tekstury cienia

//uniformy
uniform sampler3D volume;		//sampler z danymi wolumetrycznymi
uniform sampler2D shadowTex;	//tekstura cienia
uniform vec4 color;				//kolor światła	

void main()
{  
	//pobierz natężenie światła z tekstury cienia
    vec3 lightIntensity =  textureProj(shadowTex, vLightUVW.xyw).xyz;
	
	//pobierz wartość wolumetryczną
	float density = texture(volume, vUV).r;

	//odrzuć małe wartości, aby uniknąć artefaktów
	if(density > 0.1) {
		
		//wyznacz wartość alfa na podstawie wartości wolumetrycznej
		float alpha = clamp(density, 0.0, 1.0);

		//pomnóż wartość alfa z tekstury przez wartość alfa z koloru światła
		alpha *= color.a;

		//zwróć ostateczny kolor frafmentu jako iloczyn koloru światła, 
		// jego natężenia i wartości alfa
		vFragColor = vec4(color.xyz*lightIntensity*alpha, alpha);
		
	} 
}