#version 330
#extension GL_ARB_explicit_uniform_location : enable

#define v2 vec2
#define v3 vec3

// Input vertex attributes
layout(location = 0) in vec3 vertexPosition;
layout(location = 1 ) in vec2 vertexTexCoord;

// Input uniform values
layout(location = 3) uniform mat4 rotate;
layout(location = 4) uniform mat4 scale;
layout(location = 5) uniform v2 translation;

layout(location = 6) uniform v2  centerTexture;

// Output vertex attributes (to fragment shader)
out vec2 fragTexCoord;
out vec2 fragTexCoord2;

v2 ApplyCenter(v2 fragCord, v2 marker)
{
    v2 center=v2(0.5, 0.5);
    v2 ilotilat= v2(marker.y, marker.x);
    v2 S =  ilotilat - center;
    v2 T= S; // The texture has to be translated in the opposite direction
    return fragCord + T;
}

void main()
{
    // Send vertex attributes to fragment shader
    //fragTexCoord =  rotate(vertexTexCoord, rotationRadians);

    // v2 rotatedVxCoord = rotate(vertexTexCoord, frontVector);
    fragTexCoord =  ApplyCenter( vertexTexCoord, centerTexture);
    fragTexCoord2 = ApplyCenter( vertexTexCoord, centerTexture + v2(1, 0));
    // fragTexCoord =  vertexTexCoord;


    gl_Position =  rotate*scale*vec4(vertexPosition, 1.0)+vec4(translation, 0.0, 0.0);
}
