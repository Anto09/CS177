#version 120

uniform mat4 modelTransform;
attribute vec3 pos;
attribute vec4 color;

varying vec3 pos_out;
varying vec4 color_out;

void main() {
	pos_out = (modelTransform * vec4(pos,1)).xyz;
	gl_Position = vec4(pos_out,1);
	color_out = color;
}
