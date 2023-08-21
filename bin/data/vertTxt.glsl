attribute vec3 position;
attribute vec2 tex;

varying vec2 texcoord;
varying vec4 v_color;

void main()
{
    texcoord = tex.xy;
    v_color  = gl_Color.rgba;

    gl_Position = gl_ModelViewProjectionMatrix * vec4(position, 1.0);
}
