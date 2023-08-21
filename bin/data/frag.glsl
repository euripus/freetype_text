uniform sampler2D baseMap;

varying vec3 normal;
varying vec2 texcoord;

void main()
{
	vec3 n = normalize(normal);

	gl_FragColor = texture2D( baseMap, texcoord );
}