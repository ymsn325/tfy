#include <string>

#include "fft.hpp"

using namespace std;

class Sound {
 public:
  Sound(string fname, int nMargin = 1024);
  ~Sound();
  int nSamples() { return m_nSamples; }
  int nMargin() { return m_nMargin; }
  double *x() { return m_x; }
  double **spec() { return m_spec; }
  double specMax() { return m_specMax; }
  void stft(int hopSize);

 private:
  int m_fs;
  int m_nSamples;
  int m_nChannels;
  double m_duration;
  int m_nMargin;
  double *m_x;
  FFT *m_fft;
  double **m_spec;
  double m_specMax;
};