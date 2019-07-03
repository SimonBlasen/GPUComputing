varying vec3 norm;
varying vec4 viewPos;
varying vec4 diffuse;


void main()
{
	vec3 normal = normalize(norm);
	
	//the light is always where the camera is...
	vec3 light = -normalize(viewPos.xyz);

	float cosTheta = dot(normal, light);
	
	gl_FragColor = diffuse * cosTheta;
}