#include "Spire/InputWidgets/TickerInputDialog.hpp"
#include <QHeaderView>
#include <QKeyEvent>
#include <QScrollBar>
#include <QTableView>
#include "Spire/UI/CustomQtVariants.hpp"
#include "Spire/UI/UserProfile.hpp"
#include "Spire/Utilities/TickerInfoCompleter.hpp"
#include "Spire/Utilities/TickerInfoModel.hpp"
#include "ui_SecurityInputDialog.h"

using namespace Beam;
using namespace boost;
using namespace Nexus;
using namespace Spire;
using namespace Spire::UI;

void Spire::ShowTickerInputDialog(Ref<UserProfile> userProfile,
    const variant<std::string, Ticker>& initialValue, QWidget* parent,
    std::function<void (optional<Ticker>)> onResult) {
  auto dialog = [&] {
    if(auto text = get<std::string>(&initialValue)) {
      return new TickerInputDialog(Ref(userProfile), *text, parent);
    }
    return new TickerInputDialog(
      Ref(userProfile), get<Ticker>(initialValue), parent);
  }();
  dialog->setAttribute(Qt::WA_DeleteOnClose);
  QObject::connect(dialog, &TickerInputDialog::finished, parent,
    [=] (auto result) {
      if(result == QDialog::Rejected) {
        onResult(none);
      } else {
        onResult(dialog->GetTicker());
      }
    });
  dialog->show();
}

TickerInputDialog::TickerInputDialog(Ref<UserProfile> userProfile,
    const Ticker& initial, QWidget* parent, Qt::WindowFlags flags)
    : QDialog(parent, flags),
      m_ui(std::make_unique<Ui_SecurityInputDialog>()),
      m_userProfile(userProfile.get()) {
  Initialize();
  m_ui->m_input->setText(displayText(initial));
  if(parent) {
    parent->installEventFilter(this);
  }
}

TickerInputDialog::TickerInputDialog(Ref<UserProfile> userProfile,
    const std::string& text, QWidget* parent, Qt::WindowFlags flags)
    : QDialog(parent, flags),
      m_ui(std::make_unique<Ui_SecurityInputDialog>()),
      m_userProfile(userProfile.get()) {
  Initialize();
  m_ui->m_input->setText(QString::fromStdString(text));
  if(parent) {
    parent->installEventFilter(this);
  }
}

TickerInputDialog::~TickerInputDialog() = default;

Ticker TickerInputDialog::GetTicker() const {
  auto source = m_ui->m_input->text().toUpper().toStdString();
  return parse_ticker(source, m_userProfile->GetVenueDatabase());
}

QLineEdit& TickerInputDialog::GetSymbolInput() {
  return *m_ui->m_input;
}

bool TickerInputDialog::eventFilter(QObject* receiver, QEvent* event) {
  if(isVisible() && event->type() == QEvent::KeyPress) {
    auto forwardEvent = QKeyEvent(static_cast<QKeyEvent&>(*event));
    QCoreApplication::sendEvent(m_ui->m_input, &forwardEvent);
    return true;
  }
  return QDialog::eventFilter(receiver, event);
}

void TickerInputDialog::Initialize() {
  m_ui->setupUi(this);
  setFixedSize(width(), height());
  m_model = std::make_unique<TickerInfoModel>(Ref(*m_userProfile));
  m_completer = new TickerInfoCompleter(m_model.get(), this);
  m_completer->setWrapAround(false);
  m_completerPopup = new QTableView();
  m_completerPopup->setShowGrid(false);
  m_completerPopup->verticalHeader()->hide();
  m_completerPopup->horizontalHeader()->hide();
  m_completerPopup->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  auto metrics = QFontMetrics(m_completerPopup->font());
  m_completerPopup->verticalHeader()->setDefaultSectionSize(
    metrics.height() + 4);
  m_completerPopup->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_completerPopup->setSelectionMode(QAbstractItemView::SingleSelection);
  m_completer->setPopup(m_completerPopup);
  m_completer->setCaseSensitivity(Qt::CaseInsensitive);
  m_completer->setCompletionRole(Qt::DisplayRole);
  m_ui->m_input->setCompleter(m_completer);
  m_completerPopup->horizontalHeader()->setSectionResizeMode(
    QHeaderView::ResizeToContents);
  m_completerPopup->horizontalHeader()->stretchLastSection();
  connect(m_ui->m_input, &QLineEdit::textChanged, this,
    &TickerInputDialog::OnInputEdited);
  connect(m_ui->m_input, &QLineEdit::returnPressed, this,
    &TickerInputDialog::accept);
  connect(m_completer->model(), &QAbstractItemModel::rowsInserted, this,
    &TickerInputDialog::OnRowsAddedRemoved);
  connect(m_completer->model(), &QAbstractItemModel::rowsRemoved, this,
    &TickerInputDialog::OnRowsAddedRemoved);
  connect(m_completer->model(),&QAbstractItemModel::dataChanged, this,
    &TickerInputDialog::OnDataChanged);
}

void TickerInputDialog::AdjustCompleterSize() {
  m_completerPopup->resizeColumnsToContents();
  auto width = 2 + m_completerPopup->verticalHeader()->width();
  for(auto i = 0; i < TickerInfoModel::COLUMN_COUNT; ++i) {
    width += 4 + m_completerPopup->columnWidth(i);
  }
  width = std::max(width, m_ui->m_input->width());
  m_completerPopup->setMinimumWidth(width);
  m_completerPopup->setMaximumWidth(width);
  auto height = 4 + m_completerPopup->horizontalHeader()->height();
  for(auto i = 0; i < std::min(7, m_model->rowCount(QModelIndex())); ++i) {
    height += m_completerPopup->rowHeight(i);
  }
  m_completerPopup->setMinimumHeight(height);
  m_completerPopup->setMaximumHeight(height);
}

void TickerInputDialog::OnInputEdited(const QString& text) {
  m_model->Search(text.toStdString());
}

void TickerInputDialog::OnRowsAddedRemoved(
    const QModelIndex& parent, int start, int end) {
  m_completer->complete();
  AdjustCompleterSize();
}

void TickerInputDialog::OnDataChanged(
    const QModelIndex& topLeft, const QModelIndex& bottomRight) {
  m_completer->complete();
  AdjustCompleterSize();
}
