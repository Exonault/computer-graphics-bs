#version 330 core
out vec4 FragColor;
uniform vec3 color;

void main()
{
	FragColor = vec4(color, 1.0);
}

/*#version 330 core
out vec4 FragColor;
 
in vec3 color;
in vec2 texCoords;
 
uniform float alpha;
uniform sampler2D diffuse;
uniform sampler2D diffuse2;
uniform sampler2D alphaMask;

void main()
{ 
	//FragColor = vec4(color.rgb, alpha); 
	//FragColor = vec4(texture(diffuse, texCoords).rgb,alpha);
	//FragColor = texture(diffuse, vec2(texCoords.s + alpha, texCoords.t));
	vec4 texel = texture(diffuse, texCoords);
	vec4 texel2 = texture(diffuse2, texCoords);
	vec4 mixedDiffuse =  mix(texel, texel2, 0.7);
	vec4 alphaTexel = texture(alphaMask, texCoords);
	//FragColor = vec4(mixedDiffuse.rgb, alphaTexel.a);
	if(alphaTexel.a < 0.1)
	{
		discard;
	}else 
	{
		FragColor = mixedDiffuse;
	}
	
}
	*/	