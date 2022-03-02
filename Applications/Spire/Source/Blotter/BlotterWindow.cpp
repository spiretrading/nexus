#include "Spire/Blotter/BlotterWindow.hpp"
#include "Spire/Blotter/BlotterStatusBar.hpp"
#include "Spire/Blotter/BlotterTaskView.hpp"
#include "Spire/Styles/Selectors.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/SplitView.hpp"
#include "Spire/Ui/TabView.hpp"

using namespace Spire;
using namespace Spire::Styles;

BlotterWindow::BlotterWindow(QWidget* parent)
    : Window(parent) {
  setWindowTitle("Blotter");
  set_svg_icon(":/Icons/blotter.svg");
  m_body = new QWidget();
  set_body(m_body);
  m_task_view = new BlotterTaskView();
  m_tab_view = new TabView();
  m_split_view = new SplitView(*m_task_view, *m_tab_view);
  update_style(*m_split_view, [] (auto& styles) {
    styles.get(Any()).set(Qt::Orientation::Vertical);
  });
  auto layout = make_vbox_layout(m_body);
  layout->addWidget(m_split_view);
  m_status_bar = new BlotterStatusBar();
  layout->addWidget(m_status_bar);
}
