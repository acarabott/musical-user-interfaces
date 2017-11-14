#ifndef MUI_GL_UTILS
#define MUI_GL_UTILS

#include <vector>
#include <GLFW/glfw3.h>

namespace glUtils {
  void error_callback(int error, const char* description);

  bool setupGlfw(GLFWwindow*& window, uint32_t maxWidth, uint32_t maxHeight);

  // from http://www.opengl-tutorial.org/
  GLuint LoadShaders(const char* vertex_file_path, const char* fragment_file_path);

  float linlin(float val, float inMin, float inMax, float outMin=0.f,
               float outMax=1.f, bool clamp=true);

  float lingl (float val, float inMin, float inMax);

  float normToContextPos(float value);

  float yNormToContextPos(float normY);

  float contextPosToNorm(float contextPos);

  float clamp(float value, float min=0.0, float max=1.0);

  void clear(float r=0.0, float g=0.0, float b=0.0, float a=1.0);
}
#endif
