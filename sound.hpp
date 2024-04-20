#pragma once

#include <string>

#include "fft.hpp"

using namespace std;

class Sound {
 public:
  Sound(string fname, int nMargin = 1024);
  ~Sound();
  int fs() { return m_fs; }
  int nSamples() { return m_nSamples; }
  double duration() { return m_duration; }
  int nMargin() { return m_nMargin; }
  double *x() { return m_x; }
  complex<double> **spec() { return m_spec; }
  double specMax() { return m_specMax; }
  double specMin() { return m_specMin; }
  void stft(int hopSize, Window::WindowType windowType);

 private:
  int m_fs;
  int m_nSamples;
  int m_nChannels;
  double m_duration;
  int m_nMargin;
  double *m_x;
  FFT *m_fft;
  complex<double> **m_spec;
  double m_specMax;
  double m_specMin;
};