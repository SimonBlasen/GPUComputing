void main()
{
	//gl_FragColor = vec4(1.0f, 1.0f, 0.0f, 1.0f);
	
	//attenuation from texture coordinate
	float distance = length(vec2(gl_TexCoord[0].x - 0.5f, gl_TexCoord[0].y - 0.5f));

	//attenuation function
	float alpha = (0.5f - distance * distance / 0.5f) * 1.f;
	
	gl_FragColor = gl_Color;
	gl_FragColor.w = alpha;
	
	//gl_FragColor = vec4(gl_TexCoord[0].x, gl_TexCoord[0].y, 1.0f, alpha);
}