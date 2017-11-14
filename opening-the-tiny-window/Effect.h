#ifndef MUI_EFFECT
#define MUI_EFFECT

#include "audio.h"

class Effect
{
public:
  Effect() {};
  ~Effect(){}

  virtual void processAudio(float inBuffer[], float outBuffer[],
    uint32_t numFrames) {};

  void processAudio(float buffer[], uint32_t numFrames) {
    return processAudio(buffer, buffer, numFrames);
  }

protected:
};
#endif