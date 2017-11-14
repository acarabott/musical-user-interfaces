#ifndef MUI_CLIPPER
#define MUI_CLIPPER

#include "Effect.h"
#include <Accelerate/Accelerate.h>

class Clipper : public Effect
{

public:

  void setThresh(float _thresh)
  {
    thresh = fmax(0.0, fmin(_thresh, 1.0));
  }

  float getThresh() { return thresh; }

  virtual void processAudio(float inBuffer[], float outBuffer[], uint32_t numFrames)
  override
  {
    float lowThresh = -thresh;
    vDSP_vclip(inBuffer, 1, &lowThresh, &thresh, outBuffer, 1, numFrames);
  }
protected:
  float thresh = 1.0;

};
#endif