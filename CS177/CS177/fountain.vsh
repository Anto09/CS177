#version 120

uniform vec3 accel;
uniform float t;
uniform float lifetime;

attribute float startTime;
attribute vec3 initialVelocity;
attribute vec4 color;

varying vec4 out_color;

void main() {
	float x = t - startTime;
	//the physics formula: f(t) = 0.5 at^2 + vt
	vec3 pos = x * (0.5 * x * accel + initialVelocity);
	gl_Position = vec4(pos.xy, pos.z/5, 1.0);
	out_color = color * (1 - x/lifetime);
	
	float v = (pos.z + 1.0)/2.0;
	gl_PointSize = (v * 1.0 + (1-v) * 20);
}