#include "Spire/InputWidgets/SecurityInputDialog.hpp"
#include <QHeaderView>
#include <QScrollBar>
#include <QTableView>
#include "Nexus/Definitions/SecuritySet.hpp"
#include "Spire/Spire/UserProfile.hpp"
#include "Spire/Utilities/SecurityInfoCompleter.hpp"
#include "Spire/Utilities/SecurityInfoModel.hpp"
#include "ui_SecurityInputDialog.h"

using namespace Beam;
using namespace boost;
using namespace Nexus;
using namespace Spire;
using namespace Spire::UI;
using namespace std;

SecurityInputDialog::SecurityInputDialog(RefType<UserProfile> userProfile,
    const Security& initial, QWidget* parent, Qt::WindowFlags flags)
    : QDialog(parent, flags),
      m_ui(std::make_unique<Ui_SecurityInputDialog>()),
      m_userProfile(userProfile.Get()) {
  Initialize();
  auto text = ToWildCardString(initial, m_userProfile->GetMarketDatabase(),
    m_userProfile->GetCountryDatabase());
  m_ui->m_input->setText(QString::fromStdString(text));
}

SecurityInputDialog::SecurityInputDialog(RefType<UserProfile> userProfile,
    const string& text, QWidget* parent, Qt::WindowFlags flags)
    : QDialog(parent, flags),
      m_ui(std::make_unique<Ui_SecurityInputDialog>()),
      m_userProfile(userProfile.Get()) {
  Initialize();
  m_ui->m_input->setText(QString::fromStdString(text));
}

SecurityInputDialog::~SecurityInputDialog() {}

Security SecurityInputDialog::GetSecurity(bool supportWildCards) const {
  auto source = m_ui->m_input->text().toUpper().toStdString();
  auto security =
    [&] {
      if(supportWildCards) {
        auto optionalSecurity = ParseWildCardSecurity(source,
          m_userProfile->GetMarketDatabase(),
          m_userProfile->GetCountryDatabase());
        if(optionalSecurity.is_initialized()) {
          return *optionalSecurity;
        }
        return Security{};
      } else {
        return ParseSecurity(source, m_userProfile->GetMarketDatabase());
      }
    }();
  return security;
}

QLineEdit& SecurityInputDialog::GetSymbolInput() {
  return *m_ui->m_input;
}

void SecurityInputDialog::Initialize() {
  m_ui->setupUi(this);
  setFixedSize(width(), height());
  m_model = std::make_unique<SecurityInfoModel>(Ref(*m_userProfile));
  m_completer = new SecurityInfoCompleter(m_model.get(), this);
  m_completer->setWrapAround(false);
  m_completerPopup = new QTableView();
  m_completerPopup->setShowGrid(false);
  m_completerPopup->verticalHeader()->hide();
  m_completerPopup->horizontalHeader()->hide();
  m_completerPopup->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  QFontMetrics metrics(m_completerPopup->font());
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
    &SecurityInputDialog::OnInputEdited);
  connect(m_ui->m_input, &QLineEdit::returnPressed, this,
    &SecurityInputDialog::accept);
  connect(m_completer->model(), &QAbstractItemModel::rowsInserted, this,
    &SecurityInputDialog::OnRowsAddedRemoved);
  connect(m_completer->model(), &QAbstractItemModel::rowsRemoved, this,
    &SecurityInputDialog::OnRowsAddedRemoved);
  connect(m_completer->model(),&QAbstractItemModel::dataChanged, this,
    &SecurityInputDialog::OnDataChanged);
}

void SecurityInputDialog::AdjustCompleterSize() {
  m_completerPopup->resizeColumnsToContents();
  int width = 2 + m_completerPopup->verticalHeader()->width();
  for(int i = 0; i < SecurityInfoModel::COLUMN_COUNT; ++i) {
    width += 4 + m_completerPopup->columnWidth(i);
  }
  width = max(width, m_ui->m_input->width());
  m_completerPopup->setMinimumWidth(width);
  m_completerPopup->setMaximumWidth(width);
  int height = 4 + m_completerPopup->horizontalHeader()->height();
  for(int i = 0; i < min(7, m_model->rowCount(QModelIndex())); ++i) {
    height += m_completerPopup->rowHeight(i);
  }
  m_completerPopup->setMinimumHeight(height);
  m_completerPopup->setMaximumHeight(height);
}

void SecurityInputDialog::OnInputEdited(const QString& text) {
  m_model->Search(text.toStdString());
}

void SecurityInputDialog::OnRowsAddedRemoved(const QModelIndex& parent,
    int start, int end) {
  m_completer->complete();
  AdjustCompleterSize();
}

void SecurityInputDialog::OnDataChanged(const QModelIndex& topLeft,
    const QModelIndex& bottomRight) {
  m_completer->complete();
  AdjustCompleterSize();
}
