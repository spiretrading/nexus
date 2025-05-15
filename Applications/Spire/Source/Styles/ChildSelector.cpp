#include "Spire/Styles/ChildSelector.hpp"
#include <unordered_map>
#include <Beam/SignalHandling/ConnectionGroup.hpp>
#include <boost/functional/hash.hpp>
#include <QChildEvent>
#include <QWidget>
#include "Spire/Styles/CombinatorSelector.hpp"
#include "Spire/Styles/Stylist.hpp"

using namespace Beam;
using namespace Beam::SignalHandling;
using namespace boost;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  void insert_proxies(
      const Stylist& root, std::unordered_set<const Stylist*>& proxies) {
    for(auto& proxy : root.get_proxies()) {
      proxies.insert(proxy);
      insert_proxies(*proxy, proxies);
    }
  }

  struct ChildObserver : public QObject {
    SelectionUpdateSignal m_on_update;
    std::unordered_map<const QObject*, const Stylist*> m_children_stylists;
    scoped_connection m_link_connection;
    ConnectionGroup m_delete_connections;

    ChildObserver(
        const Stylist& stylist, const SelectionUpdateSignal& on_update)
        : m_on_update(on_update) {
      auto children = std::unordered_set<const Stylist*>();
      for(auto child : stylist.get_widget().children()) {
        if(child && child->isWidgetType()) {
          auto& stylist = find_stylist(static_cast<QWidget&>(*child));
          add(stylist);
          children.insert(&stylist);
        }
      }
      for(auto& link : stylist.get_links()) {
        add(*link);
        children.insert(link);
      }
      m_link_connection = stylist.connect_link_signal(
        std::bind_front(&ChildObserver::on_link, this));
      insert_proxies(stylist, children);
      if(!children.empty()) {
        m_on_update(std::move(children), {});
      }
      stylist.get_widget().installEventFilter(this);
    }

    bool is_connected() const {
      return true;
    }

    bool eventFilter(QObject* watched, QEvent* event) override {
      if(event->type() == QEvent::ChildAdded) {
        auto& child = *static_cast<QChildEvent&>(*event).child();
        if(child.isWidgetType()) {
          auto& stylist = find_stylist(static_cast<QWidget&>(child));
          add(stylist);
          m_on_update({&stylist}, {});
        }
      } else if(event->type() == QEvent::ChildRemoved) {
        remove(*static_cast<QChildEvent&>(*event).child());
      }
      return QObject::eventFilter(watched, event);
    }

    void add(const Stylist& stylist) {
      auto& child = stylist.get_widget();
      auto connection = stylist.connect_delete_signal(
        std::bind_front(&ChildObserver::remove, this, std::ref(child)));
      m_delete_connections.AddConnection(&child, connection);
      m_children_stylists.insert_or_assign(&child, &stylist);
    }

    void remove(const QObject& child) {
      auto i = m_children_stylists.find(&child);
      if(i != m_children_stylists.end()) {
        m_delete_connections.Disconnect(&child);
        auto stylist = i->second;
        m_children_stylists.erase(i);
        m_on_update({}, {stylist});
      }
    }

    void on_link(const Stylist& link) {
      add(link);
      m_on_update({&link}, {});
    }
  };
}

ChildSelector::ChildSelector(Selector base, Selector child)
  : m_base(std::move(base)),
    m_child(std::move(child)) {}

const Selector& ChildSelector::get_base() const {
  return m_base;
}

const Selector& ChildSelector::get_child() const {
  return m_child;
}

ChildSelector Spire::Styles::operator >(Selector base, Selector child) {
  return ChildSelector(std::move(base), std::move(child));
}

SelectConnection Spire::Styles::select(const ChildSelector& selector,
    const Stylist& base, const SelectionUpdateSignal& on_update) {
  return select(CombinatorSelector(selector.get_base(), selector.get_child(),
    [] (const auto& stylist, const auto& on_update) {
      return SelectConnection(
        std::make_unique<ChildObserver>(stylist, on_update));
    }), base, on_update);
}

std::size_t std::hash<ChildSelector>::operator ()(
    const ChildSelector& selector) const {
  auto seed = std::size_t(0);
  hash_combine(seed, std::hash<Selector>()(selector.get_base()));
  hash_combine(seed, std::hash<Selector>()(selector.get_child()));
  return seed;
}
