#version 330 core
const float PI = 3.1415926538;
out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;
  
//in vec4 vertexColor; // the input variable from the vertex shader (same name and same type)  

uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 objectColor;
uniform bool lighting;

void main()
{
    //ambient
    vec3 result;
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;

    if(lighting == true){
        vec3 norm = normalize(Normal);
        vec3 lightDir = normalize(lightPos - FragPos);

        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diff * lightColor;

        result = (diffuse + ambient) * objectColor;
    }else{
        result = objectColor;
        vec3 norm = normalize(Normal);
        vec3 lightDir = normalize(lightPos - FragPos);
        float dot_prod = dot(norm, lightDir);
        float angle = acos(dot_prod);
        if (angle > (PI/2.0)){
           discard;
        }
    }
    FragColor = vec4(result, 1.0);
}
