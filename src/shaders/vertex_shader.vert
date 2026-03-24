#version 330 core
layout (location = 0) in vec3 aPos; // the position variable has attribute position 0
layout (location = 1) in vec3 aNormal;

uniform mat4 scale;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 rotation;

  
//out vec4 vertexColor; // specify a color output to the fragment shader
out vec3 Normal;
out vec3 FragPos;

void main()
{
    //gl_Position = vec4(aPos, 1.0); // see how we directly give a vec3 to vec4's constructor
    //vertexColor = vec4(0.5, 0.0, 0.0, 1.0); // set the output variable to a dark-red color
    FragPos = vec3(model * rotation * scale * vec4(aPos, 1.0));
    Normal = vec3(rotation * vec4(aNormal, 1.0));
    gl_Position = projection * view * model * rotation * scale * vec4(aPos, 1.0);
}
//
