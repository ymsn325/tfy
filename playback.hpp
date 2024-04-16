#pragma once

#include <QByteArray>
#include <QIODevice>

#include "sound.hpp"

class AudioStream : public QIODevice {
  Q_OBJECT
 signals:
  void stopped();

 public:
  AudioStream(Sound *sound);
  void start() { open(QIODevice::ReadOnly); }
  void stop() {
    m_pos = 0;
    close();
  }
  void readAudioData();
  qint64 readData(char *data, qint64 maxlen) override;
  qint64 writeData(const char *data, qint64 len) override {
    Q_UNUSED(data);
    Q_UNUSED(len)
    return 0;
  }
  qint64 bytesAvailable() const override {
    return m_buf.size() + QIODevice::bytesAvailable();
  }
  qint64 size() const override { return m_buf.size(); }
  void setPos(int pos) { m_pos = pos; }

 private:
  int m_pos = 0;
  QByteArray m_buf;
  Sound *m_sound;
};
