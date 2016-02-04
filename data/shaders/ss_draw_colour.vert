#version 120

attribute vec2 position;

void main()
{
	vec2 pos = (position - 0.5)*2;
	pos.y = -pos.y;
  	gl_Position = vec4(pos, 0.0f, 1.0f);
}
