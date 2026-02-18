#version 330 core
out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;
  
//in vec4 vertexColor; // the input variable from the vertex shader (same name and same type)  

uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 objectColor;

void main()
{
    //ambient
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;

    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);

    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    vec3 result = (diffuse + ambient) * objectColor;
    FragColor = vec4(result, 1.0);
}
