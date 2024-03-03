#version 330
#extension GL_ARB_explicit_uniform_location : enable

// Input vertex attributes
layout(location = 0) in vec3 vertexPosition;
layout(location = 2) in vec3 nextVertexPosition;
layout(location = 3 ) in vec2 vertexTexCoord;
layout(location = 4) in vec4 vertexColor;
in vec3 vertexNormal;

// Input uniform values
layout(location = 1) uniform mat4 mvp = mat4(0.0f);

// Output vertex attributes (to fragment shader)
 out vec4 fragColor;
 out vec2 fragTexCoord;

// NOTE: Add here your custom variables

vec3 color(int i){
  float r = ((i >>  0) & 0xff)/255.0f;
  float g = ((i >>  8) & 0xff)/255.0f;
  float b = ((i >> 16) & 0xff)/255.0f;
  return vec3(r,g,b);
}


void main()
{
    // Send vertex attributes to fragment shader
    fragTexCoord = vertexTexCoord;
    fragColor = vertexColor;

    //Add some noise to the vertex position
    vec3 newPos=vertexPosition ;
    // Calculate final vertex position
    if (mvp[0][0]!=0.0f){
        gl_Position = mvp*vec4(newPos,1.0f);
    }
    else{
        gl_Position = vec4(newPos,1.0f);
    }
}
