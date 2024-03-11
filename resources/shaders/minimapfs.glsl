#version 330 

out vec4 FragColor;
in vec2 fragTexCoord;

uniform sampler2D ourTexture;

void main()
{
//Unit Yellow color
    FragColor = texture(ourTexture, fragTexCoord) ;
    // FragColor = vec4(1.0f, 0.0f, 0.0f, 1.0f);
} 
