#version 330 core

in vec2 a_vertexPosition;

void main() {
  gl_Position.xy = a_vertexPosition;
  gl_Position.z = 0.0;
  gl_Position.w = 1.0;
}