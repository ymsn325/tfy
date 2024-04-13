#include "fft.hpp"

#include <QtMath>
#include <iostream>

using namespace std;

FFT::FFT(int nFFT, Window windowType, double fs) {
  m_nFFT = nFFT;
  m_bitRevTable = genBitRevTable();
  m_window = genWindow(windowType);
  m_areaWindow = calcWindowArea();
  m_coef = genCoef();
  m_fs = fs;
}

FFT::~FFT() {
  delete[] m_bitRevTable;
  delete[] m_window;
  delete[] m_coef;
}

int* FFT::genBitRevTable() {
  int* bitRevTable = new int[m_nFFT];
  int bitNum = log2(m_nFFT);
  for (int i = 0; i < m_nFFT; i++) {
    bitRevTable[i] = 0;
    for (int j = bitNum - 1; j >= 0; j--) {
      bitRevTable[i] |= (1 & (i >> j)) << ((bitNum - 1) - j);
    }
  }
  return bitRevTable;
}

double* FFT::genWindow(Window windowType) {
  double* window = new double[m_nFFT];
  switch (windowType) {
    case Window::Gaussian:
      for (int i = 0; i < m_nFFT; i++) {
        window[i] = exp(-pow(3.0 * (m_nFFT / 2.0 - i) / (m_nFFT / 2.0), 2.0));
      }
      break;
    case Window::Hamming:
      for (int i = 0; i < m_nFFT; i++) {
        window[i] = 0.54 - 0.46 * cos(2.0 * M_PI * i / m_nFFT);
      }
      break;
    case Window::Hann:
      for (int i = 0; i < m_nFFT; i++) {
        window[i] = 0.5 - 0.5 * cos(2.0 * M_PI * i / m_nFFT);
      }
      break;
    case Window::Rect:
      for (int i = 0; i < m_nFFT; i++) {
        window[i] = 1.0;
      }
      break;
    default:
      cerr << "Unsupported window type." << endl;
      return nullptr;
  }
  return window;
}

double FFT::calcWindowArea() {
  double sum = 0.0;
  for (int i = 0; i < m_nFFT; i++) {
    sum += m_window[i];
  }
  return sum;
}

complex<double>* FFT::genCoef() {
  complex<double>* coef = new complex<double>[m_nFFT];
  for (int i = 0; i < m_nFFT / 2.0; i++) {
    coef[i] = exp(-2.0 * M_PI / m_nFFT * i * 1.0i);
  }
  return coef;
}

void FFT::exec(double* in, complex<double>* out) {
  complex<double>* tmp = new complex<double>[m_nFFT];
  complex<double> tmptmp;
  for (int i = 0; i < m_nFFT; i++) {
    tmp[i] = m_window[i] * in[i];
  }
  int iMax = log2(m_nFFT);
  for (int i = 0; i < iMax; i++) {
    int jMax = 1 << i;
    for (int j = 0; j < jMax; j++) {
      int kMax = m_nFFT / (1 << (i + 1));
      for (int k = 0; k < kMax; k++) {
        tmptmp = tmp[j * (kMax << 1) + k];
        tmp[j * (kMax << 1) + k] += tmp[j * (kMax << 1) + kMax + k];
        tmp[j * (kMax << 1) + kMax + k] =
            (-tmp[j * (kMax << 1) + kMax + k] + tmptmp) * m_coef[k * (1 << i)];
      }
    }
  }
  for (int i = 0; i < m_nFFT; i++) {
    out[m_bitRevTable[i]] = tmp[i] / m_areaWindow;
  }
  delete[] tmp;
}