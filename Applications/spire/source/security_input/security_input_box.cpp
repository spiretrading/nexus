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


#include <QDebug>



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
  m_security_line_edit->installEventFilter(this);
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
  auto pos = this->mapToGlobal(m_security_line_edit->geometry().topLeft());
  qDebug() << pos.x();
  qDebug() << pos.y();
  m_securities->move(pos.x(), pos.y() + m_security_line_edit->height());
  m_securities->show();
  m_securities->set_list(m_model->autocomplete("ma"));
}

connection security_input_box::connect_commit_signal(
    const commit_signal::slot_type& slot) const {
  return m_commit_signal.connect(slot);
}
