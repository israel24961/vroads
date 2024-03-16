#version 330 

out vec4 FragColor;
in vec2 fragTexCoord;
in vec2 fragTexCoord2;


uniform sampler2D mainTexture;
uniform sampler2D topTexture;
uniform sampler2D topLeftTexture;
uniform sampler2D topRightTexture;

uniform sampler2D leftTexture;
uniform sampler2D rightTexture;


uniform sampler2D bottomTexture;
uniform sampler2D bottomLeftTexture;
uniform sampler2D bottomRightTexture;


void main()
{
FragColor = texture(mainTexture, fragTexCoord) * texture(topTexture, fragTexCoord2);
} 
