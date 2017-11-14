#include "audio.h"
#include <iostream>
#include <Accelerate/Accelerate.h>

static const uint32_t originalBufSize = 20;
float originalBuf[originalBufSize] = {0.0};

static const uint32_t modBufSize = 20;
float modBuf[modBufSize] = {0.0};

void printBuffer(const float buf[], const uint32_t bufSize, std::string label)
{
  for (uint32_t i = 0; i < bufSize; ++i) {
    std::cout << label << "[" << i << "]: " << (buf[i]) << std::endl;
  }
}

int main(int argc, char const *argv[])
{
  const float rampStart = 1.0;
  const float rampStep = 1.0;
  const vDSP_Stride rampStride = 1;
  const vDSP_Length rampLength = originalBufSize;

  vDSP_vramp(&rampStart, &rampStep, originalBuf, rampStride, rampLength);
  printBuffer(originalBuf, originalBufSize, "ramp");

  const vDSP_Stride inStride = 1;
  const vDSP_Stride outStride = 1;
  const vDSP_Length smoothLength = originalBufSize;
  const vDSP_Length winLength = 3;

  smoothBuffer(originalBuf, inStride, modBuf, outStride, smoothLength, winLength);
  printBuffer(modBuf, modBufSize, "smooth");

  return 0;
}