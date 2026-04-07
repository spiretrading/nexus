#include "Spire/KeyBindings/TaskKeyBindingsDialog.hpp"
#include <QHeaderView>
#include <QStandardItemModel>
#include <QTableView>
#include "Spire/Canvas/Common/CanvasNode.hpp"
#include "Spire/Canvas/Types/OrderReferenceType.hpp"
#include "Spire/CanvasView/CanvasNodeModel.hpp"
#include "Spire/UI/CanvasWindow.hpp"
#include "Spire/UI/UserProfile.hpp"
#include "ui_TaskKeyBindingsDialog.h"

using namespace Beam;
using namespace boost;
using namespace Nexus;
using namespace Spire;
using namespace Spire::UI;
using namespace std;

namespace {
  const int F_KEY_COUNT = 12;
}

TaskKeyBindingsDialog::TaskKeyBindingsDialog(Ref<UserProfile> userProfile,
    QWidget* parent, Qt::WindowFlags flags)
    : QDialog(parent, flags),
      m_ui(std::make_unique<Ui_TaskKeyBindingsDialog>()),
      m_userProfile(userProfile.get()),
      m_keyBindings(m_userProfile->GetKeyBindings()) {
  m_ui->setupUi(this);
  auto keyComboBoxView = new QTableView(m_ui->m_keyComboBox);
  keyComboBoxView->setFont(m_ui->m_keyComboBox->font());
  QFontMetrics metrics(keyComboBoxView->font());
  keyComboBoxView->verticalHeader()->setDefaultSectionSize(
    metrics.height() + 4);
  keyComboBoxView->setSelectionBehavior(QAbstractItemView::SelectRows);
  keyComboBoxView->setSelectionMode(QAbstractItemView::SingleSelection);
  keyComboBoxView->horizontalHeader()->hide();
  keyComboBoxView->verticalHeader()->hide();
  keyComboBoxView->setShowGrid(false);
  keyComboBoxView->setModel(m_ui->m_keyComboBox->model());
  m_ui->m_keyComboBox->setView(keyComboBoxView);
  m_ui->m_venueComboBox->addItem(tr("All"));
  for(const auto& venue : m_userProfile->GetVenueDatabase().get_entries()) {
    m_venues.push_back(venue.m_venue);
    m_ui->m_venueComboBox->addItem(QString::fromStdString(
      venue.m_display_name));
  }
  auto keyComboBoxModel = static_cast<QStandardItemModel*>(
    m_ui->m_keyComboBox->model());
  keyComboBoxModel->setRowCount(F_KEY_COUNT);
  keyComboBoxModel->setColumnCount(2);
  keyComboBoxView->setColumnWidth(0, keyComboBoxView->width() / 2);
  keyComboBoxView->setColumnWidth(1, keyComboBoxView->width() / 2);
  keyComboBoxView->horizontalHeader()->setStretchLastSection(true);
  connect(m_ui->m_taskNameInput, &QLineEdit::textChanged, this,
    &TaskKeyBindingsDialog::OnTaskNameChanged);
  connect(m_ui->m_okButton, &QPushButton::clicked, this,
    &TaskKeyBindingsDialog::OnOkButton);
  connect(m_ui->m_cancelButton, &QPushButton::clicked, this,
    &TaskKeyBindingsDialog::reject);
  connect(m_ui->m_applyButton, &QPushButton::clicked, this,
    &TaskKeyBindingsDialog::OnApply);
  connect(m_ui->m_venueComboBox, static_cast<void (QComboBox::*)(int)>(
    &QComboBox::currentIndexChanged), this,
    &TaskKeyBindingsDialog::OnVenueChanged);
  connect(m_ui->m_keyComboBox, static_cast<void (QComboBox::*)(int)>(
    &QComboBox::currentIndexChanged), this,
    &TaskKeyBindingsDialog::OnKeyChanged);
  m_ui->m_keyComboBox->view()->installEventFilter(this);
  m_ui->m_venueComboBox->setCurrentIndex(0);
  m_ui->m_keyComboBox->setCurrentIndex(0);
  OnVenueChanged(0);
  OnKeyChanged(0);
}

TaskKeyBindingsDialog::~TaskKeyBindingsDialog() {}

