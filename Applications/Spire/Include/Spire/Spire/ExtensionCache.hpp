#ifndef SPIRE_EXTENSION_CACHE_HPP
#define SPIRE_EXTENSION_CACHE_HPP
#include <memory>
#include <unordered_map>
#include <QMetaObject>
#include <QObject>
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  /**
   * Keeps a cache of objects used to extend the functionality or observe a
   * QObject (or a descendent thereof). This function ensures that at most
   * one single extension object exists per QObject so that each extension
   * can be shared for the lifetime of the QObject.
   * @param object The <i>QObject</i> being extended.
   */
  template<typename E, typename T>
  std::shared_ptr<E> find_extension(T& object) {
    struct Entry {
      QMetaObject::Connection m_connection;
      std::weak_ptr<E> m_object;
    };
    static auto extensions = std::unordered_map<const T*, Entry>();
    auto i = extensions.find(&object);
    if(i != extensions.end()) {
      if(auto extension = i->second.m_object.lock()) {
        return extension;
      }
    }
    auto extension = std::shared_ptr<E>(new E(object), [&object] (auto* p) {
      if(!p) {
        return;
      } else {
        auto i = extensions.find(&object);
        if(i != extensions.end()) {
          QObject::disconnect(i->second.m_connection);
          extensions.erase(i);
        }
      }
      if constexpr(std::is_base_of_v<QObject, E>) {
        p->deleteLater();
      } else {
        delete p;
      }
    });
    auto connection = QObject::connect(&object, &QObject::destroyed,
      [] (auto object) {
        extensions.erase(static_cast<const T*>(object));
      });
    extensions.emplace(&object, Entry(connection, extension));
    return extension;
  }
}

#endif
