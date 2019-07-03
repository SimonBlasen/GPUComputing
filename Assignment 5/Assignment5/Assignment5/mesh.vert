
varying vec3 norm;
varying vec4 viewPos;
varying vec4 diffuse;

void main()
{
	vec4 v = vec4(gl_Vertex);
	v.w = 1.0;

	viewPos = gl_ModelViewMatrix * v;
	
    gl_Position = gl_ModelViewProjectionMatrix * v;
    
    norm = gl_NormalMatrix * gl_Normal;
        
    diffuse = gl_Color;
}