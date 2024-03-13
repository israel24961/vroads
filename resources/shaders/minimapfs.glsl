#version 330 

out vec4 FragColor;
in vec2 fragTexCoord;

uniform sampler2D mainTexture;

void main()
{
//Unit Yellow color

    FragColor = texture(mainTexture, fragTexCoord) ;
} 
