#version 330
#extension GL_ARB_explicit_uniform_location : enable

// Input vertex attributes
layout(location = 0) in vec3 vertexPosition;
layout(location = 1 ) in vec2 vertexTexCoord;

// Input uniform values
layout(location = 3) uniform float rotationRadians;

// Output vertex attributes (to fragment shader)
out vec2 fragTexCoord;

// NOTE: Add here your custom variables

vec2 rotate(vec2 v, float a) {
	float s = sin(a);
	float c = cos(a);
	mat2 m = mat2(c, s, -s, c);
	return m * v;
}

void main()
{
    // Send vertex attributes to fragment shader
    //fragTexCoord =  rotate(vertexTexCoord, rotationRadians);
    vec2 coord = vertexTexCoord;
    float sin_factor = sin(rotationRadians);
    float cos_factor = cos(rotationRadians);
    coord = (coord - 0.5) * mat2(cos_factor, sin_factor, -sin_factor, cos_factor);
    coord += 0.5;
    fragTexCoord = coord;

    gl_Position =  vec4(vertexPosition, 1.0);
}
