#include "spire/charting/charting_window.hpp"
#include <QHBoxLayout>
#include <QIcon>
#include <QVBoxLayout>
#include "spire/security_input/security_input_model.hpp"
#include "spire/spire/dimensions.hpp"
#include "spire/ui/window.hpp"

using namespace Beam;
using namespace Spire;

ChartingWindow::ChartingWindow(Ref<SecurityInputModel> input_model,
    QWidget* parent)
    : QWidget(parent),
      m_input_model(input_model.Get()) {
  m_body = new QWidget(this);
  m_body->setMinimumSize(scale(400, 320));
  m_body->resize(scale(400, 320));
  m_body->setStyleSheet("background-color: #FFFFFF;");
  auto window_layout = new QHBoxLayout(this);
  window_layout->setContentsMargins({});
  auto window = new Window(m_body, this);
  setWindowTitle(tr("Chart"));
  window->set_svg_icon(":/icons/chart-black.svg",
    ":/icons/chart-grey.svg");
  setWindowIcon(QIcon(":/icons/chart-icon-256x256.png"));
  window_layout->addWidget(window);
  auto layout = new QVBoxLayout(m_body);
}
