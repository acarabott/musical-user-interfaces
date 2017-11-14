#ifndef MUI_AUDIO
#define MUI_AUDIO

#include <Accelerate/Accelerate.h>
#include <math.h>
#include <sndfile.h>

void smoothBuffer(float inBuf[], const vDSP_Stride inStride,
                  float outBuf[], const vDSP_Stride outStride,
                  const vDSP_Length outLength, vDSP_Length windowLength)
{
  // const uint32_t halfWindow = windowLength / 2;
  // for (uint32_t i = 0; i < outLength; i++) {
  //   float sum = 0.0;
  //   if (windowLength == 1) {
  //       outBuf[i] = inBuf[i];
  //   } else {
  //     const uint32_t start = std::max((uint32_t)0, i - halfWindow);
  //     const uint32_t end = std::min((uint32_t)outLength - 1, i + halfWindow);
  //     for (auto j = start; j < end; ++j) {
  //       sum += inBuf[j];
  //     }
  //     outBuf[i] = sum / (end - start);
  //   }
  // }
  vDSP_vswsum(inBuf, inStride, outBuf, outStride, outLength, windowLength);
  float divScalar = windowLength;
  vDSP_vsdiv(outBuf, inStride, &divScalar, outBuf, outStride, outLength);
}

void smoothBufferMax(float inBuf[], const vDSP_Stride inStride,
                     float outBuf[], const vDSP_Stride outStride,
                     const vDSP_Length outLength, vDSP_Length windowLength)
{
  vDSP_Length numWindowPositions = outLength - (windowLength - 1);
  vDSP_vswmax(inBuf, inStride, outBuf, outStride, numWindowPositions, windowLength);

  // smoothBuffer(outBuf, outStride, outBuf, outStride, outLength, windowLength);
}

enum class BufferReduction { Sample, Abs, Avg, Max, AbsMax, MinMax};

void reduceAudioForVisualisation(BufferReduction approach,
  float inAudioBuffer[], const uint32_t inAudioBufferSize,
  float outAudioBuffer[], const uint32_t outAudioBufferSize)
{
  const uint32_t stride = std::max((uint32_t)1,
    inAudioBufferSize / outAudioBufferSize);

  if (approach == BufferReduction::Sample || approach == BufferReduction::Abs) {
    for (uint32_t i = 0; i < inAudioBufferSize; i++) {
      if (i % stride == 0) {
        const float sample = approach == BufferReduction::Abs
          ? fabs(inAudioBuffer[i])
          : inAudioBuffer[i];
        outAudioBuffer[i / stride] = sample;
      }
    }
  }
  else if (approach == BufferReduction::Max || approach == BufferReduction::AbsMax) {
    float maxAbs = 0.0;
    uint32_t maxIdx = 0;
    for (uint32_t i = 0; i < inAudioBufferSize; i++) {
      const float absSample = fabs(inAudioBuffer[i]);
      if (absSample > maxAbs) {
        maxAbs = absSample;
        maxIdx = i;
      }

      if (i % stride == 0) {
        const float sample = approach == BufferReduction::AbsMax ?
          fabs(inAudioBuffer[maxIdx]):
          inAudioBuffer[maxIdx];
        outAudioBuffer[i / stride] = sample;
        maxAbs = 0.0;
        maxIdx = 0;
      }
    }
  }
  else if (approach == BufferReduction::Avg) {
    float sum = 0.0;
    for (uint32_t i = 0; i < inAudioBufferSize; i++) {
      sum += fabs(inAudioBuffer[i]);
      if (i == 0) {
        outAudioBuffer[i] = inAudioBuffer[i];
        sum = 0.0;
      }
      else if (i % stride == 0) {
        outAudioBuffer[i / stride] = (sum / stride) * 2.f;
        sum = 0.0;
      }
    }
  }
  else if (approach == BufferReduction::Abs) {
    for (uint32_t i = 0; i < inAudioBufferSize; ++i) {
      if (i % stride == 0) {
        outAudioBuffer[i / stride] = fabs(inAudioBuffer[i]);
      }
    }
  }
  else if (approach == BufferReduction::MinMax) {
    uint32_t winSize = stride * 2;
    for (uint32_t i = 0; i < inAudioBufferSize; ++i) {
      if (i % winSize == 0) {
        uint32_t halfIdx = i + stride;
        float min0 = FLT_MAX;
        float max0 = 0.0;
        float sum0 = 0.0;
        float min1 = FLT_MAX;
        float max1 = 0.0;
        float sum1 = 0.0;
        for (uint32_t j = i; j < i + winSize; ++j) {
          float inAbs = fabs(inAudioBuffer[j]);
          if (j < halfIdx) {
            sum0 += inAbs;
            min0 = fmin(min0, inAbs);
            max0 = fmax(max0, inAbs);
          } else {
            sum1 += inAbs;
            min1 = fmin(min1, inAbs);
            max1 = fmax(max1, inAbs);
          }
        }
        const auto idx = i / stride;
        const bool firstBigger = sum0 > sum1;
        outAudioBuffer[idx] = firstBigger ? max0 : min0;
        outAudioBuffer[idx + 1] = firstBigger ? min1 : max1;
      }
    }
  }
}

uint32_t readSoundFileIntoBuffer(std::string path, float audioBuffer[],
  const uint32_t audioBufferSize)
{
  SF_INFO info;
  info.format = 0;
  SNDFILE* sndfile = sf_open(path.c_str(), SFM_READ, &info);
  if (sndfile == NULL) {
    std::cout << sf_strerror(sndfile) << std::endl;
  }

  if (audioBufferSize < info.frames) {
    std::cout << "warning, buffer too small for full audio file" << std::endl;
  }
  const auto numFrames = std::min(info.frames, (sf_count_t)audioBufferSize);

  sf_read_float(sndfile, audioBuffer, numFrames);
  sf_close(sndfile);

  return (uint32_t)numFrames;
}

uint32_t getSoundFileNumFrames(std::string path)
{
  SF_INFO info;
  info.format = 0;
  SNDFILE* sndfile = sf_open(path.c_str(), SFM_READ, &info);
  if (sndfile == NULL) {
    std::cout << sf_strerror(sndfile) << std::endl;
  }
  sf_close(sndfile);

  return (uint32_t)info.frames;
}

void clearBuffer(float buf[], const vDSP_Length bufSize)
{
  float val = 0.f;
  vDSP_vfill(&val, buf, 1, bufSize);
}

void copyBuffer(float inBuf[], float outBuf[], const uint32_t bufSize)
{
  cblas_scopy(bufSize, inBuf, 1, outBuf, 1);
}

float getBufferAbsMax(float buf[], const vDSP_Length bufSize)
{
  float absMax;
  float* temp = new float[bufSize];
  vDSP_vabs(buf, 1, temp, 1, bufSize);
  vDSP_maxv(temp, 1, &absMax, bufSize);
  delete[] temp;
  return absMax;
}

#endif