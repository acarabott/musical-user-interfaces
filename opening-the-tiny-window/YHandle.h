#ifndef MUI_YHANDLE_H
#define MUI_YHANDLE_H

#include "GUI.h"
#include "glUtils.h"

namespace GUI
{
  class YHandle : public Widget
  {
    static const uint32_t numVerts = 4;
    static const uint32_t coordsPerVert = 2;
    static const uint32_t numCoords = numVerts * coordsPerVert;

    public:
      YHandle() {}
      YHandle(Widget& parent) {
        setRect(parent.getRect());
      }

      ~YHandle() {};

      void setup() {
        glGenVertexArrays(1, &vertexArrayID);
        glGenBuffers(1, &vertexBufferID);
        programID = glUtils::LoadShaders("shaders/yhandle.vert", "shaders/handle.frag");
        a_vertexPosition = glGetAttribLocation(programID, "a_vertexPosition");
        u_thickness = glGetUniformLocation(programID, "u_thickness");
        u_offset = glGetUniformLocation(programID, "u_offset");
        u_color = glGetUniformLocation(programID, "u_color");
      }

      float getValue() { return value; }
      void setValue(float _value) { value = fmin(1.f, fmax(0.f, _value)); }

      bool wasReleased() { return released; }

      bool hasChanged() { return changed; }

      void update(GLFWInput& input) {
        bool mouseOver = false;
        bool mouseInContext = input.contextX >= -1.0 && input.contextX <= 1.0 &&
                              input.contextY >= -1.0 && input.contextY <= 1.0;

        if (mouseInContext)
        {
          float valueInContext = getValueAsPosition();
          mouseOver = input.contextX >= valueInContext - thickness * 2 &&
                      input.contextX <= valueInContext + thickness * 2 &&
                      input.contextY >= rect.bottom &&
                      input.contextY <= rect.top;
        }

        released = false;
        if (input.mouseAction == 0) {
          if (grabbed) {
            released = true;
          }
          grabbed = false;
        }
        else if (mouseOver && input.mouseButton == 0 && input.mouseAction == 1) {
          grabbed = true;
        }

        float prevValue = value;
        if (grabbed) {
          value = glUtils::clamp(getValueFromPosition(input.contextX));
        }
        changed = value != prevValue;

        thickness = mouseOver || grabbed ? maxThickness : minThickness;
        color[0] = color[1] = color[2] = grabbed ? 1.0 : 0.8;
        color[3] = grabbed ? 0.8 : 0.5;
      }

      virtual void draw() override {
        glUseProgram(programID);
        // buffer
        glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
        glBufferData(GL_ARRAY_BUFFER, numCoords * sizeof(float), vertexBuffer,
          GL_STATIC_DRAW);
        // array
        glBindVertexArray(vertexArrayID);
        glEnableVertexAttribArray(a_vertexPosition);
        glVertexAttribPointer(a_vertexPosition, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
        // thickness
        glUniform1f(u_thickness, thickness);
        glUniform1f(u_offset, getValueAsPosition());
        // draw signal
        glUniform4f(u_color, color[0], color[1], color[2], color[3]);

        // glEnable(GL_LINE_SMOOTH);
        glDrawArrays(GL_TRIANGLE_FAN, 0, numVerts);

        // tear down
        glDisableVertexAttribArray(a_vertexPosition);
        glDisableVertexAttribArray(u_thickness);
        // glDisable(GL_LINE_SMOOTH);
      }

      virtual void setRect(Rect rect) override
      {
        Widget::setRect(rect);

        vertexBuffer[0] = rect.left;
        vertexBuffer[1] = rect.top;

        vertexBuffer[2] = rect.right;
        vertexBuffer[3] = rect.top;

        vertexBuffer[4] = rect.right;
        vertexBuffer[5] = rect.bottom;

        vertexBuffer[6] = rect.left;
        vertexBuffer[7] = rect.bottom;
      }

    protected:
      GLuint vertexArrayID;
      GLuint vertexBufferID;
      GLint a_vertexPosition;
      GLint u_thickness;
      GLint u_offset;
      GLint u_color;
      GLuint programID;
      bool released;
      bool changed = false;
      const float minThickness = 0.0025;
      const float maxThickness = 0.0050;
      float thickness = minThickness;
      float color[4] = {0.8, 0.8, 0.8, 0.5};
      float value = 1.0;
      float vertexBuffer[numCoords] = {
        -1.0,  1.0,
         1.0,  1.0,
         1.0, -1.0,
        -1.0, -1.0
      };

      float getValueAsPosition() {
        return (value * (rect.right - rect.left)) + rect.left;
      }

      float getValueFromPosition(float xposition) {
        return (xposition - rect.left) / (rect.right - rect.left);
      }
    };

}

#endif
