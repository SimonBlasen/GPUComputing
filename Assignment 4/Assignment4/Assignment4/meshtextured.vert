
varying vec3 norm;
varying vec4 viewPos;
varying vec2 tex;

void main()
{
	vec4 v = vec4(gl_Vertex);
	v.w = 1.0;

	viewPos = gl_ModelViewMatrix * v;
	
    gl_Position = gl_ModelViewProjectionMatrix * v;
    
    norm = gl_NormalMatrix * gl_Normal;
    
    //two-sided rendering: if the normal faces backwards in view-space
    //flip it
    if(dot(viewPos.xyz, norm.xyz) > 0)
    	norm *= -1.0;

    tex = gl_MultiTexCoord0.xy;
    
}