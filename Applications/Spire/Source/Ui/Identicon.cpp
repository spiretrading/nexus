#include "Spire/Ui/Identicon.hpp"
#include <array>
#include <QColor>
#include <QCryptographicHash>
#include <QPainter>

using namespace Beam;
using namespace Spire;

namespace {
  const auto& COLORS() {
    static const auto colors = std::array<QColor, 16>{
      QColor(0xBF4040), QColor(0xBF406A), QColor(0xBF40BF), QColor(0x9540BF),
      QColor(0x6A40BF), QColor(0x4040BF), QColor(0x406ABF), QColor(0x4095BF),
      QColor(0x40BFBF), QColor(0x40BF95), QColor(0x40BF6A), QColor(0x6ABF40),
      QColor(0x95BF40), QColor(0xBFBF40), QColor(0xBF9540), QColor(0xBF6A40)};
    return colors;
  }

  constexpr auto DISTANCE_MASKS =
    std::array<unsigned char, 3>{0x07, 0x03, 0x01};
}

QImage Spire::make_identicon(const DirectoryEntry& account, QSize size) {
  auto hash = QCryptographicHash::hash(
    QByteArray::number(account.m_id), QCryptographicHash::Md5);
  auto color =
    COLORS()[static_cast<unsigned char>(hash[15]) % COLORS().size()];
  auto image = QImage(size, QImage::Format_ARGB32);
  image.fill(Qt::transparent);
  auto painter = QPainter(&image);
  painter.setRenderHint(QPainter::Antialiasing);
  painter.setPen(Qt::NoPen);
  auto cell_width = size.width() / 5.0;
  auto cell_height = size.height() / 5.0;
  for(auto i = 0; i < 5; ++i) {
    for(auto j = 0; j < 3; ++j) {
      auto byte = static_cast<unsigned char>(hash[i * 3 + j]);
      auto distance = std::max(2 - j, std::abs(i - 2));
      if((byte & DISTANCE_MASKS[distance]) != 0) {
        painter.fillRect(QRectF(j * cell_width, i * cell_height,
          cell_width, cell_height), color);
        if(j != 2) {
          painter.fillRect(QRectF((4 - j) * cell_width, i * cell_height,
            cell_width, cell_height), color);
        }
      }
    }
  }
  return image;
}
