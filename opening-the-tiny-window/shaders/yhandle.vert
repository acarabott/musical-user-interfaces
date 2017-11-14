#version 330 core

in vec2 a_vertexPosition;
uniform float u_thickness;
uniform float u_offset;

void main() {
  gl_Position.xy = a_vertexPosition;
  gl_Position.x *= u_thickness;
  gl_Position.x += u_offset;
  gl_Position.z = 0.0;
  gl_Position.w = 1.0;
}