#ifndef SPIRE_SHUTTLEQTTYPES_HPP
#define SPIRE_SHUTTLEQTTYPES_HPP
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

namespace Beam {
namespace Serialization {
  template<>
  struct IsStructure<QString> : std::false_type {};

  template<>
  struct Send<QString> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, const char* name,
        const QString& value) const {
      shuttle.Shuttle(name, value.toStdString());
    }
  };

  template<>
  struct Receive<QString> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, const char* name,
        QString& value) const {
      std::string s;
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
      shuttle.Shuttle("buffer",
        Beam::IO::BufferFromString<Beam::IO::SharedBuffer>(buffer));
    }
  };

  template<>
  struct Receive<QByteArray> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, QByteArray& value,
        unsigned int version) const {
      auto buffer = Beam::IO::SharedBuffer();
      shuttle.Shuttle("buffer", buffer);
      value = QByteArray(buffer.GetData(), buffer.GetSize());
    }
  };

  template<>
  struct Send<QColor> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, const QColor& value,
        unsigned int version) const {
      shuttle.Shuttle("rgba", value.rgba());
    }
  };

  template<>
  struct Receive<QColor> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, QColor& value,
        unsigned int version) const {
      QRgb rgba;
      shuttle.Shuttle("rgba", rgba);
      value.setRgba(rgba);
    }
  };

  template<>
  struct Send<QFont> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, const QFont& value,
        unsigned int version) const {
      shuttle.Shuttle("family", value.family());
      shuttle.Shuttle("point_size", value.pointSize());
      shuttle.Shuttle("weight", value.weight());
      shuttle.Shuttle("italic", value.italic());
    }
  };

  template<>
  struct Receive<QFont> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, QFont& value,
        unsigned int version) const {
      QString family;
      shuttle.Shuttle("family", family);
      int pointSize;
      shuttle.Shuttle("point_size", pointSize);
      int weight;
      shuttle.Shuttle("weight", weight);
      bool italic;
      shuttle.Shuttle("italic", italic);
      value = QFont(family, pointSize, weight, italic);
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
    void operator ()(Shuttler& shuttle, QKeySequence& value,
        unsigned int version) const {
      std::string key;
      shuttle.Shuttle("key", key);
      value = QKeySequence(QString::fromStdString(key));
    }
  };

  template<>
  struct Send<QPoint> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, const QPoint& value,
        unsigned int version) const {
      shuttle.Shuttle("x", value.x());
      shuttle.Shuttle("y", value.y());
    }
  };

  template<>
  struct Receive<QPoint> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, QPoint& value,
        unsigned int version) const {
      int x;
      shuttle.Shuttle("x", x);
      int y;
      shuttle.Shuttle("y", y);
      value = QPoint(x, y);
    }
  };
}
}

#endif
