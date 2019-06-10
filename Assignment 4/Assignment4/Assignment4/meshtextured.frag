varying vec3 norm;
varying vec4 viewPos;
varying vec2 tex;

uniform sampler2D texDiffuse;

void main()
{
	vec3 normal = normalize(norm);
	
	//the light is always where the camera is...
	vec3 light = -normalize(viewPos.xyz);

	float cosTheta = dot(normal, light);
	
	vec4 surfColor = vec4(tex.x, tex.y, 0.5, 1.0);
	
	surfColor = texture( texDiffuse, tex.xy );
	
	gl_FragColor = surfColor * cosTheta;
}