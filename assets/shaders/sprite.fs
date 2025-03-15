#version 330 core
out vec4 FragColor;

in vec3 color;
in vec2 uv;

uniform vec4 COLOR;
uniform float TIME;
uniform sampler2D texture1;
uniform sampler2D texture2;
uniform bool useTexture;
uniform bool isScrolling;

void main()
{
   if (isScrolling){
      FragColor = texture(texture2, uv + vec2((TIME*0.1f)*-1.0f, -1.0f*(TIME*0.1f)));
   }
   else{
      if (useTexture){
         FragColor = texture(texture1, uv);//COLOR;//vec4(vec3(cos(TIME)), 1.0f);
      } 
      else{
         FragColor = COLOR;
      }
   }

}