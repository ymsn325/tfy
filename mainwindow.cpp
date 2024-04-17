#include "mainwindow.hpp"

#include <QDebug>
#include <QImage>
#include <QPixmap>
#include <iostream>

#include "playback.hpp"

using namespace std;

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
  m_data = new unsigned char[w * h * 3];
  setScene(m_scene);
}

TFView::~TFView() { delete[] m_data; }

void TFView::drawTFMap(Sound *sound) {
  int w = m_scene->width();
  int h = m_scene->height();
  int hopSize = sound->nSamples() / w;
  sound->stft(hopSize);
  complex<double> **spec = sound->spec();
  double specMax = sound->specMax();
  double specMin = sound->specMin();
  double upper_dB, lower_dB;
  upper_dB = 20.0 * log10(specMax);
  // lower_dB = 20.0 * log10(specMin);
  lower_dB = -100.0;
  for (int y = 0; y < h; y++) {
    for (int x = 0; x < w; x++) {
      double2rgb(
          (20.0 * log10(abs(spec[x][y])) - lower_dB) / (upper_dB - lower_dB),
          m_data + ((h - 1 - y) * w + x) * 3,       // R
          m_data + ((h - 1 - y) * w + x) * 3 + 1,   // G
          m_data + ((h - 1 - y) * w + x) * 3 + 2);  // B
    }
  }
  QImage img(m_data, w, h, QImage::Format_RGB888);
  QPixmap pixmap = QPixmap::fromImage(img);
  m_scene->addPixmap(pixmap);
}

void TFView::double2rgb(double x, unsigned char *r, unsigned char *g,
                        unsigned char *b) {
  if (x > 1.0) {
    x = 1.0;
  }
  if (x < 0.0) {
    x = 0.0;
  }
  if (x < 3.0 / 7.0) {
    *r = 0;
  } else if (x < 4.0 / 7.0) {
    *r = 255.0 * (x - (3.0 / 7.0)) / ((4.0 - 3.0) / 7.0);
  } else {
    *r = 255;
  }
  if (x < 1.0 / 7.0) {
    *g = 0;
  } else if (x < 2.0 / 7.0) {
    *g = 255.0 * (x - (3.0 / 7.0)) / ((2.0 - 1.0) / 7.0);
  } else if (x < 4.0 / 7.0) {
    *g = 255;
  } else if (x < 5.0 / 7.0) {
    *g = 255.0 * ((5.0 / 7.0) - x) / ((5.0 - 4.0) / 7.0);
  } else if (x < 6.0 / 7.0) {
    *g = 0;
  } else {
    *g = 255 * (x - 6.0 / 7.0) / ((7.0 - 6.0) / 7.0);
  }
  if (x < 1.0 / 7.0) {
    *b = 255.0 * x / (1.0 / 7.0);
  } else if (x < 2.0 / 7.0) {
    *b = 255;
  } else if (x < 3.0 / 7.0) {
    *b = 255.0 * (3.0 / 7.0 - x) / ((3.0 - 2.0) / 7.0);
  } else if (x < 5.0 / 7.0) {
    *b = 0;
  } else if (x < 6.0 / 7.0) {
    *b = 255.0 * (x - 5.0 / 7.0) / ((6.0 - 5.0) / 7.0);
  } else {
    *b = 255;
  }
}

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
  m_centralWidget = new QWidget(this);
  m_topLayout = new QVBoxLayout(m_centralWidget);
  m_topLayout->setSpacing(0);
  m_tfView = new TFView(0, 0, 800, 1024, m_centralWidget);
  m_waveView = new WaveView(0, 0, 800, 200, m_centralWidget);
  m_playbackWidget = new QWidget(this);
  m_playbackLayout = new QHBoxLayout();
  m_volSlider = new QSlider(Qt::Horizontal, this);
  m_volSlider->setSizePolicy(
      QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding));
  m_volSlider->setTickPosition(QSlider::TicksBelow);
  m_volSlider->setTickInterval(50);
  m_volSlider->setValue(m_volSlider->maximum());
  connect(m_volSlider, &QSlider::valueChanged, this,
          &MainWindow::volSliderValueChangedHandler);
  m_playButton = new QPushButton("Play", this);
  m_playButton->setSizePolicy(
      QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding));
  m_playbackLayout->addWidget(m_volSlider);
  m_playbackLayout->addWidget(m_playButton);
  connect(m_playButton, &QPushButton::clicked, this,
          &MainWindow::playButtonClickedHandler);
  m_topLayout->addWidget(m_tfView);
  m_topLayout->addWidget(m_waveView);
  m_topLayout->addLayout(m_playbackLayout);
  setCentralWidget(m_centralWidget);
  m_sound =
      new Sound("C:/Users/yamas/Documents/audio/ichimoji_PF02_0501_033.wav");
  m_waveView->drawWaveForm(m_sound);
  m_tfView->drawTFMap(m_sound);
  m_audioDev = new QMediaDevices(this);
  m_audioStream.reset(new AudioStream(m_sound));
  connect(m_audioStream.get(), &AudioStream::stopped, this,
          &MainWindow::streamStoppedHandler);
  m_audioStream->start();
  QAudioFormat audioFormat;
  audioFormat.setChannelCount(1);
  audioFormat.setSampleRate(44100.0);
  audioFormat.setSampleFormat(QAudioFormat::Int16);
  m_audioSink.reset(
      new QAudioSink(m_audioDev->defaultAudioOutput(), audioFormat));
  m_audioIO = m_audioSink->start();
  m_playFlag = false;
  m_audioPlaybackTimer = new QTimer(this);
  connect(m_audioPlaybackTimer, &QTimer::timeout, this,
          &MainWindow::playbackTimerTimeoutHandler);
}

MainWindow::~MainWindow() {}

void MainWindow::playButtonClickedHandler() {
  if (m_playFlag == false) {
    m_playFlag = true;
    m_playButton->setText("Pause");
    m_audioPlaybackTimer->start(10);
  } else {
    m_playFlag = false;
    m_playButton->setText("Play");
    m_audioPlaybackTimer->stop();
  }
}

void MainWindow::streamStoppedHandler() {
  m_playButton->setText("Play");
  m_audioPlaybackTimer->stop();
  m_playFlag = false;
}

void MainWindow::playbackTimerTimeoutHandler() {
  int len = m_audioSink->bytesFree();
  QByteArray buf(len, 0);
  len = m_audioStream->read(buf.data(), len);
  if (len) {
    m_audioIO->write(buf.data(), len);
  }
}

void MainWindow::volSliderValueChangedHandler(int val) {
  m_audioSink->setVolume(val / 100.0);
}