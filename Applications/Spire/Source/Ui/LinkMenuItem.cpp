#include "Spire/Ui/LinkMenuItem.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/CheckButtonMenuItem.hpp"
#include "Spire/Ui/Icon.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/Ui.hpp"

using namespace Spire;
using namespace Spire::Styles;

namespace {
  const QString& to_text(LinkMenuItem::LinkableWindowType type) {
    if(type == LinkMenuItem::LinkableWindowType::BOOK_VIEW) {
      static const auto value = QObject::tr("Book View");
      return value;
    } else if(type == LinkMenuItem::LinkableWindowType::CHART) {
      static const auto value = QObject::tr("Chart");
      return value;
    } else if(type == LinkMenuItem::LinkableWindowType::TIME_AND_SALES) {
      static const auto value = QObject::tr("Time and Sales");
      return value;
    }
    static const auto value = QString();
    return value;
  }

  const QImage& load_icon(LinkMenuItem::LinkableWindowType type) {
    if(type == LinkMenuItem::LinkableWindowType::BOOK_VIEW) {
      static const auto icon =
        image_from_svg(":/Icons/bookview.svg", scale(10, 10));
      return icon;
    } else if(type == LinkMenuItem::LinkableWindowType::CHART) {
      static const auto icon =
        image_from_svg(":/Icons/chart.svg", scale(10, 10));
      return icon;
    } else if(type == LinkMenuItem::LinkableWindowType::TIME_AND_SALES) {
      static const auto icon =
        image_from_svg(":/Icons/time-sales.svg", scale(10, 10));
      return icon;
    }
    static const auto icon = QImage();
    return icon;
  }
}

LinkMenuItem::LinkMenuItem(LinkableWindowType type, QString symbol,
  QWidget* parent)
  : LinkMenuItem(type, std::move(symbol),
      std::make_shared<LocalBooleanModel>(), parent) {}

LinkMenuItem::LinkMenuItem(LinkableWindowType type, QString symbol,
    std::shared_ptr<BooleanModel> current, QWidget* parent)
    : QWidget(parent),
      m_current(std::move(current)),
BEAM_SUPPRESS_THIS_INITIALIZER()
      m_click_observer(*this) {
BEAM_UNSUPPRESS_THIS_INITIALIZER()
  setAttribute(Qt::WA_NoMousePropagation);
  auto layout = make_hbox_layout(this);
  auto icon = new Icon(load_icon(type));
  icon->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  update_style(*icon, [] (auto& style) {
    style.get(Any() || Hover()).set(Fill(QColor(Qt::black)));
  });
  layout->addWidget(icon, 0, Qt::AlignVCenter);
  layout->addSpacing(scale_width(8));
  auto label = [&] {
    if(symbol.isEmpty()) {
      return to_text(type);
    }
    return symbol + " " + QChar(0x2013) + " " + to_text(type);
  }();
  auto item = new CheckButtonMenuItem(label, m_current);
  item->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  layout->addWidget(item, 0, Qt::AlignVCenter);
  setFocusProxy(item);
  m_click_observer.connect_click_signal(
    std::bind_front(&LinkMenuItem::on_click, this));
}

const std::shared_ptr<BooleanModel>& LinkMenuItem::get_current() const {
  return m_current;
}

void LinkMenuItem::on_click() {
  m_current->set(!m_current->get());
}
