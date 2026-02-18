#version 330 core
layout (location = 0) in vec3 aPos; // the position variable has attribute position 0
layout (location = 1) in vec3 aNormal;

//uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
  
//out vec4 vertexColor; // specify a color output to the fragment shader
out vec3 Normal;
out vec3 FragPos;

void main()
{
    //gl_Position = vec4(aPos, 1.0); // see how we directly give a vec3 to vec4's constructor
    //vertexColor = vec4(0.5, 0.0, 0.0, 1.0); // set the output variable to a dark-red color
    FragPos = vec3(vec4(aPos, 1.0));
    Normal = aNormal;
    gl_Position = projection * view * vec4(FragPos, 1.0);
}