bool TaskKeyBindingsDialog::eventFilter(QObject* object, QEvent* event) {
  if(object == m_ui->m_keyComboBox->view()) {
    if(event->type() == QEvent::FocusIn) {
      auto selection =
        m_ui->m_keyComboBox->view()->selectionModel()->selection();
      for(const auto& currentSelection : selection) {
        auto topLeft = currentSelection.topLeft();
        auto bottomRight = currentSelection.bottomRight();
        if(bottomRight.column() == 0) {
          auto modifiedBottomRight = m_ui->m_keyComboBox->model()->index(
            bottomRight.row(), 1);
          m_ui->m_keyComboBox->view()->selectionModel()->select(
            QItemSelection(topLeft, modifiedBottomRight),
            QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
        } else if(topLeft.column() == 1) {
          auto modifiedTopLeft = m_ui->m_keyComboBox->model()->index(
            topLeft.row(), 0);
          m_ui->m_keyComboBox->view()->selectionModel()->select(
            QItemSelection(modifiedTopLeft, bottomRight),
            QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
        }
      }
      return QWidget::eventFilter(object, event);
    }
  }
  return QWidget::eventFilter(object, event);
}

QKeySequence TaskKeyBindingsDialog::GetCurrentKeyBinding() const {
  return QKeySequence(Qt::Key_F1 + m_currentIndex);
}

void TaskKeyBindingsDialog::PopulateComboBox() {
  auto currentKeyBindingIndex = m_ui->m_keyComboBox->currentIndex();
  auto model = static_cast<QStandardItemModel*>(m_ui->m_keyComboBox->model());
  for(auto i = 0; i < F_KEY_COUNT; ++i) {
    QKeySequence binding(Qt::Key_F1 + i);
    auto keyItem = new QStandardItem(binding.toString());
    auto taskBinding = m_keyBindings.GetTaskFromBinding(
      m_currentVenue, binding);
    QStandardItem* nameItem;
    if(taskBinding.is_initialized()) {
      nameItem = new QStandardItem(QString::fromStdString(taskBinding->m_name));
    } else {
      nameItem = new QStandardItem(QString("None"));
    }
    model->setItem(i, 0, keyItem);
    model->setItem(i, 1, nameItem);
  }
  m_ui->m_keyComboBox->blockSignals(true);
  m_ui->m_keyComboBox->setCurrentIndex(currentKeyBindingIndex);
  m_ui->m_keyComboBox->blockSignals(false);
}

void TaskKeyBindingsDialog::Commit() {
  auto canvasWindow = dynamic_cast<CanvasWindow*>(
    m_ui->m_canvasScrollArea->widget());
  if(canvasWindow == nullptr) {
    return;
  }
  auto roots = canvasWindow->GetCanvasNodeModel().GetRoots();
  boost::optional<KeyBindings::TaskBinding> taskBinding;
  for(const auto& root : roots) {
    if(root->GetType().GetCompatibility(OrderReferenceType::GetInstance()) ==
        CanvasType::Compatibility::EQUAL) {
      taskBinding = KeyBindings::TaskBinding();
      auto name = m_ui->m_taskNameInput->text().trimmed();
      if(name.isEmpty()) {
        taskBinding->m_name = root->GetText();
      } else {
        taskBinding->m_name = name.toStdString();
      }
      taskBinding->m_node = CanvasNode::Clone(*root);
      break;
    }
  }
  if(taskBinding.is_initialized()) {
    m_keyBindings.SetTaskBinding(m_currentVenue, GetCurrentKeyBinding(),
      *taskBinding);
  } else {
    m_keyBindings.ResetTaskBinding(m_currentVenue, GetCurrentKeyBinding());
  }
  PopulateComboBox();
}

void TaskKeyBindingsDialog::OnVenueChanged(int index) {
  Commit();
  if(index == 0) {
    m_currentVenue = Venue();
  } else {
    m_currentVenue = m_venues[index - 1];
  }
  PopulateComboBox();
}

void TaskKeyBindingsDialog::OnKeyChanged(int index) {
  if(index < 0 || index >= m_ui->m_keyComboBox->count() ||
      m_currentIndex == index) {
    return;
  }
  Commit();
  m_currentIndex = index;
  auto canvas = new CanvasWindow(Ref(*m_userProfile));
  m_ui->m_canvasScrollArea->setWidget(canvas);
  auto taskBinding = m_keyBindings.GetTaskFromBinding(m_currentVenue,
    GetCurrentKeyBinding());
  if(!taskBinding.is_initialized()) {
    m_ui->m_taskNameInput->clear();
    return;
  }
  canvas->GetCanvasNodeModel().Add(CanvasNodeModel::Coordinate(0, 0),
    *taskBinding->m_node);
  m_ui->m_taskNameInput->setText(QString::fromStdString(taskBinding->m_name));
}

void TaskKeyBindingsDialog::OnTaskNameChanged(const QString& text) {
  auto name = text.trimmed();
  QStandardItem* nameItem;
  if(!text.isEmpty()) {
    nameItem = new QStandardItem(text);
  } else {
    nameItem = new QStandardItem(QString("None"));
  }
  auto model = static_cast<QStandardItemModel*>(m_ui->m_keyComboBox->model());
  model->setItem(m_currentIndex, 1, nameItem);
}

void TaskKeyBindingsDialog::OnOkButton() {
  OnApply();
  accept();
}

void TaskKeyBindingsDialog::OnApply() {
  Commit();
  m_userProfile->SetKeyBindings(m_keyBindings);
  m_ui->m_applyButton->setEnabled(false);
}
