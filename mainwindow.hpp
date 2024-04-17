#pragma once

#include <QAudioSink>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QHBoxLayout>
#include <QMainWindow>
#include <QMediaDevices>
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
  void drawWaveForm(Sound *sound);

 private:
  QGraphicsScene *m_scene;
};

class TFView : public QGraphicsView {
 public:
  TFView(int x, int y, int w, int h, QWidget *parent);
  ~TFView();
  void drawTFMap(Sound *sound);

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

 public slots:
  void playButtonClickedHandler();
  void streamStoppedHandler();
  void playbackTimerTimeoutHandler();
  void volSliderValueChangedHandler(int val);

 private:
  QWidget *m_centralWidget;
  QVBoxLayout *m_topLayout;
  TFView *m_tfView;
  WaveView *m_waveView;
  QWidget *m_playbackWidget;
  QHBoxLayout *m_playbackLayout;
  QSlider *m_volSlider;
  QPushButton *m_playButton;
  Sound *m_sound;
  QMediaDevices *m_audioDev;
  QTimer *m_audioPlaybackTimer;
  QScopedPointer<AudioStream> m_audioStream;
  QScopedPointer<QAudioSink> m_audioSink;
  QIODevice *m_audioIO;
  bool m_playFlag;
};
