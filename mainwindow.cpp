#include "mainwindow.hpp"

#include <QDebug>
#include <QFileDialog>
#include <QImage>
#include <QPixmap>
#include <iostream>

#include "fft.hpp"
#include "playback.hpp"

using namespace std;

WaveView::WaveView(int x, int y, int w, int h, QWidget *parent)
    : QGraphicsView(parent) {
  m_scene = new QGraphicsScene(x, y, w, h, parent);
  m_scene->setBackgroundBrush(QColor(0, 0, 80));
  init();
  setScene(m_scene);
}

void WaveView::init() {
  m_scene->clear();
  m_scene->addLine(0, m_scene->height() / 2, m_scene->width(),
                   m_scene->height() / 2, QColor(100, 100, 200));
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

TFScene::TFScene(int x, int y, int w, int h, QWidget *parent)
    : QGraphicsScene(x, y, w, h, parent) {}

void TFScene::mouseMoveEvent(QGraphicsSceneMouseEvent *e) {
  qDebug() << e->scenePos();
}

TFView::TFView(int x, int y, int w, int h, QWidget *parent)
    : QGraphicsView(parent) {
  m_scene = new TFScene(x, y, w, h, parent);
  m_scene->setBackgroundBrush(QColor("black"));
  m_data = new unsigned char[w * h * 3];
  setMouseTracking(true);
  setScene(m_scene);
}

TFView::~TFView() { delete[] m_data; }

void TFView::drawTFMap(Sound *sound, Window windowType) {
  int w = m_scene->width();
  int h = m_scene->height();
  int hopSize = sound->nSamples() / w;
  sound->stft(hopSize, windowType);
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
  createMenuBar();
  m_centralWidget = new QWidget(this);
  m_topLayout = new QVBoxLayout(m_centralWidget);
  m_topLayout->setSpacing(0);
  m_upperLayout = new QHBoxLayout();
  m_pixmapLayout = new QVBoxLayout();
  m_tfView = new TFView(0, 0, 800, 1024, this);
  m_waveView = new WaveView(0, 0, 800, 100, this);
  m_pixmapLayout->addWidget(m_tfView);
  m_pixmapLayout->addWidget(m_waveView);
  m_upperLayout->addLayout(m_pixmapLayout);
  m_windowComboBox = new QComboBox(this);
  for (int w = 0; w < (int)Window::NumWindow; w++) {
    switch ((Window)w) {
      case Window::Gaussian:
        m_windowComboBox->addItem("Gauusian");
        break;
      case Window::Hann:
        m_windowComboBox->addItem("Hann");
        break;
      case Window::Hamming:
        m_windowComboBox->addItem("Hamming");
        break;
      case Window::Rect:
        m_windowComboBox->addItem("Rect");
        break;
      default:
        break;
    }
  }
  m_windowComboBox->setSizePolicy(
      QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
  connect(m_windowComboBox, &QComboBox::currentIndexChanged, this,
          &MainWindow::windowChangedHandler);
  m_upperLayout->addWidget(m_windowComboBox);
  m_lowerLayout = new QHBoxLayout();
  m_volSlider = new QSlider(Qt::Horizontal, this);
  m_volSlider->setSizePolicy(
      QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
  m_volSlider->setTickPosition(QSlider::TicksBelow);
  m_volSlider->setTickInterval(50);
  m_volSlider->setValue(m_volSlider->maximum());
  connect(m_volSlider, &QSlider::valueChanged, this,
          &MainWindow::volSliderValueChangedHandler);
  m_playButton = new QPushButton("Play", this);
  m_playButton->setSizePolicy(
      QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
  m_lowerLayout->addWidget(m_volSlider);
  m_lowerLayout->addWidget(m_playButton);
  connect(m_playButton, &QPushButton::clicked, this,
          &MainWindow::playButtonClickedHandler);
  m_topLayout->addLayout(m_upperLayout);
  m_topLayout->addLayout(m_lowerLayout);
  setCentralWidget(m_centralWidget);
  m_audioDev = new QMediaDevices(this);
  m_audioSink.reset();
  m_playFlag = false;
  m_audioPlaybackTimer = new QTimer(this);
  connect(m_audioPlaybackTimer, &QTimer::timeout, this,
          &MainWindow::playbackTimerTimeoutHandler);
}

MainWindow::~MainWindow() {}

void MainWindow::createMenuBar() {
  m_menuBar = new QMenuBar(this);
  m_menuFile = new QMenu("&File");
  m_openAction = new QAction("&Open", this);
  m_openAction->setShortcut(QKeySequence::Open);
  m_quitAction = new QAction("&Quit", this);
  m_quitAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Q));
  m_menuFile->addAction(m_openAction);
  m_menuFile->addSeparator();
  m_menuFile->addAction(m_quitAction);
  m_menuBar->addMenu(m_menuFile);
  connect(m_openAction, &QAction::triggered, this,
          &MainWindow::openActionTriggeredHandler);
  connect(m_quitAction, &QAction::triggered, this,
          &MainWindow::quitActionTriggeredHandler);
  setMenuBar(m_menuBar);
}

void MainWindow::openActionTriggeredHandler() {
  if (m_sound) {
    delete m_sound;
  }
  qDebug() << "openAction triggered.";
  QString fname = QFileDialog::getOpenFileName(
      this, "Select audio file", "", "WAV files(*.wav);;All file(*.*)");
  m_sound = new Sound(fname.toStdString());
  m_waveView->init();
  m_waveView->drawWaveForm(m_sound);
  m_tfView->drawTFMap(m_sound, (Window)m_windowComboBox->currentIndex());
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
}

void MainWindow::quitActionTriggeredHandler() {
  qDebug() << "quitAction triggered.";
  close();
}

void MainWindow::playButtonClickedHandler() {
  if (m_sound == nullptr) {
    return;
  }
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
  if (m_audioSink.isNull()) {
    return;
  }
  m_audioSink->setVolume(val / 100.0);
}

void MainWindow::windowChangedHandler(int val) {
  if (m_sound == nullptr) {
    return;
  }
  m_tfView->drawTFMap(m_sound, (Window)val);
}