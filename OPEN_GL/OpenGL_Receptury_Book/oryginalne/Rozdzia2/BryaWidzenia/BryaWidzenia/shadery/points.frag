#version 330 core

layout(location = 0) out vec4 vFragColor;	//wyjœcie shadera fragmentów

void main()
{
	//jeœli wspó³rzêdne fragmentu s¹ wiêksze ni¿ promieñ kuli, 
	//fragment jest odrzucany.
	vec2 pos = (gl_PointCoord.xy-0.5);
	if(0.25<dot(pos,pos))
		discard;

	//w przeciwnym razie otrymuje kolor niebieski
	vFragColor = vec4(0,0,1,1);
}