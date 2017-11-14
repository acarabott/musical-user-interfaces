#ifndef MUI_AUDIODEVICE
#define MUI_AUDIODEVICE

#include <iostream>
#include <ao/ao.h>
#include <vector>
#include "Buffer.h"
#include "Effect.h"

// #include "Clipper.h"

class AudioDevice
{
public:
  AudioDevice(Buffer& _inputBuffer) : inputBuffer(_inputBuffer)
  {
    ao_initialize();
    audioDriver = ao_default_driver_id();

    memset(&format, 0, sizeof(format));
    format.bits = bits;
    format.channels = channels;
    format.rate = sampleRate;
    format.byte_format = AO_FMT_LITTLE;

    device = ao_open_live(audioDriver, &format, NULL /* no options */);
    if (device == NULL) {
      std::cout << "Error opening device" << std::endl;
      return;
    }
  }

  ~AudioDevice()
  {
    ao_close(device);
    ao_shutdown();
  }

  // TODO use vDSP_vfix8 ?
  void updateOutputBuffer()
  {
    for (uint32_t i = 0; i < blockSize; ++i) {
      const int32_t sample = (int32_t)(fxBuffer[i] * 32768.0);
      outputBuffer[4 * i   ] = outputBuffer[4 * i + 2] = sample & 0xff;
      outputBuffer[4 * i +1] = outputBuffer[4 * i + 3] = (sample >> 8) & 0xff;
    }
  }

  void playBlock() {
    ao_play(device, outputBuffer, outBufferSize);
    m_currentFrame += blockSize;

    if (m_currentFrame >= inputBuffer.getNumFrames()) {
      if (!loop) {
        stop();
      }
      m_currentFrame = 0;
    }
  }

  void process()
  {
    copyBuffer(inputBuffer.getOffsetAudio(m_currentFrame), fxBuffer, blockSize);
    for (auto effect : fx) {
      effect->processAudio(fxBuffer, fxBuffer, blockSize);
    }
    updateOutputBuffer();
  }

  void run() {
    m_isRunning = true;
    while (m_isRunning) {
      if (m_isPlaying) {
        process();
        playBlock();
      }
    }
  }

  void togglePlayback() { m_isPlaying = !m_isPlaying; }

  uint32_t getCurrentFrame() { return m_currentFrame; }
  uint32_t getSampleRate() { return sampleRate; }

  void enableJITProcessing() { doJITProcessing = true; }
  void disableJITProcessing() { doJITProcessing = false; }

  void play() { m_isPlaying = true; }
  void stop() { m_isPlaying = false; }
  void kill() { m_isPlaying = m_isRunning = false; }
  bool isPlaying() { return m_isPlaying; };
  bool isStopped() { return !m_isPlaying; };

  void addEffect(Effect* effect) { fx.push_back(effect); }

  float getPlayheadPosition() {
    return (float)m_currentFrame / (float)inputBuffer.getNumFrames();
  }

  void setPlayheadPosition(float position) {
    m_currentFrame = position * inputBuffer.getNumFrames();
  }

  void enableLooping() { loop = true; }
  void disableLooping() { loop = true; }

protected:
  static const uint32_t blockSize = 512;
  static const uint32_t sampleRate = 44100;
  static const uint32_t bits = 16;
  static const uint32_t channels = 2;
  static const uint32_t outBufferSize = bits / 8 * channels * blockSize;

  ao_device* device;
  ao_sample_format format;
  uint32_t audioDriver;

  Buffer& inputBuffer;
  char outputBuffer[outBufferSize];
  float fxBuffer[blockSize];

  volatile uint32_t m_currentFrame = 0;
  volatile bool m_isPlaying = false;
  volatile bool m_isRunning = true;
  bool loop = false;
  std::vector<Effect*> fx;

  bool doJITProcessing = false;

  uint32_t framesToBufferSize(uint32_t numFrames)
  {
    return format.bits / 8 * format.channels * numFrames;
  }
};
#endif
