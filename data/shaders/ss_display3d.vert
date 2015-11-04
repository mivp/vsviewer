#version 120

attribute vec2 position;
attribute vec2 texcoor;

uniform mat4 tranMat;

varying vec2 uv;

void main()
{
  gl_Position = tranMat*vec4(position, 0.0f, 1.0f);
  uv = texcoor;
}
