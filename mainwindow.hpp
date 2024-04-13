#pragma once

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QMainWindow>
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
  void drawTFMap(Sound *sound);

 private:
  QGraphicsScene *m_scene;
};

class MainWindow : public QMainWindow {
  Q_OBJECT

 public:
  MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

 private:
  QWidget *m_centralWidget;
  QVBoxLayout *m_topLayout;
  TFView *m_tfView;
  WaveView *m_waveView;
  Sound *m_sound;
};
