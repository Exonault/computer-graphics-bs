#version 330 core
struct Material {
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


out vec4 FragColor;
uniform vec4 color;

void main()
{
	FragColor = color;
}

/*
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
}*/
