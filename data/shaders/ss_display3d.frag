#version 120

varying vec2 uv;

uniform bool leftFirst;
uniform sampler2D texScene1;
uniform sampler2D texScene2;

void main()
{
	vec2 p= vec2(floor(gl_FragCoord.x), floor(gl_FragCoord.y));
	if(leftFirst)
	{
		if (mod(p.y, 2.0)==0.0)
    		gl_FragColor = vec4( texture2D(texScene1, uv).bgr, 1.0 );
    	else
    		gl_FragColor = vec4( texture2D(texScene2, uv).bgr, 1.0 );
	}
	else
	{
		if (mod(p.y, 2.0)==0.0)
    		gl_FragColor = vec4( texture2D(texScene2, uv).bgr, 1.0 );
    	else
    		gl_FragColor = vec4( texture2D(texScene1, uv).bgr, 1.0 );
	}
    
}
