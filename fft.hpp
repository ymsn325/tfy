#pragma once

#include <complex>

using namespace std;

enum class Window { Gaussian, Hann, Hamming, Rect, NumWindow };

class FFT {
 public:
  FFT(int nFFT, Window windowType, double fs);
  ~FFT();
  int nFFT() { return m_nFFT; }
  double *window() { return m_window; }
  void exec(double *in, complex<double> *out);
  void setWindow(Window windowType) {
    m_window = genWindow(windowType);
    m_areaWindow = calcWindowArea();
  }

 private:
  int *genBitRevTable();
  double *genWindow(Window windowType);
  double calcWindowArea();
  complex<double> *genCoef();
  int m_nFFT;
  double *m_window;
  double m_areaWindow;
  double m_fs;
  int *m_bitRevTable;
  complex<double> *m_coef;
};