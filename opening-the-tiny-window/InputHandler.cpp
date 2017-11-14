#include "InputHandler.h"
#include "glUtils.h"
#include <iostream>

uint32_t InputHandler::smoothingWindowLength = 3;
int32_t InputHandler::width = 0;
int32_t InputHandler::height = 0;
GLFWInput InputHandler::input;

InputHandler::InputHandler() {}

InputHandler::InputHandler(GLFWwindow* _window)
{
  setWindow(window);
}

void InputHandler::setWindow(GLFWwindow* _window)
{
  window = _window;
  glfwGetWindowSize(window, &width, &height);
}

void InputHandler::setup()
{
  glfwSetKeyCallback(window, key_callback);
  glfwSetCursorPosCallback(window, cursorPos_callback);
  glfwSetMouseButtonCallback(window, mouseButton_callback);
  glfwSetWindowSizeCallback(window, windowSize_callback);
}

GLFWInput& InputHandler::getInput()
{
  return input;
}

void InputHandler::key_callback(GLFWwindow* window, int32_t key,
  int32_t scancode, int32_t action, int32_t mode)
{
  input.key = key;
  input.keyScancode = scancode;
  input.keyAction = action;
  input.keyMode = mode;

  if (action == GLFW_PRESS || action == GLFW_REPEAT) {
    if (key == GLFW_KEY_ESCAPE) {
      glfwSetWindowShouldClose(window, true);
    }
  }
}

void InputHandler::cursorPos_callback(GLFWwindow* window, double cursorX, double cursorY)
{
  input.cursorX = cursorX;
  input.cursorY = cursorY;

  int32_t width, height;
  glfwGetWindowSize(window, &width, &height);

  input.contextX = glUtils::normToContextPos(cursorX / width);
  input.contextY = glUtils::yNormToContextPos(cursorY / height);
}

void InputHandler::mouseButton_callback(GLFWwindow* window, int32_t button, int32_t action,
  int32_t mods)
{
  input.mouseButton = button;
  input.mouseAction = action;
  input.mouseMods = mods;
}

void InputHandler::windowSize_callback(GLFWwindow* window, int32_t _width, int32_t _height)
{
  width = _width;
  height = _height;
}
