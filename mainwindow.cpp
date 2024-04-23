#include "mainwindow.hpp"

#include <QFileDialog>
#include <QImage>
#include <QPixmap>

#include "fft.hpp"
#include "playback.hpp"

using namespace std;

WaveScene::WaveScene(int x, int y, int w, int h, MainWindow *parent)
    : QGraphicsScene(x, y, w, h, parent) {
  m_parent = parent;
}

void WaveScene::mouseMoveEvent(QGraphicsSceneMouseEvent *e) {
  if (!m_parent->sound()) {
    return;
  }
  double duration = m_parent->sound()->duration();
  int x = e->scenePos().x();
  double time = x / width() * duration;
  m_parent->timeLabel()->setText(QString::number(time));
}

WaveView::WaveView(int x, int y, int w, int h, MainWindow *parent)
    : QGraphicsView(parent) {
  m_scene = new WaveScene(x, y, w, h, parent);
  m_scene->setBackgroundBrush(QColor(0, 0, 80));
  init();
  setMouseTracking(true);
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

TFScene::TFScene(int x, int y, int w, int h, MainWindow *parent)
    : QGraphicsScene(x, y, w, h, parent) {
  m_parent = parent;
}

TFScene::~TFScene() {
  if (m_currentStreamPosLine) {
    delete m_currentStreamPosLine;
  }
  if (m_ticks) {
    delete m_ticks;
  }
}

void TFScene::mouseMoveEvent(QGraphicsSceneMouseEvent *e) {
  if (!m_parent->sound()) {
    return;
  }
  int fs = m_parent->sound()->fs();
  double duration = m_parent->sound()->duration();
  int y = e->scenePos().y();
  int x = e->scenePos().x();
  double freq = (height() - y) / height() * fs / 2.0;
  double time = x / width() * duration;
  m_parent->freqLabel()->setText(QString::number(freq));
  m_parent->timeLabel()->setText(QString::number(time));
}

void TFScene::drawFreqTicks(TFView::FreqScale freqScale) {
  if (m_ticks) {
    removeItem(m_ticks);
    delete m_ticks;
  }
  m_ticks = new QGraphicsItemGroup();
  int fs = m_parent->sound()->fs();
  int nFFT = m_parent->sound()->fft()->nFFT();
  int h = height();
  double fCur = 1.0;
  double fStep;
  double yPos;
  static double erbHi = TFView::hz2erb(fs / 2.0);
  static double cbrHi = TFView::hz2bark(fs / 2.0);
  static double mHi = TFView::hz2mel(fs / 2.0);
  switch (freqScale) {
    case TFView::FreqScale::Linear:
      for (double f = 0; f < fs / 2.0; f += 100.0) {
        yPos = h - f / (fs / 2.0) * h;
        if (!((int)f % 5000)) {
          m_ticks->addToGroup(addLine(0, yPos, 10.0, yPos, QColor(Qt::gray)));
          QGraphicsTextItem *text = addText(QString::number(f));
          text->setDefaultTextColor(QColor(Qt::gray));
          text->setPos(10.0, yPos);
          m_ticks->addToGroup(text);
        } else if (!(int)f % 1000) {
          m_ticks->addToGroup(addLine(0, yPos, 5.0, yPos, QColor(Qt::gray)));
        } else {
          m_ticks->addToGroup(addLine(0, yPos, 2.0, yPos, QColor(Qt::gray)));
        }
      }
      break;
    case TFView::FreqScale::Log:
      for (int i = 0; i < log10((double)fs / 2.0); i++) {
        for (int j = 1; j < 10; j++) {
          double f = j * pow(10.0, i);
          if (f > fs / 2.0) {
            break;
          }
          double kLinear = f / fs * nFFT;
          double kLog = log(kLinear + 1) / log(nFFT / 2.0);
          yPos = h - kLog * h;
          if (j == 1) {
            m_ticks->addToGroup(addLine(0, yPos, 10.0, yPos, QColor(Qt::gray)));
            QGraphicsTextItem *text = addText(QString::number(f));
            text->setDefaultTextColor(QColor(Qt::gray));
            text->setPos(10.0, yPos);
          } else {
            m_ticks->addToGroup(addLine(0, yPos, 5.0, yPos, QColor(Qt::gray)));
          }
        }
      }
      break;
    case TFView::FreqScale::ERB:
      for (int i = 0; i < log10((double)fs / 2.0); i++) {
        for (int j = 1; j < 10; j++) {
          double f = j * pow(10.0, i);
          if (f > fs / 2.0) {
            break;
          }
          double erbs = TFView::hz2erb(f);
          yPos = h - erbs / erbHi * h;
          if (j == 1) {
            m_ticks->addToGroup(addLine(0, yPos, 10.0, yPos, QColor(Qt::gray)));
            QGraphicsTextItem *text = addText(QString::number(f));
            text->setDefaultTextColor(QColor(Qt::gray));
            text->setPos(10.0, yPos);
          } else {
            m_ticks->addToGroup(addLine(0, yPos, 5.0, yPos, QColor(Qt::gray)));
          }
        }
      }
      break;
    case TFView::FreqScale::Bark:
      for (int i = 0; i < log10((double)fs / 2.0); i++) {
        for (int j = 1; j < 10; j++) {
          double f = j * pow(10.0, i);
          if (f > fs / 2.0) {
            break;
          }
          double cbr = TFView::hz2bark(f);
          yPos = h - cbr / cbrHi * h;
          if (j == 1) {
            m_ticks->addToGroup(addLine(0, yPos, 10.0, yPos, QColor(Qt::gray)));
            QGraphicsTextItem *text = addText(QString::number(f));
            text->setDefaultTextColor(QColor(Qt::gray));
            text->setPos(10.0, yPos);
          } else {
            m_ticks->addToGroup(addLine(0, yPos, 5.0, yPos, QColor(Qt::gray)));
          }
        }
      }
      break;
    case TFView::FreqScale::Mel:
      for (int i = 0; i < log10((double)fs / 2.0); i++) {
        for (int j = 1; j < 10; j++) {
          double f = j * pow(10.0, i);
          if (f > fs / 2.0) {
            break;
          }
          double m = TFView::hz2mel(f);
          yPos = h - m / mHi * h;
          if (j == 1) {
            m_ticks->addToGroup(addLine(0, yPos, 10.0, yPos, QColor(Qt::gray)));
            QGraphicsTextItem *text = addText(QString::number(f));
            text->setDefaultTextColor(QColor(Qt::gray));
            text->setPos(10.0, yPos);
          } else {
            m_ticks->addToGroup(addLine(0, yPos, 5.0, yPos, QColor(Qt::gray)));
          }
        }
      }
      break;
    default:
      break;
  }
  addItem(m_ticks);
}

TFView::TFView(int x, int y, int w, int h, MainWindow *parent)
    : QGraphicsView(parent) {
  m_scene = new TFScene(x, y, w, h, parent);
  m_scene->setBackgroundBrush(QColor("black"));
  m_data = new unsigned char[w * h * 3];
  setMouseTracking(true);
  setScene(m_scene);
}

TFView::~TFView() {
  delete[] m_data;
  if (m_scaledIdx) {
    delete[] m_scaledIdx;
  }
}

void TFView::drawTFMap(Window::WindowType windowType, int windowSize) {
  int w = m_scene->width();
  int h = m_scene->height();
  int hopSize = m_parentSound->nSamples() / w;
  if (m_flagModified) {
    m_parentSound->stft(hopSize, windowType, windowSize);
    m_flagModified = false;
  }
  complex<double> **spec = m_parentSound->spec();
  double specMax = m_parentSound->specMax();
  double specMin = m_parentSound->specMin();
  double upper_dB, lower_dB;
  upper_dB = 20.0 * log10(specMax);
  // lower_dB = 20.0 * log10(specMin);
  lower_dB = -120.0;
  for (int x = 0; x < w; x++) {
    for (int y = 0; y < h; y++) {
      double2rgb((20.0 * log10(abs(spec[x][m_scaledIdx[y]])) - lower_dB) /
                     (upper_dB - lower_dB),
                 m_data + ((h - 1 - y) * w + x) * 3,       // R
                 m_data + ((h - 1 - y) * w + x) * 3 + 1,   // G
                 m_data + ((h - 1 - y) * w + x) * 3 + 2);  // B
    }
  }
  QImage img(m_data, w, h, QImage::Format_RGB888);
  QPixmap pixmap = QPixmap::fromImage(img);
  m_scene->addPixmap(pixmap);
  m_scene->drawFreqTicks(m_freqScale);
}

void TFView::setFreqScale(FreqScale type) {
  int nFFT = m_parentSound->fft()->nFFT();
  int fs = m_parentSound->fs();
  m_freqScale = type;
  switch (type) {
    case FreqScale::Linear:
      for (int k = 0; k < nFFT / 2; k++) {
        m_scaledIdx[k] = k;
      }
      break;
    case FreqScale::Log:
      for (int k = 0; k < nFFT / 2; k++) {
        m_scaledIdx[k] = (int)pow(nFFT / 2.0, (double)k / (nFFT / 2.0));
      }
      break;
    case FreqScale::ERB:
      for (int k = 0; k < nFFT / 2; k++) {
        m_scaledIdx[k] =
            (int)(erb2hz((double)k / (nFFT / 2.0) * (hz2erb(22050.0))) / fs *
                  nFFT);
      }
      break;
    case FreqScale::Bark:
      for (int k = 0; k < nFFT / 2; k++) {
        m_scaledIdx[k] =
            (int)(bark2hz((double)k / (nFFT / 2.0) * (hz2bark(22050.0))) / fs *
                  nFFT);
      }
      break;
    case FreqScale::Mel:
      for (int k = 0; k < nFFT / 2; k++) {
        m_scaledIdx[k] =
            (int)(mel2hz((double)k / (nFFT / 2.0) * (hz2mel(22050.0))) / fs *
                  nFFT);
      }
      break;
    default:
      qDebug() << "Unsupported frequency scale type.";
      qDebug() << "Force set to linear.";
      for (int k = 0; k < nFFT / 2; k++) {
        m_scaledIdx[k] = k;
      }
      break;
  }
  m_scene->drawFreqTicks(type);
}
void TFView::genFreqIdx(FreqScale scaleType) {
  if (!m_parentSound) {
    return;
  }
  int nFFT = m_parentSound->fft()->nFFT();
  m_scaledIdx = new int[nFFT / 2];
  setFreqScale(scaleType);
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
  m_freqLabel = new QLabel("-.-", this);
  m_HzLabel = new QLabel(" Hz", this);
  m_timeLabel = new QLabel("-.-", this);
  m_secLabel = new QLabel(" sec", this);
  m_tfView = new TFView(0, 0, 1200, 1024, this);
  m_waveView = new WaveView(0, 0, 1200, 100, this);
  m_pixmapLayout->addWidget(m_tfView);
  m_pixmapLayout->addWidget(m_waveView);
  m_upperLayout->addLayout(m_pixmapLayout);
  m_tfControllLayout = new QVBoxLayout();
  m_windowTypeComboBox = new QComboBox(this);
  for (int w = 0; w < (int)Window::NumWindow; w++) {
    switch ((Window::WindowType)w) {
      case Window::WindowType::Gaussian:
        m_windowTypeComboBox->addItem("Gauusian");
        break;
      case Window::WindowType::Hann:
        m_windowTypeComboBox->addItem("Hann");
        break;
      case Window::WindowType::Hamming:
        m_windowTypeComboBox->addItem("Hamming");
        break;
      case Window::WindowType::Rect:
        m_windowTypeComboBox->addItem("Rect");
        break;
      default:
        break;
    }
  }

  m_windowTypeComboBox->setSizePolicy(
      QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
  connect(m_windowTypeComboBox, &QComboBox::currentIndexChanged, this,
          &MainWindow::windowTypeChangedHandler);
  m_windowSizeComboBox = new QComboBox(this);
  m_windowSizeComboBox->addItems(m_windowSizeList);
  m_freqScaleComboBox = new QComboBox(this);
  for (int i = 0; i < (int)TFView::FreqScale::NumFreqScale; i++) {
    switch ((TFView::FreqScale)i) {
      case TFView::FreqScale::Linear:
        m_freqScaleComboBox->addItem("Linear");
        break;
      case TFView::FreqScale::Log:
        m_freqScaleComboBox->addItem("Log");
        break;
      case TFView::FreqScale::ERB:
        m_freqScaleComboBox->addItem("ERB");
        break;
      case TFView::FreqScale::Bark:
        m_freqScaleComboBox->addItem("Bark");
        break;
      case TFView::FreqScale::Mel:
        m_freqScaleComboBox->addItem("Mel");
        break;
      default:
        break;
    }
  }
  m_tfControllLayout->addWidget(m_windowTypeComboBox);
  m_tfControllLayout->addWidget(m_windowSizeComboBox);
  m_tfControllLayout->addWidget(m_freqScaleComboBox);
  connect(m_windowSizeComboBox, &QComboBox::currentIndexChanged, this,
          &MainWindow::windowSizeChangedHandler);
  connect(m_freqScaleComboBox, &QComboBox::currentIndexChanged, this,
          &MainWindow::freqScaleChangedHandler);
  m_tfControllLayout->addStretch(0);
  m_upperLayout->addLayout(m_tfControllLayout);
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
  m_lowerLayout->addWidget(m_freqLabel);
  m_lowerLayout->addWidget(m_HzLabel);
  m_lowerLayout->addStretch();
  m_lowerLayout->addWidget(m_timeLabel);
  m_lowerLayout->addWidget(m_secLabel);
  m_lowerLayout->addStretch();
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
  QString fname = QFileDialog::getOpenFileName(
      this, "Select audio file", "", "WAV files(*.wav);;All file(*.*)");
  m_sound = new Sound(fname.toStdString(), 1024,
                      (Window::WindowType)m_windowTypeComboBox->currentIndex());
  m_waveView->init();
  m_waveView->drawWaveForm(m_sound);
  m_tfView->setParentSound(m_sound);
  m_tfView->genFreqIdx((TFView::FreqScale)m_freqScaleComboBox->currentIndex());
  m_tfView->setFreqBounds(20.0, m_sound->fs() / 2.0);
  m_tfView->setFlagModified();
  m_tfView->drawTFMap(
      (Window::WindowType)m_windowTypeComboBox->currentIndex(),
      m_windowSizeList[m_windowSizeComboBox->currentIndex()].toInt());
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

void MainWindow::quitActionTriggeredHandler() { close(); }

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

void MainWindow::windowTypeChangedHandler(int val) {
  if (m_sound == nullptr) {
    return;
  }
  m_tfView->setFlagModified();
  m_tfView->drawTFMap(
      (Window::WindowType)val,
      m_windowSizeList[m_windowSizeComboBox->currentIndex()].toInt());
}

void MainWindow::windowSizeChangedHandler(int val) {
  if (m_sound == nullptr) {
    return;
  }
  m_tfView->setFlagModified();
  m_tfView->drawTFMap((Window::WindowType)m_windowTypeComboBox->currentIndex(),
                      m_windowSizeList[val].toInt());
}

void MainWindow::freqScaleChangedHandler(int val) {
  if (m_sound == nullptr) {
    return;
  }
  m_tfView->setFreqScale((TFView::FreqScale)val);
  m_tfView->drawTFMap(
      (Window::WindowType)val,
      m_windowSizeList[m_windowSizeComboBox->currentIndex()].toInt());
}