#ifndef MUI_XHANDLE_H
#define MUI_XHANDLE_H

#include "GUI.h"
#include "glUtils.h"

namespace GUI
{
  class XHandle : public Widget
  {
    static const uint32_t numVerts = 4;
    static const uint32_t coordsPerVert = 2;
    static const uint32_t numCoords = numVerts * coordsPerVert;

    public:
      XHandle() {}
      XHandle(Widget& parent) {
        setRect(parent.getRect());
      }

      ~XHandle() {};

      void setup() {
        glGenVertexArrays(1, &vertexArrayID);
        glGenBuffers(1, &vertexBufferID);
        programID = glUtils::LoadShaders("shaders/xhandle.vert", "shaders/handle.frag");
        a_vertexPosition = glGetAttribLocation(programID, "a_vertexPosition");
        u_thickness = glGetUniformLocation(programID, "u_thickness");
        u_offset = glGetUniformLocation(programID, "u_offset");
        u_color = glGetUniformLocation(programID, "u_color");
      }

      float getValue() { return value; }
      void setValue(float _value) { value = fmin(1.f, fmax(0.f, _value)); }

      bool hasChanged() { return changed; }

      void update(GLFWInput& input) {
        bool mouseOver = false;
        bool mouseInContext = input.contextX >= -1.0 && input.contextX <= 1.0 &&
                              input.contextY >= -1.0 && input.contextY <= 1.0;

        if (mouseInContext)
        {
          float valueInContext = getValueAsPosition();
          mouseOver = input.contextX >= rect.left &&
                      input.contextX <= rect.right &&
                      input.contextY >= valueInContext - thickness * 2 &&
                      input.contextY <= valueInContext + thickness * 2;
        }

        if (input.mouseAction == 0) {
          grabbed = false;
        }
        else if (mouseOver && input.mouseButton == 0 && input.mouseAction == 1) {
          grabbed = true;
        }

        float prevValue = value;
        if (grabbed) {
          value = glUtils::clamp(getValueFromPosition(input.contextY));
        }
        changed = value != prevValue;

        thickness = mouseOver || grabbed ? maxThickness : minThickness;
        grabbed ? setToOnColor() : setToOffColor();
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
      bool changed = false;
      const float minThickness = 0.005;
      const float maxThickness = 0.015;
      float thickness = minThickness;
      const float onColor[4] = { 0.00, 0.69, 0.30, 0.9 };
      const float offColor[4] = { 1.0, 1.0, 1.0, 0.8 };
      float color[4] = {0.8, 0.0, 0.0, 0.8};
      float value = 1.0;
      float vertexBuffer[numCoords] = {
        -1.0,  1.0,
         1.0,  1.0,
         1.0, -1.0,
        -1.0, -1.0
      };

      float getValueAsPosition() {
        return (value * (rect.top - rect.bottom)) + rect.bottom;
      }

      float getValueFromPosition(float yposition) {
        return (yposition - rect.bottom) / (rect.top - rect.bottom);
      }

      float getXAsPosition(float value) {
        return (value * (rect.right - rect.left)) + rect.left;
      }

      void setToOnColor() {
        for (int i = 0; i < 4; ++i) { color[i] = onColor[i]; }
      }

      void setToOffColor() {
        for (int i = 0; i < 4; ++i) { color[i] = offColor[i]; }
      }
    };

}

#endif
