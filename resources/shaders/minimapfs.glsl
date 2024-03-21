#version 330 

out vec4 FragColor;
//Coods
in vec2 fragTexTopLeftCoord;
in vec2 fragTexTopCenterCoord;
in vec2 fragTexTopRightCoord;
in vec2 fragTexLeftCoord;
in vec2 fragTexCenterCoord;
in vec2 fragTexRightCoord;
in vec2 fragTexBottomLeftCoord;
in vec2 fragTexBottomCenterCoord;
in vec2 fragTexBottomRightCoord;



//Textures
uniform sampler2D topLeftTexture;
uniform sampler2D topCenterTexture;
uniform sampler2D topRightTexture;

uniform sampler2D leftTexture;
uniform sampler2D mainTexture;
uniform sampler2D rightTexture;


uniform sampler2D bottomLeftTexture;
uniform sampler2D bottomTexture;
uniform sampler2D bottomRightTexture;


void main()
{
    //FragColor = texture(mainTexture, fragTexCoord) * texture(topTexture, fragTexCoord2);
    vec4 topCoords= texture(topLeftTexture, fragTexTopLeftCoord) * 
                    texture(topCenterTexture, fragTexTopCenterCoord) * 
                    texture(topRightTexture, fragTexTopRightCoord);
    vec4  MiddleCoords=  texture(leftTexture, fragTexLeftCoord) * 
                        texture(mainTexture, fragTexCenterCoord) * 
                        texture(rightTexture, fragTexRightCoord);
    vec4 bottomCoords= texture(bottomLeftTexture, fragTexBottomLeftCoord) *
                       texture(bottomTexture, fragTexBottomCenterCoord) *
                       texture(bottomRightTexture, fragTexBottomRightCoord);

    FragColor = topCoords * MiddleCoords * bottomCoords;

} 
