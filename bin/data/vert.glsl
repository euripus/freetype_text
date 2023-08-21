attribute vec3 position;
attribute vec3 inNormal;
attribute vec2 tex;

varying vec3 normal;
varying vec2 texcoord;

void main()
{
	normal   = gl_NormalMatrix * inNormal;
	texcoord = tex.xy;
	
	gl_Position = gl_ModelViewProjectionMatrix * vec4(position, 1.0);
}