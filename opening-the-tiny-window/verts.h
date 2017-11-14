#ifndef MUI_VERTS
#define MUI_VERTS

#include "glUtils.h"
#include <math.h>


enum NumCoords : uint32_t
{
  JoinedLines = 2,
  VerticalLines = 8,
  Fill = 12,
  Max = 12
};

void generateSineVertsInBuffer(const float freq, float vertexBuffer[],
  const uint32_t vertexBufferSize)
{
  const auto numVerts = vertexBufferSize / 2;
  const float step = 1.f / numVerts;

  for (auto i = 0; i < vertexBufferSize; i += 2) {
    vertexBuffer[i] = ((step * i) * 2.f) - 1.f; // x co-ord
    vertexBuffer[i + 1] = sin(i * step * M_PI * freq) * 0.5; // y co-ord
  }
}

uint32_t createJoinedLineVerticesFromAudio(
  float vertexBuffer[], const uint32_t vertexBufferSize,
  float audioBuffer[], const uint32_t numAudioFrames,
  const float xstart=-1.0, const float xend=1.0,
  const float ystart=-1.0, const float yend=1.0,
  const bool abs=true)
{
  const uint32_t numVerts = numAudioFrames * NumCoords::JoinedLines;
  if (vertexBufferSize < numVerts) {
    std::cout << "vertex buffer too small!" << std::endl;
    return 0;
  }

  const float xstep = (xend - xstart) / (numAudioFrames - 1);
  const float yrange = yend - ystart;

  for (auto i = 0; i < numAudioFrames; ++i) {
    const auto sample = abs ? fabs(audioBuffer[i]) : audioBuffer[i];
    const auto x = xstart + (i * xstep);
    const auto y = sample * yrange + (abs ? ystart : (ystart + (yrange / 2)));

    vertexBuffer[i * NumCoords::JoinedLines] = x;
    vertexBuffer[i * NumCoords::JoinedLines + 1] = y;
  }
  return numVerts;
}

uint32_t createVerticalLineVerticesFromAudio(
  float vertexBuffer[], const uint32_t vertexBufferSize,
  float audioBuffer[], const uint32_t numAudioFrames,
  const float xstart=-1.0, const float xend=1.0,
  const float ystart=-1.0, const float yend=1.0,
  const bool abs=true)
{
  const uint32_t numVerts = numAudioFrames * NumCoords::VerticalLines;
  if (vertexBufferSize < numVerts) {
    std::cout << "vertex buffer too small!" << std::endl;
    return 0;
  }

  const float xstep = (xend - xstart) / (numAudioFrames - 1);
  const float yrange = yend - ystart;

  for (auto i = 0; i < numAudioFrames; ++i) {
    const auto sample = abs ? fabs(audioBuffer[i]) : audioBuffer[i];
    const auto x = xstart + (i * xstep);
    const auto x2 = x + xstep;
    const auto y = abs ? ystart : ystart + (yrange / 2);
    const auto y2 = y + (sample * yrange);

    vertexBuffer[i * NumCoords::VerticalLines    ] = x2;
    vertexBuffer[i * NumCoords::VerticalLines + 1] = y;

    vertexBuffer[i * NumCoords::VerticalLines + 2] = x2;
    vertexBuffer[i * NumCoords::VerticalLines + 3] = y2;

    vertexBuffer[i * NumCoords::VerticalLines + 4] = x;
    vertexBuffer[i * NumCoords::VerticalLines + 5] = y2;

    vertexBuffer[i * NumCoords::VerticalLines + 6] = x;
    vertexBuffer[i * NumCoords::VerticalLines + 7] = y;
  }
  return numVerts;
}

