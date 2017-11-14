#ifndef MUI_INPUT_H
#define MUI_INPUT_H

#include <GLFW/glfw3.h>

struct GLFWInput
{
  int32_t key, keyScancode, keyAction, keyMode;
  double cursorX, cursorY;
  double contextX, contextY;
  int32_t mouseButton, mouseAction, mouseMods;
};

class InputHandler
{
public:
  InputHandler();
  InputHandler(GLFWwindow* _window);

  void setWindow(GLFWwindow* _window);

  void setup();

  GLFWInput& getInput();
  static const uint32_t getSmoothingWindowLength();

protected:
  GLFWwindow* window;
  static struct GLFWInput input;
  static uint32_t smoothingWindowLength;
  static int32_t width;
  static int32_t height;

  static void key_callback(GLFWwindow* window, int32_t key, int32_t scancode,
    int32_t action, int32_t mode);

  static void cursorPos_callback(GLFWwindow* window, double cursorX, double cursorY);

  static void mouseButton_callback(GLFWwindow* window, int32_t button, int32_t action,
    int32_t mods);

  static void windowSize_callback(GLFWwindow* window, int32_t _width, int32_t _height);

};


#endif
