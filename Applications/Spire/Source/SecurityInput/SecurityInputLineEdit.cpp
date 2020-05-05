#include "Spire/SecurityInput/SecurityInputLineEdit.hpp"
#include <QKeyEvent>
#include <QPainter>
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

  auto SEARCH_ICON() {
    static auto icon = imageFromSvg(":/Icons/search.svg",
      {ICON_HEIGHT(), ICON_WIDTH()});
    return icon;
  }
}

SecurityInputLineEdit::SecurityInputLineEdit(const QString& initial_text,
    Beam::Ref<SecurityInputModel> model, bool is_icon_visible, QWidget* parent)
    : QLineEdit(initial_text, parent),
      m_model(model.Get()),
      m_is_icon_visible(is_icon_visible) {
  parent->installEventFilter(this);
  setObjectName("SecurityInputLineEdit");
  setStyleSheet(QString(R"(
    #SecurityInputLineEdit {
      background-color: #FFFFFF;
      border: none;
      font-family: Roboto;
      font-size: %1px;
      padding: %2px 0px %2px %3px;
    })").arg(scale_height(12)).arg(scale_height(6)).arg(scale_width(6)));
  connect(this, &QLineEdit::textEdited, this,
    &SecurityInputLineEdit::on_text_edited);
  m_securities = new SecurityInfoListView(this);
  m_securities->connect_activate_signal(
    [=] (auto& s) { on_activated(s); });
  m_securities->connect_commit_signal([=] (auto& s) {
    on_commit(s);
  });
  m_securities->setVisible(false);
  window()->installEventFilter(this);
}

const Nexus::Security& SecurityInputLineEdit::get_security() const {
  return m_security;
}

connection SecurityInputLineEdit::connect_commit_signal(
    const CommitSignal::slot_type& slot) const {
  return m_commit_signal.connect(slot);
}

bool SecurityInputLineEdit::eventFilter(QObject* watched, QEvent* event) {
  if(watched == window()) {
    if(event->type() == QEvent::Move) {
      move_securities_list();
    } else if(event->type() == QEvent::FocusIn) {
      setFocus();
    } else if(event->type() == QEvent::WindowDeactivate &&
        !m_securities->isActiveWindow()) {
      m_securities->hide();
    }
  } else if(watched == parent()) {
    if(event->type() == QEvent::Wheel) {
      m_securities->hide();
    }
   }
  return QLineEdit::eventFilter(watched, event);
}

void SecurityInputLineEdit::hideEvent(QHideEvent* event) {
  m_securities->close();
}

void SecurityInputLineEdit::keyPressEvent(QKeyEvent* event) {
  if(event->key() == Qt::Key_Down) {
    m_securities->activate_next();
    return;
  } else if(event->key() == Qt::Key_Up) {
    m_securities->activate_previous();
    return;
  } else if(event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) {
    on_commit(ParseSecurity(text().toUpper().toStdString()));
    return;
  } else if(event->key() == Qt::Key_Delete) {
    on_commit({});
  }
  QLineEdit::keyPressEvent(event);
}

void SecurityInputLineEdit::paintEvent(QPaintEvent* event) {
  QLineEdit::paintEvent(event);
  if(m_is_icon_visible) {
    auto painter = QPainter(this);
    painter.drawImage(width() - ICON_WIDTH() - scale_width(8),
      height() - (height() / 2) - (ICON_HEIGHT() / 2), SEARCH_ICON());
  }
}

void SecurityInputLineEdit::resizeEvent(QResizeEvent* event) {
  m_securities->setFixedWidth(max(width(), scale_width(142)));
}

void SecurityInputLineEdit::showEvent(QShowEvent* event) {
  on_text_edited();
  m_securities->setFixedWidth(max(width(), scale_width(142)));
}

void SecurityInputLineEdit::move_securities_list() {
  auto x_pos = static_cast<QWidget*>(parent())->mapToGlobal(
    geometry().bottomLeft()).x();
  auto y_pos = static_cast<QWidget*>(parent())->mapToGlobal(
    frameGeometry().bottomLeft()).y();
  m_securities->move(x_pos, y_pos + 2);
}

void SecurityInputLineEdit::on_activated(const Security& security) {
  auto item_delegate = CustomVariantItemDelegate();
  setText(item_delegate.displayText(QVariant::fromValue(security), QLocale()));
}

void SecurityInputLineEdit::on_commit(const Security& security) {
  m_security = security;
  emit editingFinished();
  m_commit_signal(security);
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
    m_securities->set_list(completions);
    if(completions.empty()) {
      m_securities->hide();
    } else {
      move_securities_list();
      m_securities->setVisible(true);
      m_securities->raise();
    }
  });
}
