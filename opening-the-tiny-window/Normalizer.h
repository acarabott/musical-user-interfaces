#ifndef MUI_LEVELLER
#define MUI_LEVELLER

#include "Effect.h"
#include "audio.h"
#include <Accelerate/Accelerate.h>

class Normalizer : public Effect
{

public:
  void setLevel(float _level) { level = fmax(0.0, fmin(_level, 0.99)); }
  float getLevel() { return level; }

  void processAudio(float inBuffer[], float outBuffer[], uint32_t numFrames)
  {
    float mul = level / absMax;
    vDSP_vsmul(inBuffer, 1, &mul, outBuffer, 1, numFrames);
  }

  void setAbsMaxFromBuffer(float fullAudioBuffer[], uint32_t numFrames)
  {
    absMax = getBufferAbsMax(fullAudioBuffer, numFrames);
  }

  void setAbsMax(float _absMax) { absMax = _absMax; }

protected:
  float level = 1.0;
  float absMax = 0.0;
};
#endif