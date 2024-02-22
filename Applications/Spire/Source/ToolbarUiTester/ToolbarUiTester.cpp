#include "Spire/ToolbarUiTester/ToolbarUiTester.hpp"
#include <QVBoxLayout>
#include "Spire/Toolbar/ToolbarWindow.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/CheckBox.hpp"
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace Beam;
using namespace Beam::ServiceLocator;
using namespace Nexus;
using namespace Nexus::AdministrationService;
using namespace Spire;
using namespace Spire::LegacyUI;

ToolbarUiTester::ToolbarUiTester(QWidget* parent)
    : QWidget(parent) {
  setWindowIcon(QIcon(":/Icons/spire-icon-48x48.png"));
  m_user_name = new TextBox("Test");
  m_user_name->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
  m_user_name->setFixedWidth(scale_width(100));
  m_mananger_check_box = new CheckBox();
  m_mananger_check_box->set_label(tr("Manager"));
  auto toolbar_button = make_label_button(tr("ToolbarWindow"));
  toolbar_button->connect_click_signal(
    std::bind_front(&ToolbarUiTester::on_toolbar_click, this));
  m_output = new QTextEdit();
  m_output->setReadOnly(true);
  auto left_layout = new QVBoxLayout();
  left_layout->setSpacing(scale_height(10));
  left_layout->addWidget(m_user_name);
  left_layout->addWidget(m_mananger_check_box);
  left_layout->addWidget(toolbar_button);
  left_layout->addStretch();
  auto layout = new QHBoxLayout(this);
  layout->setSpacing(scale_width(10));
  layout->addLayout(left_layout);
  layout->addWidget(m_output);
}

void ToolbarUiTester::closeEvent(QCloseEvent* event) {
  m_toolbar_window->close();
  QWidget::closeEvent(event);
}

void ToolbarUiTester::on_toolbar_click() {
  if(m_toolbar_window) {
    if(m_toolbar_window->isVisible()) {
      return;
    }
    m_toolbar_window.clear();
  }
  auto account = DirectoryEntry::MakeAccount(
    123, m_user_name->get_current()->get().toStdString());
  auto roles = AccountRoles();
  if(m_mananger_check_box->get_current()->get()) {
    roles.Set(AccountRole::MANAGER);
  }
  auto recently_closed_windows =
    std::make_shared<ArrayListModel<std::shared_ptr<WindowSettings>>>();
  auto pinned_blotters = std::make_shared<ArrayListModel<BlotterModel*>>();
  m_toolbar_window =
    new ToolbarWindow(account, roles, recently_closed_windows, pinned_blotters);
  m_toolbar_window->connect_open_signal(
    std::bind_front(&ToolbarUiTester::on_open, this));
  m_toolbar_window->connect_minimize_all_signal(
    std::bind_front(&ToolbarUiTester::on_minimize_all, this));
  m_toolbar_window->connect_restore_all_signal(
    std::bind_front(&ToolbarUiTester::on_restore_all, this));
  m_toolbar_window->connect_sign_out_signal(
    std::bind_front(&ToolbarUiTester::on_sign_out, this));
  m_toolbar_window->show();
  m_toolbar_window->move(x(), y() + frameGeometry().height() + 10);
}

void ToolbarUiTester::on_open(ToolbarWindow::WindowType window) {
  m_output->append(QString("Open: %1").arg(displayText(window)));
}

void ToolbarUiTester::on_minimize_all() {
  m_output->append("Minimize all");
}

void ToolbarUiTester::on_restore_all() {
  m_output->append("Restore all");
}

void ToolbarUiTester::on_sign_out() {
  m_output->append("Sign out");
}
