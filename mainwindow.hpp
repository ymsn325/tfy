#pragma once

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QMainWindow>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

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

 private:
  QWidget *m_centralWidget;
  QVBoxLayout *m_topLayout;
  TFView *m_tfView;
  WaveView *m_waveView;
  QPushButton *m_playButton;
  Sound *m_sound;
  bool m_playFlag;
};
