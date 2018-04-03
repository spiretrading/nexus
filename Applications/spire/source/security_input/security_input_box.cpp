#include "spire/security_input/security_input_box.hpp"
#include <QEvent>
#include <QHBoxLayout>
#include <QKeyEvent>
#include "spire/security_input/local_security_input_model.hpp"
#include "spire/security_input/security_info_list_view.hpp"
#include "spire/spire/dimensions.hpp"
#include "spire/ui/ui.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace spire;

security_input_box::security_input_box(security_input_model& model,
    QWidget* parent)
    : QWidget(parent),
      m_model(&model) {
  setFixedSize(scale(180, 30));
  setObjectName("security_input_box_line_edit");
  setStyleSheet(QString(R"(
    #security_input_box_line_edit {
      border: %1px solid #C8C8C8;
    }
    :hover {
      border: %1px solid #4b23A0;
    })").arg(scale_width(1)));
  auto layout = new QHBoxLayout(this);
  layout->setMargin(scale_width(1));
  layout->setSpacing(0);
  m_security_line_edit = new QLineEdit(this);
  m_security_line_edit->installEventFilter(this);
  connect(m_security_line_edit, &QLineEdit::returnPressed,
    [=] { enter_pressed(); });
  m_security_line_edit->setStyleSheet(QString(R"(
    background-color: #FFFFFF;
    border: none;
    font-family: Roboto;
    font-size: %1px;
    padding: %2px 0px %2px %3px;)")
    .arg(scale_height(12)).arg(scale_height(9)).arg(scale_width(8)));
  layout->addWidget(m_security_line_edit);
  m_icon_label = new QLabel(this);
  m_icon_label->setPixmap(QPixmap::fromImage(imageFromSvg(":/icons/search.svg",
    scale(10, 10))));
  m_icon_label->setStyleSheet(QString(R"(
    background-color: #FFFFFF;
    border: none;
    padding: %1px %2px %1px 0px;)")
    .arg(scale_height(9)).arg(scale_width(8)));
  layout->addWidget(m_icon_label);
  m_securities = new security_info_list_view(m_security_line_edit, this);
  m_securities->connect_clicked_signal(
    [=] (const Security& s) { security_selected(s); });
  m_securities->connect_highlighted_signal(
    [=] (const Security& s) { security_highlighted(s); });
  m_securities->setVisible(false);
  this->parent()->installEventFilter(this);
}

connection security_input_box::connect_commit_signal(
    const commit_signal::slot_type& slot) const {
  return m_commit_signal.connect(slot);
}

bool security_input_box::eventFilter(QObject* watched, QEvent* event) {
  if(watched == m_security_line_edit) {
    if(event->type() == QEvent::KeyPress) {
      auto e = static_cast<QKeyEvent*>(event);
      if(e->key() == Qt::Key_Down) {
        m_securities->highlight_next_item();
      } else if(e->key() == Qt::Key_Up) {
        m_securities->highlight_previous_item();
      } else  {
        update_autocomplete();
      }
    }
    if(event->type() == QEvent::FocusIn) {
      setStyleSheet(QString(R"(
        #security_input_box_line_edit {
          border: %1px solid #4b23A0;
        }
        #security_input_box_line_edit:hover {
          border: %1px solid #4b23A0;
        })").arg(scale_width(1)));
    } else if(event->type() == QEvent::FocusOut) {
      setStyleSheet(QString(R"(
        #security_input_box_line_edit {
          border: %1px solid #C8C8C8;
        }
        #security_input_box_line_edit:hover {
          border: %1px solid #4b23A0;
        })").arg(scale_width(1)));
    }
  }
  if(watched == parent()) {
    if(event->type() == QEvent::Move) {
      auto pos = mapToGlobal(m_security_line_edit->geometry().topLeft());
      m_securities->move(pos.x() - scale_width(1), pos.y() +
        m_security_line_edit->height() + scale_height(1));
    }
  }
  return QWidget::eventFilter(watched, event);
}

void security_input_box::security_selected(const Security& security) {
  m_security_line_edit->setText(QString::fromStdString(
    Nexus::ToString(security)));
  m_securities->setVisible(false);
}

void security_input_box::security_highlighted(const Security& security) {
  m_security_line_edit->setText(QString::fromStdString(
    Nexus::ToString(security)));
}

void security_input_box::update_autocomplete() {
  if(!m_security_line_edit->text().isEmpty()) {
    auto list = m_model->autocomplete(
      m_security_line_edit->text().toStdString());
    m_securities->set_list(list);
    if(list.size() > 0) {
      auto pos = mapToGlobal(m_security_line_edit->geometry().topLeft());
      m_securities->move(pos.x() - scale_width(1), pos.y() +
        m_security_line_edit->height() + scale_height(1));
      m_securities->setVisible(true);
      m_securities->raise();
    } else {
      m_securities->setVisible(false);
    }
  } else {
    m_securities->setVisible(false);
  }
}

void security_input_box::enter_pressed() {
  m_commit_signal(Security(ParseSecurity(
    QString(m_security_line_edit->text()).toStdString())));
}
