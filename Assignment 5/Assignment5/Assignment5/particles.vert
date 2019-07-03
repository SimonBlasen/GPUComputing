#extension GL_EXT_gpu_shader4 : enable
uniform samplerBuffer tboSampler;

vec4 colorCode(float value)
{
	value *= 0.2;

	vec4 retVal;
	retVal.x = 2.0 * value * mix(0.0, 1.0, max(0.0, 1.0 - value)); 
	retVal.y = 2.0 * value * min( mix(0.0, 1.0, max(0.0, value) / 0.5), mix(0.0, 1.0, max(0.0, (1.0 - value) / 0.5))); 
	retVal.z = 1.f;
	retVal.w = 0.4;
	
	return retVal;
}



void main() {
	vec4 v = vec4(gl_Vertex);
	 
	vec3 speed = texelFetchBuffer(tboSampler, gl_VertexID).xyz;		

	if (v.w < 1.f)
		gl_FrontColor = colorCode(v.w);
	else
		gl_FrontColor = vec4(0.3, 0.01, 0.01, 0.1);

	//gl_FrontColor = colorCode(length(speed));


	//if (v.w <= 0.f)
		//gl_FrontColor = vec4(1.0, 0.0, 0.0, 1.0);
	//else
		//gl_FrontColor = vec4(0.0, 1.0, 0.0, 1.0);
	//
	//the w coordinate is the life, if the particle is dead throw it outside the frustum
	if (v.w <= 0.f)
		v = vec4(1.0, 1.0, 1.0, 0.0);
	else
		v.w = 1.0;
    
	gl_Position = gl_ModelViewProjectionMatrix * v;
}
