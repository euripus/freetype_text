uniform sampler2D baseMap;

varying vec2 texcoord;

void main()
{
	gl_FragColor = texture2D( baseMap, texcoord );
}