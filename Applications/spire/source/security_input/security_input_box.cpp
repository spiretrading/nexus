#include "spire/security_input/security_input_box.hpp"
#include <QHBoxLayout>
#include "spire/security_input/local_security_input_model.hpp"
#include "spire/security_input/security_info_list_view.hpp"
#include "spire/spire/dimensions.hpp"
#include "spire/ui/ui.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace spire;

security_input_box::security_input_box(security_input_model& model)
    : m_model(&model) {
  setFixedSize(scale(180, 30));
  setStyleSheet(QString(R"(
    QWidget {
      border: %1px solid #C8C8C8;
    }
    :hover {
      border: %1px solid #4b23A0;
    })").arg(scale_width(1)));
  auto layout = new QHBoxLayout(this);
  layout->setMargin(scale_width(1));
  layout->setSpacing(0);
  m_security_line_edit = new QLineEdit(this);
  connect(m_security_line_edit, &QLineEdit::textChanged,
    [=] { on_text_changed(); });
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
  m_securities = new security_info_list_view();
  m_securities->setVisible(false);
}

connection security_input_box::connect_commit_signal(
    const commit_signal::slot_type& slot) const {
  return m_commit_signal.connect(slot);
}

void security_input_box::on_text_changed() {
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
