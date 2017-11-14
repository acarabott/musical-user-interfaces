#include "glUtils.h"

#include <fstream>
#include <vector>
#include <GLFW/glfw3.h>

namespace glUtils {

  void error_callback(int error, const char* description)
  {
    fprintf(stderr, "Error: %s\n", description);
  }

  bool setupGlfw(GLFWwindow*& window, uint32_t maxWidth, uint32_t maxHeight)
  {
    if (!glfwInit()) {
      return false;
    }

    glfwSetErrorCallback(error_callback);

    // window
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    int32_t width = maxWidth;
    int32_t height = maxHeight;

    window = glfwCreateWindow(width, height, "", NULL, NULL);

    if (!window) {
      glfwTerminate();
      return false;
    }

    glfwMakeContextCurrent(window);
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);
    glfwSwapInterval(1);

    return true;
  }

  // from http://www.opengl-tutorial.org/
  GLuint LoadShaders(const char* vertex_file_path, const char* fragment_file_path) {
    // Create the shaders
    GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

    // Read the Vertex Shader code from the file
    std::string VertexShaderCode;
    std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
    if(VertexShaderStream.is_open()){
      std::string Line = "";
      while(getline(VertexShaderStream, Line))
        VertexShaderCode += "\n" + Line;
      VertexShaderStream.close();
    }else{
      printf("Impossible to open %s. Are you in the right directory ? Don't forget to read the FAQ !\n", vertex_file_path);
      getchar();
      return 0;
    }

    // Read the Fragment Shader code from the file
    std::string FragmentShaderCode;
    std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
    if(FragmentShaderStream.is_open()){
      std::string Line = "";
      while(getline(FragmentShaderStream, Line))
        FragmentShaderCode += "\n" + Line;
      FragmentShaderStream.close();
    }

    GLint Result = GL_FALSE;
    int InfoLogLength;


    // Compile Vertex Shader
    printf("Compiling shader : %s\n", vertex_file_path);
    char const * VertexSourcePointer = VertexShaderCode.c_str();
    glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
    glCompileShader(VertexShaderID);

    // Check Vertex Shader
    glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if ( InfoLogLength > 0 ){
      std::vector<char> VertexShaderErrorMessage(InfoLogLength+1);
      glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL,
                         &VertexShaderErrorMessage[0]);
      printf("%s\n", &VertexShaderErrorMessage[0]);
    }



    // Compile Fragment Shader
    printf("Compiling shader : %s\n", fragment_file_path);
    char const * FragmentSourcePointer = FragmentShaderCode.c_str();
    glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
    glCompileShader(FragmentShaderID);

    // Check Fragment Shader
    glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if ( InfoLogLength > 0 ){
      std::vector<char> FragmentShaderErrorMessage(InfoLogLength+1);
      glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL,
                         &FragmentShaderErrorMessage[0]);
      printf("%s\n", &FragmentShaderErrorMessage[0]);
    }



    // Link the program
    printf("Linking program\n");
    GLuint ProgramID = glCreateProgram();
    glAttachShader(ProgramID, VertexShaderID);
    glAttachShader(ProgramID, FragmentShaderID);
    glLinkProgram(ProgramID);

    // Check the program
    glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
    glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if ( InfoLogLength > 0 ){
      std::vector<char> ProgramErrorMessage(InfoLogLength+1);
      glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
      printf("%s\n", &ProgramErrorMessage[0]);
    }


    glDetachShader(ProgramID, VertexShaderID);
    glDetachShader(ProgramID, FragmentShaderID);

    glDeleteShader(VertexShaderID);
    glDeleteShader(FragmentShaderID);

    return ProgramID;
  }

  float linlin(float val, float inMin, float inMax, float outMin, float outMax,
               bool clamp)
  {
    if (clamp) {
      if (val >= inMax) { return outMax; }
      if (val <= inMin) { return outMin; }
    }

    const auto inRange = inMax - inMin;
    const auto norm = val / inRange;
    const auto outRange = outMax - outMin;
    return outMin + (norm * outRange);
  }

  float lingl (float val, float inMin, float inMax) {
    return linlin(val, inMin, inMax, -1.f, 1.f);
  }

  float normToContextPos(float value) {
    return (value * 2) - 1.0;
  }

  float yNormToContextPos(float normY) {
    return normToContextPos(normY) * -1.0;
  }

  float contextPosToNorm(float contextPos) {
    return (contextPos + 1.0) / 2.0;
  }

  float clamp(float value, float min, float max) {
    if (value < min) { return min; }
    if (value > max) { return max; }
    return value;
  }

  void clear(float r, float g, float b, float a) {
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  }
}