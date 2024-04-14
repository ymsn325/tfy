#include "sound.hpp"

#include <climits>
#include <cstring>
#include <fstream>
#include <iostream>

using namespace std;

Sound::Sound(string fname, int nMargin) {
  ifstream fin;
  char tag[4];
  int buf4;
  short buf2;
  int blockSize;
  m_nMargin = nMargin;
  cerr << "Read file: " << fname << endl;
  fin.open(fname, ios::in | ios::binary);
  if (!fin) {
    cerr << "Cannot open file: " << fname << endl;
    return;
  }
  fin.read(tag, 4);  // "RIFF"
  if (strncmp(tag, "RIFF", 4)) {
    cerr << "File is not RIFF format." << endl;
    return;
  }
  fin.read((char *)&buf4, 4);  // RIFFチャンクサイズ
  fin.read(tag, 4);            // "WAVE"
  if (strncmp(tag, "WAVE", 4)) {
    cerr << "File is not WAVE format." << endl;
    return;
  }
  fin.read(tag, 4);  // "fmt "
  if (strncmp(tag, "fmt ", 4)) {
    cerr << "fmt chunk is invalid." << endl;
    return;
  }
  fin.read((char *)&buf4, 4);  // fmtチャンクサイズ
  if (buf4 != 16) {
    cerr << "Unsupported format." << endl;
    return;
  }
  fin.read((char *)&buf2, 2);  // 音声フォーマット
  fin.read((char *)&buf2, 2);  // チャネル数
  m_nChannels = buf2;
  fin.read((char *)&buf4, 4);  // サンプルレート
  m_fs = buf4;
  cerr << m_fs << " Hz" << endl;
  fin.read((char *)&buf4, 4);  // bytes / sec
  fin.read((char *)&buf2, 2);  // ブロックサイズ
  blockSize = buf2;
  fin.read((char *)&buf2, 2);  // ビット深度
  fin.read(tag, 4);            // "data"
  if (strncmp(tag, "data", 4)) {
    cerr << "data chunk not found." << endl;
    return;
  }
  fin.read((char *)&buf4, 4);  // dataチャンクサイズ
  m_nSamples = buf4 / blockSize;
  m_duration = (double)m_nSamples / m_fs;
  cerr << m_duration << " sec" << endl;
  m_x = new double[m_nSamples + 2 * nMargin];
  for (int n = 0; n < nMargin; n++) {
    m_x[n] = 0.0;
    m_x[m_nSamples + nMargin + n] = 0.0;
  }
  for (int n = nMargin; n < m_nSamples + nMargin; n++) {
    fin.read((char *)&buf2, 2);
    m_x[n] = (double)(buf2 + (SHRT_MAX + 1.0) + 0.5) / (SHRT_MAX + 1.0) - 1.0;
  }
  fin.close();
  m_fft = new FFT(2048, Window::Gaussian, m_fs);
}

Sound::~Sound() {
  delete[] m_x;
  delete m_fft;
}

void Sound::stft(int hopSize) {
  int nFFT = m_fft->nFFT();
  if (m_nMargin < nFFT / 2) {
    cerr << "Too short nMargin: " << m_nMargin << ", nFFT: " << nFFT << endl;
    return;
  }
  double *in = new double[nFFT];
  complex<double> *out = new complex<double>[nFFT];
  double *window = m_fft->window();
  double width = m_nSamples / hopSize;
  m_spec = new complex<double> *[int(width)];
  for (int i = 0; i < width; i++) {
    m_spec[i] = new complex<double>[nFFT / 2];
  }
  m_specMax = 0.0;
  m_specMin = 1.0;
  for (int i = 0; i < m_nSamples / hopSize; i++) {
    for (int n = -nFFT / 2; n < nFFT / 2; n++) {
      in[n + nFFT / 2] =
          m_x[i * hopSize + m_nMargin + n] * window[n + nFFT / 2];
    }
    m_fft->exec(in, out);
    for (int k = 0; k < nFFT / 2; k++) {
      m_spec[i][k] = out[k];
      if (abs(m_spec[i][k]) > m_specMax) {
        m_specMax = abs(m_spec[i][k]);
      }
      if (abs(m_spec[i][k]) < m_specMin) {
        m_specMin = abs(m_spec[i][k]);
      }
    }
  }
  delete[] in;
  delete[] out;
}
