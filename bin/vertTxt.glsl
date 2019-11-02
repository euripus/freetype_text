attribute vec3 position;
attribute vec2 tex;

varying vec2 texcoord;

void main()
{
	texcoord = tex.xy;
	
	gl_Position = gl_ModelViewProjectionMatrix * vec4(position, 1.0);
}