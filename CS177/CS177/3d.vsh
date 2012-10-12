#version 120

uniform mat4 mvp;
attribute vec3 pos;
attribute vec4 color;

varying vec3 pos_out;
varying vec4 color_out;

void main() {
	pos_out = (mvp * vec4(pos,1)).xyz;
	gl_Position = vec4(pos_out, 1);
	color_out = color;
}
