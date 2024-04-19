#pragma once

#include <QAction>
#include <QAudioSink>
#include <QComboBox>
#include <QGraphicsItemGroup>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QHBoxLayout>
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

class WaveView : public QGraphicsView {
 public:
  WaveView(int x, int y, int w, int h, QWidget *parent);
  void init();
  void drawWaveForm(Sound *sound);

 private:
  QGraphicsScene *m_scene;
};

class TFView : public QGraphicsView {
 public:
  TFView(int x, int y, int w, int h, QWidget *parent);
  ~TFView();
  void drawTFMap(Sound *sound, Window windowType);

 private:
  QGraphicsScene *m_scene;
  void double2rgb(const double x, unsigned char *r, unsigned char *g,
                  unsigned char *b);
  unsigned char *m_data;
};

class MainWindow : public QMainWindow {
  Q_OBJECT

 public:
  MainWindow(QWidget *parent = nullptr);
  ~MainWindow();
  void createMenuBar();

 public slots:
  void openActionTriggeredHandler();
  void quitActionTriggeredHandler();
  void playButtonClickedHandler();
  void streamStoppedHandler();
  void playbackTimerTimeoutHandler();
  void volSliderValueChangedHandler(int val);
  void windowChangedHandler(int val);

 private:
  QMenuBar *m_menuBar;
  QMenu *m_menuFile;
  QAction *m_openAction;
  QAction *m_quitAction;
  QWidget *m_centralWidget;
  QVBoxLayout *m_topLayout;
  QHBoxLayout *m_upperLayout;
  QVBoxLayout *m_pixmapLayout;
  TFView *m_tfView;
  QComboBox *m_windowComboBox;
  WaveView *m_waveView;
  QHBoxLayout *m_lowerLayout;
  QSlider *m_volSlider;
  QPushButton *m_playButton;
  Sound *m_sound = nullptr;
  QMediaDevices *m_audioDev;
  QTimer *m_audioPlaybackTimer;
  QScopedPointer<AudioStream> m_audioStream;
  QScopedPointer<QAudioSink> m_audioSink;
  QIODevice *m_audioIO;
  bool m_playFlag;
};
