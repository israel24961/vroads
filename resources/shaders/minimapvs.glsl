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

layout(location = 6) uniform v2  centerTextureOffset;

// Output vertex attributes (to fragment shader)
out vec2 fragTexTopLeftCoord;
out vec2 fragTexTopCenterCoord;
out vec2 fragTexTopRightCoord;
out vec2 fragTexLeftCoord;
out vec2 fragTexCenterCoord;
out vec2 fragTexRightCoord;
out vec2 fragTexBottomLeftCoord;
out vec2 fragTexBottomCenterCoord;
out vec2 fragTexBottomRightCoord;

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
    fragTexTopLeftCoord = ApplyCenter( vertexTexCoord, centerTextureOffset + v2(1, 1));
    fragTexTopCenterCoord = ApplyCenter( vertexTexCoord, centerTextureOffset + v2(1, 0));
    fragTexTopRightCoord = ApplyCenter( vertexTexCoord, centerTextureOffset + v2(1,-1));
    fragTexLeftCoord = ApplyCenter( vertexTexCoord, centerTextureOffset + v2(0, 1));
    fragTexCenterCoord = ApplyCenter( vertexTexCoord, centerTextureOffset + v2(0, 0));
    fragTexRightCoord = ApplyCenter( vertexTexCoord, centerTextureOffset + v2(0, -1));
    fragTexBottomLeftCoord = ApplyCenter( vertexTexCoord, centerTextureOffset + v2(-1, 1));
    fragTexBottomCenterCoord = ApplyCenter( vertexTexCoord, centerTextureOffset + v2(-1, 0));
    fragTexBottomRightCoord = ApplyCenter( vertexTexCoord, centerTextureOffset + v2(-1,-1));


    // fragTexCoord =  vertexTexCoord;


    gl_Position =  rotate*scale*vec4(vertexPosition, 1.0)+vec4(translation, 0.0, 0.0);
}
