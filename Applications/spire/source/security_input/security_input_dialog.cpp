#include "spire/security_input/security_input_dialog.hpp"
#include <QLabel>
#include <QVBoxLayout>
#include "spire/spire/dimensions.hpp"
#include "spire/security_input/security_info_list_view.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace spire;

security_input_dialog::security_input_dialog(security_input_model& model) {
  setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
  setFixedSize(scale(196, 68));
  setContentsMargins(scale_width(8), scale_height(6), scale_width(8),
    scale_height(8));
  setStyleSheet(QString(R"(
    background-color: #F5F5F5;
    border: %1px solid #A0A0A0;
  )").arg(scale_width(1)));
  auto layout = new QVBoxLayout(this);
  layout->setMargin(0);
  layout->setSpacing(scale_height(10));
  auto text_label = new QLabel(tr("Security"), this);
  text_label->setStyleSheet(QString(R"(
    border: none;
    font-family: Roboto;
    font-size: %1px;
  )").arg(scale_height(12)));
  layout->addWidget(text_label);
  m_security_input_box = new security_input_box(model);
  layout->addWidget(m_security_input_box);
}

const Security& security_input_dialog::get_security() const noexcept {
  return m_security;
}
