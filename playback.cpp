#include "playback.hpp"

#include <QDebug>
#include <QtMath>
#include <climits>

AudioStream::AudioStream(Sound *sound) {
  m_sound = sound;
  readAudioData();
}

void AudioStream::readAudioData() {
  double *x = m_sound->x();
  int nSamples = m_sound->nSamples();
  m_buf.resize(nSamples * 2);
  unsigned char *buf = reinterpret_cast<unsigned char *>(m_buf.data());
  for (int n = 0; n < nSamples; n++) {
    *reinterpret_cast<qint16 *>(buf) =
        static_cast<qint16>((x[n] + 1.0) * SHRT_MAX + SHRT_MIN + 0.5);
    buf += 2;
  }
}

qint64 AudioStream::readData(char *data, qint64 len) {
  qint64 total = 0;
  if (!m_buf.isEmpty()) {
    while (len - total > 0) {
      const qint64 chunk = qMin((m_buf.size() - m_pos), len - total);
      if (m_pos + chunk < m_buf.size()) {
        memcpy(data + total, m_buf.constData() + m_pos, chunk);
        m_pos = (m_pos + chunk);
        total += chunk;
      } else {
        m_pos = 0;
        emit stopped();
      }
    }
  }
  return total;
}
