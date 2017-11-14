#define MAX_WINDOW_WIDTH 1800
#define MAX_WINDOW_HEIGHT 600

#include <iostream>
#define GLFW_INCLUDE_GLCOREARB
#include <GLFW/glfw3.h>
#include <thread>

#include "glUtils.h"
#include "InputHandler.h"
#include "AudioDevice.h"
#include "audio.h"
#include "Buffer.h"
#include "Waveform.h"
#include "XHandle.h"
#include "YHandle.h"
#include "Clipper.h"
#include "Normalizer.h"

GLFWwindow* window = NULL;
InputHandler inputHandler;
int32_t prevKeyAction = -1;

Buffer buffer("audio/prototype.wav");
AudioDevice audioDevice(buffer);

bool drawOriginal = false;
GUI::Waveform originalWaveform;

Clipper clipper;
GUI::Waveform clipWaveform;
GUI::XHandle clipHandle;

float bufferAbsMax = getBufferAbsMax(buffer.getAudio(), buffer.getNumFrames());
Normalizer normalizer;
GUI::Waveform normWaveform;
GUI::XHandle normHandle;

GUI::YHandle playHead;

std::thread audioThread([]{ audioDevice.run(); });

void processWaveform(GUI::Waveform& waveform, Effect& effect)
{
  effect.processAudio(waveform.getAudioBuffer(), waveform.getRenderBuffer(),
    waveform.getAudioBufferSize());
  waveform.createVerts();
}

void updateNormalizerFromClipper()
{
  normalizer.setAbsMax(fmin(bufferAbsMax, clipper.getThresh()));
}

void setup()
{
  inputHandler.setWindow(window);
  inputHandler.setup();

  // audioDevice.enableLooping();
  GUI::Rect frame(0.975, 0.99, -0.975, -0.99);
  playHead.setRect(frame);

  // original waveform
  originalWaveform.setMode(WaveformMode::Filled);
  originalWaveform.setWindow(window);
  originalWaveform.setRect(GUI::Rect(frame.top, frame.right, 0.025, frame.left));
  originalWaveform.setAudio(buffer.getAudio(), buffer.getNumFrames());
  originalWaveform.setColor(1.0, 1.0, 1.0, 0.3);

  // clipper
  audioDevice.addEffect(&clipper);

  clipWaveform.setMode(WaveformMode::Filled);

  clipWaveform.setWindow(window);
  GUI::Rect clipRect(frame.top, frame.right, 0.00, frame.left);
  clipWaveform.setRect(clipRect);
  clipWaveform.setAudio(buffer.getAudio(), buffer.getNumFrames());
  clipWaveform.setColor(1.0, 1.0, 1.0, 1.0);
  processWaveform(clipWaveform, clipper);
  clipHandle.setRect(clipRect);
  clipHandle.setup();
  clipHandle.setValue(clipper.getThresh());

  // normalizer
  updateNormalizerFromClipper();
  normalizer.setLevel(bufferAbsMax);
  audioDevice.addEffect(&normalizer);

  normWaveform.setMode(WaveformMode::Filled);
  normWaveform.setWindow(window);
  GUI::Rect normRect(-0.025, frame.right, frame.bottom, frame.left);
  normWaveform.setRect(normRect);
  normWaveform.setColor(0.17, 0.61, 0.83, 1.0);
  normWaveform.setAudio(clipWaveform.getRenderBuffer(),
  clipWaveform.getAudioBufferSize());
  processWaveform(normWaveform, normalizer);
  normHandle.setRect(normRect);
  normHandle.setup();
  normHandle.setValue(normalizer.getLevel());

  playHead.setup();
  playHead.setValue(0.0);
}

void update()
{
  GLFWInput& input = inputHandler.getInput();

  bool anyGrabbed = clipHandle.isGrabbed()
                 || normHandle.isGrabbed()
                 || playHead.isGrabbed();

  if (!anyGrabbed || clipHandle.isGrabbed()) clipHandle.update(input);
  if (clipHandle.hasChanged()) {
    clipper.setThresh(clipHandle.getValue());
    processWaveform(clipWaveform, clipper);

    updateNormalizerFromClipper();
    normWaveform.setAudio(clipWaveform.getRenderBuffer(),
      clipWaveform.getAudioBufferSize());
    processWaveform(normWaveform, normalizer);
  }

  if (!anyGrabbed || normHandle.isGrabbed()) normHandle.update(input);
  if (normHandle.hasChanged()) {
    normalizer.setLevel(normHandle.getValue());
    processWaveform(normWaveform, normalizer);
  }

  if (prevKeyAction != input.keyAction && input.keyAction == GLFW_PRESS) {
    if (input.key == GLFW_KEY_SPACE) {
      audioDevice.togglePlayback();
    }
    else if (input.key == GLFW_KEY_ENTER) {
      drawOriginal = !drawOriginal;
    }
  }
  prevKeyAction = input.keyAction;

  if (audioDevice.isStopped() && (!anyGrabbed || playHead.isGrabbed())) {
    playHead.update(input);
  }

  if (playHead.isGrabbed()) {
    audioDevice.stop();
    audioDevice.setPlayheadPosition(playHead.getValue());
  }
  else {
    playHead.setValue(audioDevice.getPlayheadPosition());
  }
};

void draw()
{
  if (drawOriginal) originalWaveform.draw();

  clipWaveform.draw();
  clipHandle.draw();

  normWaveform.draw();
  normHandle.draw();

  playHead.draw();
}

void cleanup() {
  audioDevice.kill();
  audioThread.join();
}

int main(int argc, char const *argv[])
{
  if (!glUtils::setupGlfw(window, MAX_WINDOW_WIDTH, MAX_WINDOW_HEIGHT)) {
    return -1;
  }
  setup();

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();
    update();

    glUtils::clear(0.05, 0.05, 0.05, 1.0);

    draw();

    glfwSwapBuffers(window);
  }
  glfwTerminate();

  cleanup();
  return 0;
}
