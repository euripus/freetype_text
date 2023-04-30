uniform sampler2D baseMap;

varying vec2 texcoord;
varying vec4 v_color;

void main()
{
    gl_FragColor = v_color * texture2D( baseMap, texcoord );
}
