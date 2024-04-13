#include "mainwindow.hpp"

#include <QDebug>
#include <QImage>
#include <QPixmap>

WaveView::WaveView(int x, int y, int w, int h, QWidget *parent)
    : QGraphicsView(parent) {
  m_scene = new QGraphicsScene(x, y, w, h, parent);
  m_scene->setBackgroundBrush(QColor(0, 0, 80));
  m_scene->addLine(0, m_scene->height() / 2, m_scene->width(),
                   m_scene->height() / 2, QColor(100, 100, 200));
  setScene(m_scene);
}

void WaveView::drawWaveForm(Sound *sound) {
  int w = m_scene->width();
  int h = m_scene->height();
  double bias = h / 2.0;
  double gain = h / 2.0;
  double samplesPerPix = sound->nSamples() / w;
  double *x = sound->x();
  int nMargin = sound->nMargin();
  double max, min;
  for (int i = 0; i < w; i++) {
    max = -1.0;
    min = 1.0;
    for (int n = i * samplesPerPix; n < (i + 1) * samplesPerPix; n++) {
      if (x[(int)n + nMargin] > max) {
        max = x[(int)n + nMargin];
      }
      if (x[(int)n + nMargin] < min) {
        min = x[(int)n + nMargin];
      }
    }
    m_scene->addLine(i, -min * gain + bias, i, -max * gain + bias,
                     QColor("white"));
  }
}

TFView::TFView(int x, int y, int w, int h, QWidget *parent)
    : QGraphicsView(parent) {
  m_scene = new QGraphicsScene(x, y, w, h, parent);
  m_scene->setBackgroundBrush(QColor("black"));
  setScene(m_scene);
}

void TFView::drawTFMap(Sound *sound) {
  int w = m_scene->width();
  int h = m_scene->height();
  int hopSize = sound->nSamples() / w;
  sound->stft(hopSize);
  double **spec = sound->spec();
  unsigned char *buf = new unsigned char[h * w];
  double specMax = sound->specMax();
  for (int y = 0; y < h; y++) {
    for (int x = 0; x < w; x++) {
      buf[y * w + x] = (spec[x][y] + 100.0) / (specMax + 100.0) * 255.0;
    }
  }
  QImage img(buf, w, h, QImage::Format_Grayscale8);
  QPixmap pixmap = QPixmap::fromImage(img);
  m_scene->addPixmap(pixmap);
}

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
  m_centralWidget = new QWidget(this);
  m_topLayout = new QVBoxLayout(m_centralWidget);
  m_topLayout->setSpacing(0);
  m_tfView = new TFView(0, 0, 800, 1024, m_centralWidget);
  m_waveView = new WaveView(0, 0, 800, 200, m_centralWidget);
  m_topLayout->addWidget(m_tfView);
  m_topLayout->addWidget(m_waveView);
  m_centralWidget->setLayout(m_topLayout);
  setCentralWidget(m_centralWidget);
  //   m_sound = new Sound("C:/Users/yamas/Documents/audio/sin1k.wav");
  m_sound =
      new Sound("C:/Users/yamas/Documents/audio/ichimoji_PF02_0501_033.wav");
  m_waveView->drawWaveForm(m_sound);
  m_tfView->drawTFMap(m_sound);
}

MainWindow::~MainWindow() {}
