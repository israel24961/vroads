#version 330 

out vec4 FragColor;
in vec2 fragTexCoord;
in vec4 fragColor;

uniform bool hasTexture=true;
uniform sampler2D ourTexture;
uniform sampler2D ourTexture2;

void main()
{
//Unit white color
    FragColor = texture(ourTexture, fragTexCoord) ;
} 
