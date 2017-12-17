#version 330 core
out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;

uniform vec3 lightPosR;
uniform vec3 lightColorR;
uniform vec3 lightPosG;
uniform vec3 lightColorG;
uniform vec3 lightPosB;
uniform vec3 lightColorB;
uniform vec3 viewPos;
uniform vec3 objectColor;
void ProcessLight(in int color, in bool isdiffuse, in bool isspecular, out vec3 result){
    vec3 lightColor = vec3(0,0,0);
    vec3 lightPos = vec3(0,0,0);
    switch(color){
        case 0:{
            lightColor = lightPosR;
            lightPos = lightPosR;
            break;
        }
        case 1:{
            lightColor = lightPosG;
            lightPos = lightPosG;
            break;
        }
        case 2:{
            lightColor = lightPosB;
            lightPos = lightPosB;
            break;
        }
    }
    
    
    // diffuse
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
    
    // specular
    float specularStrength = 0.6;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;
    
    if(!isdiffuse) diffuse=vec3(0,0,0);
    if(!isspecular) specular=vec3(0,0,0);
    result = diffuse + specular;
}
void main()
{
    vec3 r0,r1,r2;
    bool diffuse = true;
    bool specular = true;
    // ambient
    float ambientStrength = 0.2;
    vec3 ambient = ambientStrength * vec3(1.0f,1.0f,1.0f);
    
    ProcessLight(0,diffuse,specular,r0);
    ProcessLight(1,diffuse,specular,r1);
    ProcessLight(2,diffuse,specular,r2);
    vec3 result = (r0+r1+r2+ambient) * objectColor;
    FragColor = vec4(result, 1.0);
}
