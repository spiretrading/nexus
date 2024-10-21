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

namespace Beam::Serialization {
  template<>
  struct IsStructure<QString> : std::false_type {};

  template<>
  struct Send<QString> {
    template<typename Shuttler>
    void operator ()(
        Shuttler& shuttle, const char* name, const QString& value) const {
      shuttle.Shuttle(name, value.toStdString());
    }
  };

  template<>
  struct Receive<QString> {
    template<typename Shuttler>
    void operator ()(
        Shuttler& shuttle, const char* name, QString& value) const {
      auto s = std::string();
      shuttle.Shuttle(name, s);
      value = QString::fromStdString(s);
    }
  };

  template<>
  struct Send<QByteArray> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, const QByteArray& value,
        unsigned int version) const {
      auto buffer = std::string(value.data(), value.size());
      shuttle.Shuttle("buffer", IO::BufferFromString<IO::SharedBuffer>(buffer));
    }
  };

  template<>
  struct Receive<QByteArray> {
    template<typename Shuttler>
    void operator ()(
        Shuttler& shuttle, QByteArray& value, unsigned int version) const {
      auto buffer = IO::SharedBuffer();
      shuttle.Shuttle("buffer", buffer);
      value = QByteArray(buffer.GetData(), buffer.GetSize());
    }
  };

  template<>
  struct Send<QColor> {
    template<typename Shuttler>
    void operator ()(
        Shuttler& shuttle, const QColor& value, unsigned int version) const {
      shuttle.Shuttle("rgba", value.rgba());
    }
  };

  template<>
  struct Receive<QColor> {
    template<typename Shuttler>
    void operator ()(
        Shuttler& shuttle, QColor& value, unsigned int version) const {
      auto rgba = QRgb();
      shuttle.Shuttle("rgba", rgba);
      value.setRgba(rgba);
    }
  };

  template<>
  struct Send<QFont> {
    template<typename Shuttler>
    void operator ()(
        Shuttler& shuttle, const QFont& value, unsigned int version) const {
      shuttle.Shuttle("family", value.family());
      shuttle.Shuttle("point_size", value.pointSize());
      shuttle.Shuttle("weight", value.weight());
      shuttle.Shuttle("italic", value.italic());
      if(value.pointSize() == -1) {
        shuttle.Shuttle("pixel_size", Spire::unscale_width(value.pixelSize()));
      }
    }
  };

  template<>
  struct Receive<QFont> {
    template<typename Shuttler>
    void operator ()(
        Shuttler& shuttle, QFont& value, unsigned int version) const {
      auto family = QString();
      shuttle.Shuttle("family", family);
      auto point_size = int();
      shuttle.Shuttle("point_size", point_size);
      auto weight = int();
      shuttle.Shuttle("weight", weight);
      auto italic = bool();
      shuttle.Shuttle("italic", italic);
      value = QFont(family, point_size, weight, italic);
      if(point_size == -1) {
        auto pixel_size = int();
        shuttle.Shuttle("pixel_size", pixel_size);
        value.setPixelSize(Spire::scale_width(pixel_size));
      }
    }
  };

  template<>
  struct Send<QKeySequence> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, const QKeySequence& value,
        unsigned int version) const {
      shuttle.Shuttle("key", value.toString().toStdString());
    }
  };

  template<>
  struct Receive<QKeySequence> {
    template<typename Shuttler>
    void operator ()(
        Shuttler& shuttle, QKeySequence& value, unsigned int version) const {
      auto key = std::string();
      shuttle.Shuttle("key", key);
      value = QKeySequence(QString::fromStdString(key));
    }
  };

  template<>
  struct Send<QPoint> {
    template<typename Shuttler>
    void operator ()(
        Shuttler& shuttle, const QPoint& value, unsigned int version) const {
      shuttle.Shuttle("x", value.x());
      shuttle.Shuttle("y", value.y());
    }
  };

  template<>
  struct Receive<QPoint> {
    template<typename Shuttler>
    void operator ()(
        Shuttler& shuttle, QPoint& value, unsigned int version) const {
      auto x = int();
      shuttle.Shuttle("x", x);
      auto y = int();
      shuttle.Shuttle("y", y);
      value = QPoint(x, y);
    }
  };
}

#endif
