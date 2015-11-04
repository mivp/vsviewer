#version 120

varying vec2 uv;

uniform sampler2D texScene;

void main()
{
	vec3 texel = texture2D(texScene, uv).bgr;
    gl_FragColor = vec4( texel, 1.0 );
}
