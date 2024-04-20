#include "fft.hpp"

#include <QDebug>
#include <QtMath>
#include <iostream>

using namespace std;

Window::Window(int nFFT, int size, WindowType type) {
  m_data = new double[nFFT];
  m_area = 0.0;
  for (int i = 0; i < nFFT; i++) {
    m_data[i] = 0.0;
  }
  switch (type) {
    case WindowType::Gaussian:
      qDebug() << "Window type: Gaussian";
      for (int i = 0; i < nFFT; i++) {
        m_data[i] = exp(-pow(3.0 * (nFFT / 2.0 - i) / (size / 2.0), 2.0));
        m_area += m_data[i];
      }
      break;
    case WindowType::Hamming:
      qDebug() << "Window type: Hamming";
      for (int i = nFFT / 2 - size / 2; i < nFFT / 2 + size / 2; i++) {
        m_data[i] =
            0.54 - 0.46 * cos(2.0 * M_PI * (i - nFFT / 2 - size / 2) / size);
        m_area += m_data[i];
      }
      break;
    case WindowType::Hann:
      qDebug() << "Window type: Hann";
      for (int i = nFFT / 2 - size / 2; i < nFFT / 2 + size / 2; i++) {
        m_data[i] =
            0.5 - 0.5 * cos(2.0 * M_PI * (i - nFFT / 2 - size / 2) / size);
        m_area += m_data[i];
      }
      break;
    case WindowType::Rect:
      qDebug() << "Window type: Rect";
      for (int i = nFFT / 2 - size / 2; i < nFFT / 2 + size / 2; i++) {
        m_data[i] = 1.0;
        m_area += m_data[i];
      }
      break;
    default:
      cerr << "Unsupported window type." << endl;
      cerr << "Force set to Rectangle window." << endl;
      for (int i = nFFT / 2 - size / 2; i < nFFT / 2 + size / 2; i++) {
        m_data[i] = 1.0;
        m_area += m_data[i];
      }
      break;
  }
}

FFT::FFT(int nFFT, Window::WindowType windowType, double fs) {
  m_nFFT = nFFT;
  m_window = new Window(nFFT, nFFT, Window::WindowType::Gaussian);
  m_bitRevTable = genBitRevTable();
  m_coef = genCoef();
  m_fs = fs;
}

FFT::~FFT() {
  delete[] m_bitRevTable;
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
    tmp[i] = m_window->data()[i] * in[i];
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
    out[m_bitRevTable[i]] = tmp[i] / m_window->area();
  }
  delete[] tmp;
}