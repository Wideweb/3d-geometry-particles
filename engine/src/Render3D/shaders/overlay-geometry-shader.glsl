#version 330

/////////////////////////////////////////////////////////////
///////////////////////// VARYING ///////////////////////////
/////////////////////////////////////////////////////////////
layout(triangles) in;
layout(line_strip, max_vertices = 4) out;

flat in int g_id[];
in vec3 g_color[];
in vec2 g_texCoord[];

flat out int v_id;
out vec3 v_color;
out vec2 v_texCoord;

/////////////////////////////////////////////////////////////
////////////////////////// MAIN /////////////////////////////
/////////////////////////////////////////////////////////////
void main() {
    int i;
    for (i = 0; i < gl_in.length(); i++) {
        v_id = g_id[i];
        v_color = g_color[i];
        v_texCoord = g_texCoord[i];
        gl_Position = gl_in[i].gl_Position;
        EmitVertex();
    }

    v_id = g_id[0];
    v_color = g_color[0];
    v_texCoord = g_texCoord[0];
    gl_Position = gl_in[0].gl_Position;
    EmitVertex();

    EndPrimitive();
}