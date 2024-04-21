#pragma once

#include <QAction>
#include <QAudioSink>
#include <QComboBox>
#include <QGraphicsItemGroup>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsView>
#include <QHBoxLayout>
#include <QLabel>
#include <QMainWindow>
#include <QMediaDevices>
#include <QMenu>
#include <QMenuBar>
#include <QPushButton>
#include <QScopedPointer>
#include <QSlider>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>

#include "playback.hpp"
#include "sound.hpp"

class MainWindow;
class WaveScene : public QGraphicsScene {
 public:
  WaveScene(int x, int y, int w, int h, MainWindow *parent);
  void setCurrentStreamPosLine(double x);
  QGraphicsItem *currentStreamPosLine() { return m_currentStreamPosLine; }
  void mouseMoveEvent(QGraphicsSceneMouseEvent *e) override;

 private:
  MainWindow *m_parent;
  QGraphicsItem *m_currentStreamPosLine = nullptr;
};

class WaveView : public QGraphicsView {
 public:
  WaveView(int x, int y, int w, int h, MainWindow *parent);
  WaveScene *scene() { return m_scene; }
  void init();
  void drawWaveForm(Sound *sound);

 private:
  WaveScene *m_scene;
};

class TFScene : public QGraphicsScene {
 public:
  TFScene(int x, int y, int w, int h, MainWindow *parent);
  void setCurrentStreamPosLine(double x);
  void mouseMoveEvent(QGraphicsSceneMouseEvent *e) override;

 private:
  MainWindow *m_parent;
  QGraphicsItem *m_currentStreamPosLine = nullptr;
};

class TFView : public QGraphicsView {
 public:
  enum FreqScale { Linear, Log, ERB, Bark, Mel, NumFreqScale };
  TFView(int x, int y, int w, int h, MainWindow *parent);
  ~TFView();
  TFScene *scene() { return m_scene; }
  void setParentSound(Sound *sound) { m_parentSound = sound; }
  void drawTFMap(Window::WindowType windowType, int windowSize);
  void setFreqScale(FreqScale type);
  void setFreqBounds(double lo, double hi) {
    m_freqLo = lo;
    m_freqHi = hi;
  }
  void setFlagModified() { m_flagModified = true; }
  void genFreqIdx(FreqScale scaleType);

 private:
  void double2rgb(const double x, unsigned char *r, unsigned char *g,
                  unsigned char *b);
  unsigned char *m_data;
  Sound *m_parentSound;
  TFScene *m_scene;
  FreqScale m_freqScale = Linear;
  int *m_scaledIdx = nullptr;
  bool m_flagModified;
  double m_freqLo;
  double m_freqHi;
};

class MainWindow : public QMainWindow {
  Q_OBJECT

 public:
  MainWindow(QWidget *parent = nullptr);
  ~MainWindow();
  QLabel *freqLabel() { return m_freqLabel; }
  QLabel *timeLabel() { return m_timeLabel; }
  Sound *sound() { return m_sound; }

 public slots:
  void openActionTriggeredHandler();
  void quitActionTriggeredHandler();
  void playButtonClickedHandler();
  void streamStoppedHandler();
  void playbackTimerTimeoutHandler();
  void volSliderValueChangedHandler(int val);
  void windowTypeChangedHandler(int val);
  void windowSizeChangedHandler(int val);
  void freqScaleChangedHandler(int val);

 private:
  void createMenuBar();
  QMenuBar *m_menuBar;
  QMenu *m_menuFile;
  QAction *m_openAction;
  QAction *m_quitAction;
  QWidget *m_centralWidget;
  QVBoxLayout *m_topLayout;
  QHBoxLayout *m_upperLayout;
  QVBoxLayout *m_pixmapLayout;
  TFView *m_tfView;
  WaveView *m_waveView;
  QVBoxLayout *m_tfControllLayout;
  QComboBox *m_windowTypeComboBox;
  QComboBox *m_windowSizeComboBox;
  QComboBox *m_freqScaleComboBox;
  QHBoxLayout *m_lowerLayout;
  QLabel *m_freqLabel;
  QLabel *m_HzLabel;
  QLabel *m_timeLabel;
  QLabel *m_secLabel;
  QSlider *m_volSlider;
  QPushButton *m_playButton;
  Sound *m_sound = nullptr;
  QMediaDevices *m_audioDev;
  QTimer *m_audioPlaybackTimer;
  QScopedPointer<AudioStream> m_audioStream;
  QScopedPointer<QAudioSink> m_audioSink;
  QIODevice *m_audioIO;
  bool m_playFlag;
  QStringList m_windowSizeList = {"2048", "1024", "512", "256",
                                  "128",  "64",   "32"};
};
