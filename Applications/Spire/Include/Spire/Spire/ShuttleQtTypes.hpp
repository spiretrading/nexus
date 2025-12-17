#ifndef SPIRE_SHUTTLE_QT_TYPES_HPP
#define SPIRE_SHUTTLE_QT_TYPES_HPP
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Serialization/Receiver.hpp>
#include <Beam/Serialization/Sender.hpp>
#include <QByteArray>
#include <QColor>
#include <QFont>
#include <QKeySequence>
#include <QPoint>
#include <QString>
#include "Spire/LegacyUI/LegacyUI.hpp"
#include "Spire/Spire/Dimensions.hpp"

namespace Beam {
  template<>
  constexpr auto is_structure<QString> = false;

  template<>
  struct Send<QString> {
    template<IsSender S>
    void operator ()(S& sender, const char* name, const QString& value) const {
      sender.send(name, value.toStdString());
    }
  };

  template<>
  struct Receive<QString> {
    template<IsReceiver R>
    void operator ()(R& receiver, const char* name, QString& value) const {
      value = QString::fromStdString(receive<std::string>(receiver, name));
    }
  };

  template<>
  struct Send<QByteArray> {
    template<IsSender S>
    void operator ()(
        S& sender, const QByteArray& value, unsigned int version) const {
      auto buffer = std::string(value.data(), value.size());
      sender.send("buffer", buffer);
    }
  };

  template<>
  struct Receive<QByteArray> {
    template<IsReceiver R>
    void operator ()(
        R& receiver, QByteArray& value, unsigned int version) const {
      auto buffer = receive<std::string>(receiver, "buffer");
      value = QByteArray(buffer.c_str(), buffer.size());
    }
  };

  template<>
  struct Send<QColor> {
    template<IsSender S>
    void operator ()(
        S& sender, const QColor& value, unsigned int version) const {
      sender.send("rgba", value.rgba());
    }
  };

  template<>
  struct Receive<QColor> {
    template<IsReceiver R>
    void operator ()(R& receiver, QColor& value, unsigned int version) const {
      value.setRgba(receive<QRgb>(receiver, "rgba"));
    }
  };

  template<>
  struct Send<QFont> {
    template<IsSender S>
    void operator ()(
        S& sender, const QFont& value, unsigned int version) const {
      sender.send("family", value.family());
      sender.send("point_size", value.pointSize());
      sender.send("weight", value.weight());
      sender.send("italic", value.italic());
      auto pixel_size = value.pixelSize();
      sender.send("pixel_size", Spire::unscale_width(pixel_size));
    }
  };

  template<>
  struct Receive<QFont> {
    template<IsReceiver R>
    void operator ()(R& receiver, QFont& value, unsigned int version) const {
      auto family = receive<QString>(receiver, "family");
      auto point_size = receive<int>(receiver, "point_size");
      auto weight = receive<int>(receiver, "weight");
      auto italic = receive<bool>(receiver, "italic");
      value = QFont(family, point_size, weight, italic);
      auto pixel_size = receive<int>(receiver, "pixel_size");
      value.setPixelSize(Spire::scale_width(pixel_size));
    }
  };

  template<>
  struct Send<QKeySequence> {
    template<IsSender S>
    void operator ()(
        S& sender, const QKeySequence& value, unsigned int version) const {
      sender.send("key", value.toString().toStdString());
    }
  };

  template<>
  struct Receive<QKeySequence> {
    template<IsReceiver R>
    void operator ()(
        R& receiver, QKeySequence& value, unsigned int version) const {
      auto key = receive<std::string>(receiver, "key");
      value = QKeySequence(QString::fromStdString(key));
    }
  };

  template<>
  struct Send<QPoint> {
    template<IsSender S>
    void operator ()(
        S& sender, const QPoint& value, unsigned int version) const {
      sender.send("x", value.x());
      sender.send("y", value.y());
    }
  };

  template<>
  struct Receive<QPoint> {
    template<IsReceiver R>
    void operator ()(R& receiver, QPoint& value, unsigned int version) const {
      auto x = receive<int>(receiver, "x");
      auto y = receive<int>(receiver, "y");
      value = QPoint(x, y);
    }
  };
}

#endif
