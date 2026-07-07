#include "Spire/Utilities/LinkMenu.hpp"
#include <QApplication>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/CheckButtonMenuItem.hpp"
#include "Spire/Ui/ContextMenu.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/Ui.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;
using namespace Spire::LegacyUI;

namespace {
  const QString& to_text(LinkableWindowType type) {
    if(type == LinkableWindowType::BOOK_VIEW) {
      static const auto value = QObject::tr("Book View");
      return value;
    } else if(type == LinkableWindowType::CHART) {
      static const auto value = QObject::tr("Chart");
      return value;
    } else if(type == LinkableWindowType::TIME_AND_SALES) {
      static const auto value = QObject::tr("Time and Sales");
      return value;
    }
    static const auto value = QString();
    return value;
  }

  const QImage& load_icon(LinkableWindowType type) {
    if(type == LinkableWindowType::BOOK_VIEW) {
      static const auto icon =
        image_from_svg(":/Icons/bookview.svg", scale(10, 10));
      return icon;
    } else if(type == LinkableWindowType::CHART) {
      static const auto icon =
        image_from_svg(":/Icons/chart.svg", scale(10, 10));
      return icon;
    } else if(type == LinkableWindowType::TIME_AND_SALES) {
      static const auto icon =
        image_from_svg(":/Icons/time-sales.svg", scale(10, 10));
      return icon;
    }
    static const auto icon = QImage();
    return icon;
  }

  struct LinkModel : BooleanModel {
    TickerContext* m_source;
    TickerContext* m_target;
    LocalBooleanModel m_value;

    LinkModel(TickerContext& source, TickerContext& target)
      : m_source(&source),
        m_target(&target),
        m_value(m_source->GetLinkedIdentifier() == m_target->GetIdentifier()) {}

    const Type& get() const override {
      return m_value.get();
    }

    QValidator::State set(const bool& value) {
      if(value == get()) {
        return QValidator::State::Acceptable;
      }
      if(value) {
        m_source->Link(*m_target);
        if(m_target->GetLinkedIdentifier().empty()) {
          m_target->Link(*m_source);
        }
      } else {
        m_source->Unlink();
      }
      return m_value.set(value);
    }

    connection connect_update_signal(
        const UpdateSignal::slot_type& slot) const {
      return m_value.connect_update_signal(slot);
    }
  };
}

void Spire::add_link_menu(ContextMenu& parent, TickerContext& context) {
  struct Item {
    Ticker m_ticker;
    QString m_title;
    std::shared_ptr<BooleanModel> m_is_linked;
  };
  auto windows = QApplication::topLevelWidgets();
  auto items = std::vector<Item>();
  for(auto& window : windows) {
    auto window_context = dynamic_cast<TickerContext*>(window);
    if(window->isVisible() && window_context != nullptr &&
        window_context != &context) {
      auto item = Item(window_context->GetDisplayedTicker(),
        window->windowTitle(),
        std::make_shared<LinkModel>(context, *window_context));
      items.push_back(std::move(item));
    }
  }
  if(items.empty()) {
    return;
  }
  auto link_menu = new ContextMenu(static_cast<QWidget&>(parent));
  std::sort(items.begin(), items.end(),
    [&] (const auto& left, const auto& right) {
      return to_text(left.m_ticker) < to_text(right.m_ticker);
    });
  for(auto& item : items) {
    link_menu->add_check_box(item.m_title, item.m_is_linked);
    item.m_is_linked->connect_update_signal([&] (auto value) {
      parent.hide();
    });
  }
  parent.add_menu(QObject::tr("Link to"), *link_menu);
}

CheckButtonMenuItem* Spire::make_link_menu_item(LinkableWindowType type,
    const Ticker& ticker, QWidget* parent) {
  return make_link_menu_item(type, ticker,
    std::make_shared<LocalBooleanModel>(), parent);
}

CheckButtonMenuItem* Spire::make_link_menu_item(LinkableWindowType type,
    const Ticker& ticker, std::shared_ptr<BooleanModel> current,
    QWidget* parent) {
  auto label = [&] {
    if(ticker) {
      return to_text(ticker) + " " + QChar(0x2013) + " " + ::to_text(type);
    }
    return ::to_text(type);
  }();
  return new CheckButtonMenuItem(load_icon(type), std::move(label),
    std::move(current), parent);
}
