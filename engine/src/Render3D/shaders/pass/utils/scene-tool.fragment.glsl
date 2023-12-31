#version 330 core

/////////////////////////////////////////////////////////////
//////////////////////// VARYING ////////////////////////////
/////////////////////////////////////////////////////////////
flat in int v_id;
in vec3 v_color;

/////////////////////////////////////////////////////////////
/////////////////////////// OUT /////////////////////////////
/////////////////////////////////////////////////////////////
layout(location = 0) out vec4 o_fragColor;
layout(location = 1) out int o_id;

/////////////////////////////////////////////////////////////
////////////////////////// MAIN /////////////////////////////
/////////////////////////////////////////////////////////////
void main() {
    o_id = v_id;
    o_fragColor = vec4(v_color, 1.0);
}
