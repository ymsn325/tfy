#pragma once

#include <complex>

using namespace std;

class Window {
 public:
  enum WindowType { Gaussian, Hann, Hamming, Rect, NumWindow };
  Window(int nFFT, int size, WindowType type);
  ~Window() { delete[] m_data; }
  double *data() { return m_data; }
  double area() { return m_area; }

 private:
  double *m_data;
  double m_area;
};

class FFT {
 public:
  FFT(int nFFT, Window::WindowType windowType, double fs);
  ~FFT();
  int nFFT() { return m_nFFT; }
  Window *window() { return m_window; }
  void exec(double *in, complex<double> *out);
  void setWindow(Window::WindowType windowType, int windowSize) {
    delete m_window;
    m_window = new Window(m_nFFT, windowSize, windowType);
  }

 private:
  int *genBitRevTable();
  complex<double> *genCoef();
  int m_nFFT;
  Window *m_window;
  double m_fs;
  int *m_bitRevTable;
  complex<double> *m_coef;
};