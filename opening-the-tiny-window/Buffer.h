#ifndef MUI_BUFFER
#define MUI_BUFFER

#include "audio.h"

class Buffer
{
static const uint32_t maxAudioBufferSize = 44100 * 60 * 10;
static const uint32_t maxUIAudioBufferSize = MAX_WINDOW_WIDTH;
public:
  Buffer();

  Buffer(std::string path)
  {
    loadAudio(path);
  }
  ~Buffer()
  {
    delete[] audioBuffer;
    delete[] uiBuffer;
  }

  void createUIBuffer()
  {
    reduceAudioForVisualisation(BufferReduction::AbsMax,
      audioBuffer, numFrames, uiBuffer, uiBufferSize);
  }

  void setRawAudio(float _audioBuffer[], uint32_t _numFrames)
  {
    if (numFrames > maxAudioBufferSize) {
      std::cout << "audio too big for the effect buffer!" << std::endl;
      return;
    }
    audioBuffer = _audioBuffer;
    numFrames = _numFrames;
  }

  void loadAudio(std::string path)
  {
    clearBuffers();
    numFrames = readSoundFileIntoBuffer(path, audioBuffer, numFrames);
    createUIBuffer();
  }

  float* getAudio() { return audioBuffer; }

  float* getOffsetAudio(uint32_t startFrame) { return &audioBuffer[startFrame]; }

  float* getUIAudio() { return uiBuffer; }

  const uint32_t getNumFrames() { return numFrames; }

  const uint32_t getUIAudioNumFrames() { return uiBufferSize; }

protected:
  float* audioBuffer = new float[maxAudioBufferSize];
  uint32_t numFrames = maxAudioBufferSize;

  float* uiBuffer = new float[maxUIAudioBufferSize];
  uint32_t uiBufferSize = maxUIAudioBufferSize;


  void clearBuffers()
  {
    clearBuffer(audioBuffer, maxAudioBufferSize);
    numFrames = maxAudioBufferSize;
    clearBuffer(uiBuffer, maxUIAudioBufferSize);
    uiBufferSize = maxUIAudioBufferSize;
  }
};
#endif