// TODO make this moveable
uint32_t createFilledVerticesFromAudio(float vertexBuffer[],
  const uint32_t vertexBufferSize, float audioBuffer[],
  const uint32_t numAudioFrames, bool fullHeight=false)
{
  const auto numVerts = std::min(numAudioFrames * NumCoords::Fill,
                                 vertexBufferSize);
  const float start = -1.0;
  const float end = 1.0;
  const float step = (end - start) / (numAudioFrames - 1);

  for (auto i = 0; i < numAudioFrames; ++i) {
    const auto sample = fullHeight ? glUtils::normToContextPos(audioBuffer[i])
                                   : audioBuffer[i];
    const auto isLastSample = i == numAudioFrames - 1;
    const auto nextSample = isLastSample
      ? sample
      : (fullHeight
          ? glUtils::normToContextPos(audioBuffer[i + 1])
          : audioBuffer[i + 1]);
    const auto nextIsOpposite = isLastSample
      ? false
      : (sample > 0.0 && nextSample < 0.0) || (sample < 0.0 && nextSample > 0.0);
    // calculate zero crossing if swapping between pos <-> neg
    // calculated as percentage between sample x positions
    const auto proportion = nextIsOpposite ?
      fabs(sample) / (fabs(sample) + fabs(nextSample)) :
      1.0;

    // calculate points
    // triangle 0
    const auto t0x0 = glUtils::lingl(i, 0.0, numAudioFrames - 1);
    const auto t0y0 = sample;
    const auto t0x1 = t0x0;
    const auto t0y1 = fullHeight ? -1.0 : 0.0;
    const auto t0x2 = t0x0 + (step * proportion);
    const auto t0y2 = nextIsOpposite ? (fullHeight ? -1.0 : 0.0) : nextSample;
    // triangle 1
    const auto t1x0 = t0x0 + step;
    const auto t1y0 = nextSample;
    const auto t1x1 = t0x0 + step;
    const auto t1y1 = fullHeight ? -1.0 : 0.0;
    const auto t1x2 = nextIsOpposite ? t0x2 : t0x0;
    const auto t1y2 = nextIsOpposite ? t0y2 : t0y1;

    vertexBuffer[i * NumCoords::Fill    ] = t0x0;
    vertexBuffer[i * NumCoords::Fill + 1] = t0y0;
    vertexBuffer[i * NumCoords::Fill + 2] = t0x1;
    vertexBuffer[i * NumCoords::Fill + 3] = t0y1;
    vertexBuffer[i * NumCoords::Fill + 4] = t0x2;
    vertexBuffer[i * NumCoords::Fill + 5] = t0y2;
    vertexBuffer[i * NumCoords::Fill + 6] = t1x0;
    vertexBuffer[i * NumCoords::Fill + 7] = t1y0;
    vertexBuffer[i * NumCoords::Fill + 8] = t1x1;
    vertexBuffer[i * NumCoords::Fill + 9] = t1y1;
    vertexBuffer[i * NumCoords::Fill + 10] = t1x2;
    vertexBuffer[i * NumCoords::Fill + 11] = t1y2;
  }
  return numVerts;
}

uint32_t createFilledVerticesFromAudioAbs(
  float vertexBuffer[], const uint32_t vertexBufferSize,
  float audioBuffer[], const uint32_t numAudioFrames,
  const float xstart=-1.0, const float xend=1.0,
  const float ystart=-1.0, const float yend=1.0)
{
  const auto numVerts = std::min(numAudioFrames * NumCoords::Fill,
                                 vertexBufferSize);
  const float xstep = (xend - xstart) / (numAudioFrames - 1);
  const float yrange = yend - ystart;

  for (auto i = 0; i < numAudioFrames; ++i) {
    const auto sample = fabs(audioBuffer[i]);
    const auto isLast = i == numAudioFrames - 1;
    const auto nextSample = isLast ? sample : audioBuffer[i + 1];

    // calculate points
    // triangle 0
    const auto t0x0 = xstart + (i * xstep);
    const auto t0y0 = ystart + (sample * yrange);
    const auto t0x1 = t0x0;
    const auto t0y1 = ystart;
    const auto t0x2 = t0x0 + xstep;
    const auto t0y2 = ystart + (nextSample * yrange);

    // triangle 1
    const auto t1x0 = t0x2;
    const auto t1y0 = t0y2;
    const auto t1x1 = t0x1;
    const auto t1y1 = t0y1;
    const auto t1x2 = t1x0;
    const auto t1y2 = ystart;

    vertexBuffer[i * NumCoords::Fill     ] = t0x0;
    vertexBuffer[i * NumCoords::Fill +  1] = t0y0;
    vertexBuffer[i * NumCoords::Fill +  2] = t0x1;
    vertexBuffer[i * NumCoords::Fill +  3] = t0y1;
    vertexBuffer[i * NumCoords::Fill +  4] = t0x2;
    vertexBuffer[i * NumCoords::Fill +  5] = t0y2;
    vertexBuffer[i * NumCoords::Fill +  6] = t1x0;
    vertexBuffer[i * NumCoords::Fill +  7] = t1y0;
    vertexBuffer[i * NumCoords::Fill +  8] = t1x1;
    vertexBuffer[i * NumCoords::Fill +  9] = t1y1;
    vertexBuffer[i * NumCoords::Fill + 10] = t1x2;
    vertexBuffer[i * NumCoords::Fill + 11] = t1y2;
  }
  return numVerts;
}

#endif
