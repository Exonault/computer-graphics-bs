#version 330 core
struct Material {
    vec3 emission;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;    
	float ka; //ambient coefficient
	float kd; //diffuse coefficient
	float ks; //specular coefficient
    float shininess;
}; 

struct Light {
    vec3 position;
    vec3 diffuse; //the color of the light
};


/*struct Spotlight {
    vec3 position;  
    vec3 direction;

    float cutOff;
    float outerCutOff;
  
    vec3 diffuse;
};*/


vec3 getAmbient(Material);
vec3 getDiffuse(Material, Light);
vec3 getSpecular(Material, Light);


out vec4 FragColor;
uniform vec4 color; //Test 

in vec3 FragPos;  
in vec3 Normal;  
  
uniform vec3 viewPos;

uniform Material fragMaterial;

uniform Light ceilingLampLight;
uniform Light nightLampLight;

uniform bool ceilingLampStatus;
uniform bool nightLampStatus;

uniform vec3 nightLampLightDirection;
uniform float nightLampLightCutOff;
uniform float nightLampLightOuterCutOff;

void main()
{
    vec3 ambient = getAmbient(fragMaterial);
    
    vec3 diffuseCeilingLampLight = getDiffuse(fragMaterial,ceilingLampLight);
    vec3 specularCeilingLampLight = getSpecular(fragMaterial, ceilingLampLight);

    vec3 diffuseNightLampLight = getDiffuse(fragMaterial,nightLampLight);
    vec3 specularNightLampLight = getSpecular(fragMaterial, nightLampLight);

    if(!ceilingLampStatus)
    {
        diffuseCeilingLampLight = vec3(0.0f,0.0f,0.0f);
        specularCeilingLampLight = vec3(0.0f,0.0f,0.0f);
    }

    if(nightLampStatus)
    {
        vec3 lightDirection = normalize(nightLampLight.position - FragPos);
        float theta = dot(lightDirection, normalize(-nightLampLightDirection));
        float epsilon = (nightLampLightCutOff - nightLampLightOuterCutOff);
        float intensity = clamp((theta - nightLampLightOuterCutOff) / epsilon, 0.0, 1.0);
            
        diffuseNightLampLight *= intensity;
        specularNightLampLight *= intensity;
    }
    else
    {
        diffuseNightLampLight = vec3(0.0f,0.0f,0.0f);
        specularNightLampLight = vec3(0.0f,0.0f,0.0f);
    }
   
   vec3 result = fragMaterial.emission + ambient + 
   diffuseCeilingLampLight + specularCeilingLampLight +
   diffuseNightLampLight + specularNightLampLight;

   FragColor = vec4(result, 1.0f);

    //FragColor = vec4(ambient, 1.0f);
    //FragColor = vec4(result, 1.0f);
	//FragColor = color;
}


vec3 getAmbient(Material material)
{
    vec3 ambient = material.ka * material.ambient;
    return ambient;
}


vec3 getDiffuse(Material material, Light light)
{
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(light.position - FragPos);
    float diff = max(dot(norm, lightDir), 0.0); //cos to light direction
    vec3 diffuse = light.diffuse * material.kd * (diff * material.diffuse);

    return diffuse;
}


vec3 getSpecular(Material material, Light light)
{
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(light.position - FragPos);


    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
	//cos to the power of shininess of angle between light reflected ray and view direction
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess); 
    vec3 specular = material.ks * (spec * material.specular);  
    return specular;
}
