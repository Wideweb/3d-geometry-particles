/////////////////////////////////////////////////////////////
/////////////////////// SSAO BEGIN //////////////////////////
/////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////
/////////////////////// DECLARATION /////////////////////////
/////////////////////////////////////////////////////////////
float ssao();

/////////////////////////////////////////////////////////////
//////////////////////// UNIFORMS ///////////////////////////
/////////////////////////////////////////////////////////////
uniform sampler2D u_ssao;

/////////////////////////////////////////////////////////////
///////////////////////// VARYING ///////////////////////////
/////////////////////////////////////////////////////////////
in vec2 v_fragScreenPos;

/////////////////////////////////////////////////////////////
////////////////////////// MAIN /////////////////////////////
/////////////////////////////////////////////////////////////
float ssao() {

#ifdef DEFERRED
    return texture(u_ssao, v_texCoord).r;
#else
    return texture(u_ssao, v_fragScreenPos * 0.5 + 0.5).r;
#endif

}

/////////////////////////////////////////////////////////////
///////////////////////// SSAO END //////////////////////////
/////////////////////////////////////////////////////////////
