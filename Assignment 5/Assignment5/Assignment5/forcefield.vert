uniform sampler3D texForceField;

void main() {
	vec4 v = vec4(gl_Vertex);

  vec3 lookUp = v.xyz;	
	vec4 force = texture3D(texForceField, lookUp);
	
  gl_FrontColor = vec4(v.w, 1 - v.w, 0, 0.5);
  
  //gl_FrontColor *= length(force.xyz) * 0.01f; 

	if(v.w == 0.0)
	{
		v.w = 1.0;
		v.xyz += 0.0015 * force.xyz;
	}
		
  gl_Position = gl_ModelViewProjectionMatrix * v;
}
