#include "Spire/SecurityInput/SecurityInputBox.hpp"
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include "Spire/SecurityInput/LocalSecurityInputModel.hpp"
#include "Spire/SecurityInput/SecurityInfoListView.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/Utility.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/Ui.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

SecurityInputBox::SecurityInputBox(Ref<SecurityInputModel> model,
    bool is_compact, QWidget* parent)
    : SecurityInputBox(Ref(model), "", is_compact, parent) {}

SecurityInputBox::SecurityInputBox(Ref<SecurityInputModel> model,
    const QString& initial_text, bool is_compact, QWidget* parent)
    : QWidget(parent),
      m_is_compact(is_compact),
      m_model(model.Get()) {
  parent->installEventFilter(this);
  setObjectName("SecurityInputBox");
  if(!m_is_compact) {
    setStyleSheet(QString(R"(
      #SecurityInputBox {
        border: %1px solid #C8C8C8;
      }
      :hover {
        border: %1px solid #4b23A0;
      })").arg(scale_width(1)));
  }
  auto layout = new QHBoxLayout(this);
  layout->setMargin(scale_width(1));
  layout->setSpacing(0);
  m_security_line_edit = new QLineEdit(this);
  m_security_line_edit->installEventFilter(this);
  connect(m_security_line_edit, &QLineEdit::returnPressed,
    [=] { enter_pressed(); });
  connect(m_security_line_edit, &QLineEdit::textEdited,
    [=] { on_text_edited(); });
  m_security_line_edit->setStyleSheet(QString(R"(
    background-color: #FFFFFF;
    border: none;
    font-family: Roboto;
    font-size: %1px;
    padding: %2px 0px %2px %3px;)")
    .arg(scale_height(12)).arg(scale_height(6)).arg(scale_width(5)));
  layout->addWidget(m_security_line_edit);
  if(!m_is_compact) {
    auto icon_label = new QLabel(this);
    icon_label->setPixmap(QPixmap::fromImage(imageFromSvg(
      ":/Icons/search.svg", scale(10, 10))));
    icon_label->setStyleSheet(QString(R"(
      background-color: #FFFFFF;
      border: none;
      padding: %1px %2px %1px 0px;)")
      .arg(scale_height(9)).arg(scale_width(8)));
    layout->addWidget(icon_label);
  }
  m_securities = new SecurityInfoListView(this);
  m_securities->connect_activate_signal(
    [=] (auto& s) { on_activated(s); });
  m_securities->connect_commit_signal([=] (auto& s) { on_commit(s); });
  m_securities->setVisible(false);
  window()->installEventFilter(this);
  m_security_line_edit->setText(initial_text);
  setFocusProxy(m_security_line_edit);
}

connection SecurityInputBox::connect_commit_signal(
    const CommitSignal::slot_type& slot) const {
  return m_commit_signal.connect(slot);
}

bool SecurityInputBox::eventFilter(QObject* watched, QEvent* event) {
  if(watched == m_security_line_edit) {
    if(event->type() == QEvent::KeyPress) {
      auto e = static_cast<QKeyEvent*>(event);
      if(e->key() == Qt::Key_Down) {
        m_securities->activate_next();
      } else if(e->key() == Qt::Key_Up) {
        m_securities->activate_previous();
      }
    }
    if(event->type() == QEvent::FocusIn && !m_is_compact) {
      setStyleSheet(QString(R"(
        #SecurityInputBox {
          border: %1px solid #4b23A0;
        }
        #SecurityInputBox:hover {
          border: %1px solid #4b23A0;
        })").arg(scale_width(1)));
    } else if(event->type() == QEvent::FocusOut && !m_is_compact) {
      setStyleSheet(QString(R"(
        #SecurityInputBox {
          border: %1px solid #C8C8C8;
        }
        #SecurityInputBox:hover {
          border: %1px solid #4b23A0;
        })").arg(scale_width(1)));
    }
  } else if(watched == window()) {
    if(event->type() == QEvent::Move) {
      move_line_edit();
    } else if(event->type() == QEvent::FocusIn) {
      m_security_line_edit->setFocus();
    } else if(event->type() == QEvent::WindowDeactivate &&
        !m_securities->isActiveWindow()) {
      m_securities->hide();
    }
  } else if(watched == parent()) {
    if(event->type() == QEvent::Wheel) {
      m_securities->hide();
    }
  }
  return QWidget::eventFilter(watched, event);
}

void SecurityInputBox::hideEvent(QHideEvent* event) {
  m_securities->close();
}

void SecurityInputBox::resizeEvent(QResizeEvent* event) {
  m_securities->setFixedWidth(max(width(), scale_width(142)));
}

void SecurityInputBox::showEvent(QShowEvent* event) {
  on_text_edited();
  m_securities->setFixedWidth(max(width(), scale_width(142)));
}

void SecurityInputBox::on_text_edited() {
  m_completions = m_model->autocomplete(
    m_security_line_edit->text().toStdString());
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
      move_line_edit();
      m_securities->setVisible(true);
      m_securities->raise();
    }
  });
}

void SecurityInputBox::move_line_edit() {
  auto x_pos = static_cast<QWidget*>(parent())->mapToGlobal(
    geometry().bottomLeft()).x();
  auto y_pos = static_cast<QWidget*>(parent())->mapToGlobal(
    frameGeometry().bottomLeft()).y();
  if(!m_is_compact) {
    m_securities->move(x_pos, y_pos + 1);
  } else {
    m_securities->move(x_pos, y_pos + 2);
  }
}

void SecurityInputBox::enter_pressed() {
  m_commit_signal(ParseSecurity(
    m_security_line_edit->text().toUpper().toStdString()));
}

void SecurityInputBox::on_activated(const Security& security) {
  auto item_delegate = CustomVariantItemDelegate();
  m_security_line_edit->setText(item_delegate.displayText(
    QVariant::fromValue(security), QLocale()));
}

void SecurityInputBox::on_commit(const Security& security) {
  m_commit_signal(security);
}
