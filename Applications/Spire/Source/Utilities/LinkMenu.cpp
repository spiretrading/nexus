#include "Spire/Utilities/LinkMenu.hpp"
#include <tuple>
#include <unordered_set>
#include <QApplication>
#include <QPointer>
#include "Spire/BookView/BookViewWindow.hpp"
#include "Spire/Charting/ChartWindow.hpp"
#include "Spire/Spire/AssociativeValueModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/TimeAndSales/TimeAndSalesWindow.hpp"
#include "Spire/Ui/CheckButtonMenuItem.hpp"
#include "Spire/Ui/ContextMenu.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/Ui.hpp"
#include "Spire/Ui/Window.hpp"
#include "Spire/Ui/WindowHighlight.hpp"

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

  Window* find_ancestor_window(QWidget& widget) {
    for(auto candidate = &widget; candidate;
        candidate = candidate->parentWidget()) {
      if(auto window = dynamic_cast<Window*>(candidate)) {
        return window;
      }
    }
    return nullptr;
  }

  auto get_link_group(TickerContext& target) {
    auto visited = std::unordered_set<std::string>();
    auto group = std::vector<Window*>();
    auto context = optional<TickerContext&>(target);
    while(context && visited.insert(context->GetIdentifier()).second) {
      if(auto window = dynamic_cast<Window*>(&*context);
          window && window->isVisible()) {
        group.push_back(window);
      }
      auto& next_id = context->GetLinkedIdentifier();
      if(next_id.empty()) {
        break;
      }
      context = TickerContext::FindTickerContext(next_id);
    }
    return group;
  }

  void install_hover_highlight(ContextMenu& submenu,
      std::shared_ptr<std::vector<LinkableWindowInfo>> windows) {
    auto source = find_ancestor_window(submenu);
    if(!source) {
      return;
    }
    auto highlight = std::make_shared<std::unique_ptr<WindowHighlight>>();
    QObject::connect(source, &QObject::destroyed, &submenu,
      [highlight] { highlight->reset(); });
    submenu.connect_current_signal(
      [windows = std::move(windows), source = QPointer<Window>(source),
          highlight] (const auto& current_index) {
        highlight->reset();
        if(!source || !current_index ||
            *current_index >= static_cast<int>(windows->size())) {
          return;
        }
        auto target = TickerContext::FindTickerContext(
          (*windows)[*current_index].m_id.toStdString());
        if(!target) {
          return;
        }
        auto group = get_link_group(*target);
        if(std::ranges::find(group, source.data()) == group.end()) {
          group.push_back(source.data());
        }
        *highlight = std::make_unique<WindowHighlight>(std::move(group));
      });
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
  auto sorted_windows =
    std::make_shared<std::vector<LinkableWindowInfo>>(std::move(windows));
  for(auto& window : *sorted_windows) {
    auto item = make_link_menu_item(window.m_type, window.m_ticker,
      current_id->get_association(window.m_id));
    item->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    submenu->add_action("",
      [item] {
        item->get_current()->set(!item->get_current()->get());
      }, item);
  }
  if(auto initial = current->get()) {
    current_id->set(initial->m_id);
  }
  current_id->connect_update_signal(
    [current, sorted_windows, &parent] (const auto& id) {
      if(id.isEmpty()) {
        current->set(none);
      } else if(auto i = std::ranges::find(*sorted_windows, id,
          &LinkableWindowInfo::m_id); i != sorted_windows->end()) {
        current->set(*i);
      }
      parent.hide();
    });
  QObject::connect(submenu, &QObject::destroyed, [current_id] {});
  parent.add_menu(QObject::tr("Link To"), *submenu);
  install_hover_highlight(*submenu, std::move(sorted_windows));
}
