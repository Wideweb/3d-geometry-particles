#version 330 core

/////////////////////////////////////////////////////////////
//////////////////////// ATTRIBUTES /////////////////////////
/////////////////////////////////////////////////////////////
layout(location = 1) in mat4 a_model;
layout(location = 5) in vec3 a_vertexPosition;
layout(location = 6) in vec3 a_vertexNormal;

/////////////////////////////////////////////////////////////
//////////////////////// UNIFORMS ///////////////////////////
/////////////////////////////////////////////////////////////
uniform mat4 u_view;
uniform mat4 u_projection;

/////////////////////////////////////////////////////////////
///////////////////////// VARYING ///////////////////////////
/////////////////////////////////////////////////////////////
out vec2 v_vertexPosition;
out vec3 v_fragWorldPosition;
out vec3 v_fragCameraPosition;
out vec3 v_normal;

/////////////////////////////////////////////////////////////
////////////////////////// MAIN /////////////////////////////
/////////////////////////////////////////////////////////////
void main() {
    vec4 fragWorldPosition = a_model * vec4(a_vertexPosition, 1.0);
    vec4 fragCameraPosition = u_view * fragWorldPosition;

    v_vertexPosition = a_vertexPosition.xz;
    v_fragWorldPosition = vec3(fragWorldPosition);
    v_fragCameraPosition = vec3(fragCameraPosition);
    v_normal = normalize(mat3(transpose(inverse(a_model))) * a_vertexNormal);
    
    gl_Position = u_projection * fragCameraPosition;
}