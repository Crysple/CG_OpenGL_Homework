#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;
uniform sampler2D texture_diffuse1;
uniform vec3 lightColor;
uniform vec3 lightPos;
void main()
{
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
    
    float ambientStrength = 0.7;
    vec3 ambient = ambientStrength*lightColor;
    vec3 res = ambient+diffuse;
    FragColor = texture(texture_diffuse1, TexCoords)*vec4(res,1.0);
    //FragColor = vec4(0.3,0.3,0.5,1.0);
}
