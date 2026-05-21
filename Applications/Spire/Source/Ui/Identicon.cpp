#include "Spire/Ui/Identicon.hpp"
#include <array>
#include <QColor>
#include <QCryptographicHash>
#include <QPainter>

using namespace Beam;
using namespace Spire;

namespace {
  constexpr auto GRID_SIZE = 5;
  constexpr auto CENTER = GRID_SIZE / 2;
  constexpr auto DISTANCE_MASKS =
    std::array<unsigned char, 3>{0x07, 0x03, 0x01};

  const auto COLORS = std::array{
    QColor(0xBF4040), QColor(0xBF406A), QColor(0xBF40BF), QColor(0x9540BF),
    QColor(0x6A40BF), QColor(0x4040BF), QColor(0x406ABF), QColor(0x4095BF),
    QColor(0x40BFBF), QColor(0x40BF95), QColor(0x40BF6A), QColor(0x6ABF40),
    QColor(0x95BF40), QColor(0xBFBF40), QColor(0xBF9540), QColor(0xBF6A40)};
}

QImage Spire::make_identicon(const DirectoryEntry& account, QSize size) {
  auto hash = QCryptographicHash::hash(
    QByteArray::number(account.m_id), QCryptographicHash::Md5);
  auto color =
    COLORS[static_cast<unsigned char>(hash.back()) % COLORS.size()];
  auto image = QImage(size, QImage::Format_ARGB32);
  image.fill(Qt::transparent);
  auto painter = QPainter(&image);
  painter.setRenderHint(QPainter::Antialiasing);
  painter.setPen(Qt::NoPen);
  auto cell_width = size.width() / static_cast<double>(GRID_SIZE);
  auto cell_height = size.height() / static_cast<double>(GRID_SIZE);
  for(auto i = 0; i < GRID_SIZE; ++i) {
    for(auto j = 0; j <= CENTER; ++j) {
      auto byte = static_cast<unsigned char>(hash[(CENTER + 1) * i + j]);
      auto distance = std::max(CENTER - j, std::abs(i - CENTER));
      if((byte & DISTANCE_MASKS[distance]) != 0) {
        painter.fillRect(QRectF(cell_width * j, cell_height * i,
          cell_width, cell_height), color);
        if(j != CENTER) {
          painter.fillRect(QRectF(cell_width * (GRID_SIZE - 1 - j),
            cell_height * i, cell_width, cell_height), color);
        }
      }
    }
  }
  return image;
}
