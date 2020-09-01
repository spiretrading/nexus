#include "Spire/SecurityInput/SecurityInputLineEdit.hpp"
#include <algorithm>
#include <QKeyEvent>
#include <QPainter>
#include "Spire/SecurityInput/SecurityInfoWidget.hpp"
#include "Spire/SecurityInput/SecurityInputModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/Utility.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"

using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

namespace {
  auto ICON_HEIGHT() {
    static auto height = scale_height(10);
    return height;
  }

  auto ICON_WIDTH() {
    static auto width = scale_width(10);
    return width;
  }

  const auto& SEARCH_ICON() {
    static auto icon = imageFromSvg(":/Icons/search.svg",
      {ICON_HEIGHT(), ICON_WIDTH()});
    return icon;
  }
}

SecurityInputLineEdit::SecurityInputLineEdit(Security security,
    Beam::Ref<SecurityInputModel> model, bool is_icon_visible, QWidget* parent)
    : SecurityInputLineEdit("", model, is_icon_visible, parent) {
  m_security = std::move(security);
}

SecurityInputLineEdit::SecurityInputLineEdit(const QString& initial_text,
    Beam::Ref<SecurityInputModel> model, bool is_icon_visible, QWidget* parent)
    : TextInputWidget(initial_text, parent),
      m_model(model.Get()),
      m_is_icon_visible(is_icon_visible) {
  setText(initial_text);
  set_cell_style();
  //parent->installEventFilter(this);
  setObjectName("SecurityInputLineEdit");
  setStyleSheet(QString(R"(
    #SecurityInputLineEdit {
      background-color: #FFFFFF;
      border: none;
      font-family: Roboto;
      font-size: %1px;
      padding: %2px 0px %2px %3px;
    })").arg(scale_height(12)).arg(scale_height(6)).arg(scale_width(6)));
  connect(this, &TextInputWidget::textEdited, this,
    &SecurityInputLineEdit::on_text_edited);
  m_securities = new DropDownList({}, false, this);
  //m_securities->connect_activate_signal(
  //  [=] (auto& s) { on_activated(s); });
  //m_securities->connect_commit_signal([=] (auto& s) {
  //  on_commit(s);
  //});
  m_securities->setVisible(false);
  window()->installEventFilter(this);
}

const Nexus::Security& SecurityInputLineEdit::get_security() const {
  return m_security;
}

//connection SecurityInputLineEdit::connect_commit_signal(
//    const CommitSignal::slot_type& slot) const {
//  return m_commit_signal.connect(slot);
//}

//bool SecurityInputLineEdit::eventFilter(QObject* watched, QEvent* event) {
//  if(watched == window()) {
//    if(event->type() == QEvent::Move) {
//      move_securities_list();
//    } else if(event->type() == QEvent::FocusIn) {
//      setFocus();
//    } else if(event->type() == QEvent::WindowDeactivate &&
//        !m_securities->isActiveWindow()) {
//      m_securities->hide();
//    }
//  } else if(watched == parent()) {
//    if(event->type() == QEvent::Wheel) {
//      m_securities->hide();
//    }
//   }
//  return TextInputWidget::eventFilter(watched, event);
//}

void SecurityInputLineEdit::focusInEvent(QFocusEvent* event) {
  if(!m_is_icon_visible) {
    TextInputWidget::focusInEvent(event);
  }
}

//void SecurityInputLineEdit::hideEvent(QHideEvent* event) {
//  m_securities->close();
//}

void SecurityInputLineEdit::keyPressEvent(QKeyEvent* event) {
  if(event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) {
    auto current_security = ParseSecurity(text().toUpper().toStdString());
    if(!current_security.GetSymbol().empty()) {
      on_commit(current_security);
    } else {
      on_commit(m_security);
    }
    return;
  } else if(event->key() == Qt::Key_Delete) {
    on_commit({});
  }
  TextInputWidget::keyPressEvent(event);
}

void SecurityInputLineEdit::paintEvent(QPaintEvent* event) {
  QLineEdit::paintEvent(event);
  if(m_is_icon_visible) {
    auto painter = QPainter(this);
    painter.drawImage(width() - ICON_WIDTH() - scale_width(8),
      height() - (height() / 2) - (ICON_HEIGHT() / 2), SEARCH_ICON());
  }
}

//void SecurityInputLineEdit::resizeEvent(QResizeEvent* event) {
//  m_securities->setFixedWidth(std::max(width(), scale_width(142)));
//}

void SecurityInputLineEdit::showEvent(QShowEvent* event) {
  on_text_edited();
  m_securities->setFixedWidth(std::max(width(), scale_width(142)));
}

//void SecurityInputLineEdit::move_securities_list() {
//  auto x_pos = static_cast<QWidget*>(parent())->mapToGlobal(
//    geometry().bottomLeft()).x();
//  auto y_pos = static_cast<QWidget*>(parent())->mapToGlobal(
//    frameGeometry().bottomLeft()).y();
//  m_securities->move(x_pos, y_pos + 2);
//}

void SecurityInputLineEdit::on_activated(const Security& security) {
  auto item_delegate = CustomVariantItemDelegate();
  setText(item_delegate.displayText(QVariant::fromValue(security), QLocale()));
  m_security = ParseSecurity(text().toUpper().toStdString());
}

void SecurityInputLineEdit::on_commit(const Security& security) {
  m_security = security;
  Q_EMIT editingFinished();
  //m_commit_signal(security);
}

void SecurityInputLineEdit::on_text_edited() {
  m_completions = m_model->autocomplete(text().toStdString());
  m_completions.then([=] (auto result) {
    auto completions = [&] {
      try {
        return result.Get();
      } catch(const std::exception&) {
        return std::vector<SecurityInfo>();
      }
    }();
    //m_securities->
    //m_securities->set_list(completions);
    //m_securities->set_items()
    auto widget_items = std::vector<DropDownItem*>(completions.size());
    std::transform(completions.begin(), completions.end(),
      widget_items.begin(), [&] (const auto& item) {
        auto item_widget = new SecurityInfoWidget(item, this);
        return item_widget;
      });
    m_securities->set_items(widget_items);
    if(completions.empty()) {
      m_securities->hide();
    } else {
      //move_securities_list();
      m_securities->setVisible(true);
      m_securities->raise();
    }
  });
  auto current_security = ParseSecurity(text().toUpper().toStdString());
  if(!current_security.GetSymbol().empty()) {
    m_security = current_security;
  }
}
