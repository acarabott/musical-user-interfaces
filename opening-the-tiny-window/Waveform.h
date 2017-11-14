#ifndef MUI_WAVEFORM
#define MUI_WAVEFORM

#define GLFW_INCLUDE_GLCOREARB
#include <iostream>
#include <GLFW/glfw3.h>
#include "GUI.h"
#include "glUtils.h"
#include "verts.h"
#include "audio.h"

enum class WaveformMode { Filled, JoinedLines, VerticalLines };

namespace GUI {

  class Waveform : public Widget
  {
  static const uint32_t audioBufferSize = MAX_WINDOW_WIDTH;
  static const uint32_t maxVertBufferSize = audioBufferSize * NumCoords::Max;

  public:
    Waveform() {}

    Waveform(GLFWwindow* window)
    {
      setWindow(window);
    }

    ~Waveform() {}

    void setWindow(GLFWwindow* _window)
    {
      window = _window;
      setupOpenGl();
    }

    void setAudio(float inBuffer[], uint32_t inBufferSize)
    {
      clearAudioBuffers();
      reduceAudioForVisualisation(
        BufferReduction::AbsMax,
        inBuffer, inBufferSize,
        audioBuffer, audioBufferSize
      );
      copyBuffer(audioBuffer, renderBuffer, audioBufferSize);
      createVerts();
    }

    void createVerts()
    {
      clearBuffer(verts, maxVertBufferSize);
      if (mode == WaveformMode::Filled) {
        createFilledVerticesFromAudioAbs(
          verts,
          numVerts,
          renderBuffer,
          audioBufferSize,
          rect.left,
          rect.right,
          rect.bottom,
          rect.top
        );
      }
      else if (mode == WaveformMode::JoinedLines) {
        createJoinedLineVerticesFromAudio(
          verts,
          numVerts,
          renderBuffer,
          audioBufferSize,
          rect.left,
          rect.right,
          rect.bottom,
          rect.top
        );
      }
      else if (mode == WaveformMode::VerticalLines) {
        createVerticalLineVerticesFromAudio(
          verts,
          numVerts,
          renderBuffer,
          audioBufferSize,
          rect.left,
          rect.right,
          rect.bottom,
          rect.top
        );
      }
    }

    float* getAudioBuffer() { return audioBuffer; }
    float* getRenderBuffer() { return renderBuffer; }
    uint32_t getAudioBufferSize() { return audioBufferSize; }

    virtual void draw() override
    {
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      glUseProgram(programID);
      glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
      glBufferData(GL_ARRAY_BUFFER, numVerts * sizeof(float), verts, GL_STATIC_DRAW);
      glBindVertexArray(vertexArrayID);
      glEnableVertexAttribArray(a_vertexPosition);
      glVertexAttribPointer(a_vertexPosition, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
      glUniform4f(u_inColor, r, g, b, a);
      glDrawArrays(drawArraysMode, 0, numVerts);
      glDisableVertexAttribArray(a_vertexPosition);
    }

    void setColor(float _r=1.0, float _g=1.0, float _b=1.0, float _a=1.0)
    {
      r = _r;
      g = _g;
      b = _b;
      a = _a;
    }

    void setMode(WaveformMode _mode)
    {
      mode = _mode;

      switch (mode) {
        case WaveformMode::Filled:
          drawArraysMode = GL_TRIANGLES;
          break;
        case WaveformMode::JoinedLines:
          drawArraysMode = GL_LINES;
          break;
        case WaveformMode::VerticalLines:
          drawArraysMode = GL_LINES;
          break;
        default:
            break;
      }
    }

    virtual void setRect(Rect rect) override
    {
      Widget::setRect(rect);
      createVerts();
    }

  protected:
    GLFWwindow* window;

    WaveformMode mode = WaveformMode::VerticalLines;

    float audioBuffer[audioBufferSize];
    float renderBuffer[audioBufferSize];

    GLuint programID;
    GLuint vertexArrayID;
    GLuint vertexBufferID;
    GLint a_vertexPosition;
    GLint u_inColor;
    GLenum drawArraysMode = GL_TRIANGLES;

    uint32_t numVerts = maxVertBufferSize;
    float verts[maxVertBufferSize];

    float r = 1.0;
    float g = 1.0;
    float b = 1.0;
    float a = 1.0;

    void setupOpenGl()
    {
      programID = glUtils::LoadShaders("shaders/waveform.vert", "shaders/waveform.frag");
      glGenVertexArrays(1, &vertexArrayID);
      glGenBuffers(1, &vertexBufferID);
      a_vertexPosition = glGetAttribLocation(programID, "a_vertexPosition");
      glEnableVertexAttribArray(a_vertexPosition);
      glVertexAttribPointer(a_vertexPosition, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
      u_inColor = glGetUniformLocation(programID, "inColor");
    }

    void clearAudioBuffers()
    {
      clearBuffer(audioBuffer, audioBufferSize);
      clearBuffer(renderBuffer, audioBufferSize);
    }

  };
}

#endif
