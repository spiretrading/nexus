#include "Spire/Utilities/LinkMenu.hpp"
#include <tuple>
#include <unordered_map>
#include <QApplication>
#include "Spire/BookView/BookViewWindow.hpp"
#include "Spire/Charting/ChartWindow.hpp"
#include "Spire/Spire/AssociativeValueModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/TimeAndSales/TimeAndSalesWindow.hpp"
#include "Spire/Ui/CheckButtonMenuItem.hpp"
#include "Spire/Ui/ContextMenu.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/Ui.hpp"

using namespace boost;
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

  optional<LinkableWindowType> get_window_type(QWidget& widget) {
    if(dynamic_cast<TimeAndSalesWindow*>(&widget)) {
      return LinkableWindowType::TIME_AND_SALES;
    } else if(dynamic_cast<BookViewWindow*>(&widget)) {
      return LinkableWindowType::BOOK_VIEW;
    } else if(dynamic_cast<ChartWindow*>(&widget)) {
      return LinkableWindowType::CHART;
    }
    return none;
  }
}

void Spire::add_link_menu(ContextMenu& parent, TickerContext& context) {
  auto windows = std::vector<LinkableWindowInfo>();
  auto initial = optional<LinkableWindowInfo>();
  auto linked_id = QString::fromStdString(context.GetLinkedIdentifier());
  for(auto& widget : QApplication::topLevelWidgets()) {
    auto target = dynamic_cast<TickerContext*>(widget);
    if(!widget->isVisible() || !target || target == &context) {
      continue;
    }
    if(auto type = get_window_type(*widget)) {
      auto info = LinkableWindowInfo{
        QString::fromStdString(target->GetIdentifier()),
        *type, target->GetDisplayedTicker()};
      if(!linked_id.isEmpty() && info.m_id == linked_id) {
        initial = info;
      }
      windows.push_back(std::move(info));
    }
  }
  auto current = std::make_shared<LocalOptionalLinkableWindowInfoModel>();
  if(initial) {
    current->set(*initial);
  }
  current->connect_update_signal([&context] (const auto& selection) {
    if(!selection) {
      context.Unlink();
      return;
    }
    if(auto target =
        TickerContext::FindTickerContext(selection->m_id.toStdString())) {
      context.Link(*target);
      if(target->GetLinkedIdentifier().empty()) {
        target->Link(context);
      }
    }
  });
  add_link_sub_menu_item(parent, std::move(windows), current);
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

std::shared_ptr<OptionalLinkableWindowInfoModel>
    Spire::add_link_sub_menu_item(ContextMenu& parent,
      std::vector<LinkableWindowInfo> windows) {
  auto current = std::make_shared<LocalOptionalLinkableWindowInfoModel>();
  add_link_sub_menu_item(parent, std::move(windows), current);
  return current;
}

void Spire::add_link_sub_menu_item(ContextMenu& parent,
    std::vector<LinkableWindowInfo> windows,
    std::shared_ptr<OptionalLinkableWindowInfoModel> current) {
  auto submenu = new ContextMenu(static_cast<QWidget&>(parent));
  std::sort(windows.begin(), windows.end(),
    [] (const auto& left, const auto& right) {
      return std::tie(left.m_ticker, left.m_type) <
        std::tie(right.m_ticker, right.m_type);
    });
  auto current_id = std::make_shared<AssociativeValueModel<QString>>();
  auto lookup =
    std::make_shared<std::unordered_map<QString, LinkableWindowInfo>>();
  for(auto& window : windows) {
    auto item = make_link_menu_item(window.m_type, window.m_ticker,
      current_id->get_association(window.m_id));
    item->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    submenu->add_action("",
      [item] {
        item->get_current()->set(!item->get_current()->get());
      }, item);
    lookup->emplace(window.m_id, window);
  }
  if(auto initial = current->get()) {
    current_id->set(initial->m_id);
  }
  current_id->connect_update_signal(
    [current, lookup, &parent] (const auto& id) {
      if(id.isEmpty()) {
        current->set(none);
      } else if(auto i = lookup->find(id); i != lookup->end()) {
        current->set(i->second);
      }
      parent.hide();
    });
  QObject::connect(submenu, &QObject::destroyed, [current_id] {});
  parent.add_menu(QObject::tr("Link To"), *submenu);
}
