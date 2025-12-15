#include "Spire/BookView/BookViewPanel.hpp"
#include "Spire/BookView/BookViewModel.hpp"
#include "Spire/LegacyUI/CustomQtVariants.hpp"
#include "Spire/LegacyUI/UserProfile.hpp"
#include "ui_BookViewPanel.h"

using namespace Beam;
using namespace boost;
using namespace Nexus;
using namespace Spire;
using namespace Spire::LegacyUI;

namespace {
  const auto MPID_WIDTH = 60;
  const auto PRICE_WIDTH = 60;
  const auto SIZE_WIDTH = 40;
}

BookViewPanel::BookViewPanel(QWidget* parent, Qt::WindowFlags flags)
    : QWidget(parent, flags),
      m_ui(std::make_unique<Ui_BookViewPanel>()),
      m_topRow(-1),
      m_currentRow(-1) {
  m_eventHandler.emplace();
  m_ui->setupUi(this);
  m_ui->m_bookView->horizontalHeader()->setSectionsMovable(true);
  m_ui->m_bookView->verticalHeader()->setMinimumSectionSize(0);
  m_ui->m_bookView->setItemDelegate(
    new CustomVariantItemDelegate(Ref(*m_userProfile)));
  m_ui->m_bookView->horizontalHeader()->setSectionResizeMode(
    QHeaderView::ResizeToContents);
  m_ui->m_bboSeparatorLabel->setText(tr("N/A"));
}

void BookViewPanel::Initialize(Ref<UserProfile> userProfile,
    const BookViewProperties& properties, Side side) {
  m_userProfile = userProfile.get();
  m_itemDelegate.emplace(Ref(*m_userProfile));
  SetProperties(properties);
  m_side = side;
  DisconnectModel();
  m_model = std::make_unique<BookViewModel>(
    Ref(*m_userProfile), m_properties, Security(), side);
  m_ui->m_bookView->setModel(m_model.get());
  ConnectModel();
}

const Quote& BookViewPanel::GetBestQuote() const {
  return m_bestQuote;
}

QTableView& BookViewPanel::GetQuoteList() {
  return *m_ui->m_bookView;
}

void BookViewPanel::SetProperties(const BookViewProperties& properties) {
  m_properties = properties;
  m_ui->m_bookView->setFont(m_properties.GetBookQuoteFont());
  auto metrics = QFontMetrics(m_properties.GetBookQuoteFont());
  m_ui->m_bookView->verticalHeader()->setDefaultSectionSize(metrics.height());
  if(m_model) {
    m_model->SetProperties(m_properties);
  }
  m_ui->m_bookView->setShowGrid(properties.GetShowGrid());
  if(properties.GetShowBbo()) {
    m_ui->m_bboFrame->show();
  } else {
    m_ui->m_bboFrame->hide();
  }
}

void BookViewPanel::DisplaySecurity(const Security& security) {
  m_security = security;
  m_bestQuote = Quote();
  auto widths = std::vector<int>();
  for(auto i = 0; i < BookViewModel::COLUMN_COUNT; ++i) {
    widths.push_back(m_ui->m_bookView->columnWidth(i));
  }
  DisconnectModel();
  m_ui->m_bboSeparatorLabel->setText(tr("N/A"));
  m_ui->m_bboPriceLabel->clear();
  m_ui->m_bboQuantityLabel->clear();
  auto newModel = std::make_unique<BookViewModel>(
    Ref(*m_userProfile), m_properties, security, m_side);
  m_ui->m_bookView->setModel(newModel.get());
  m_model.swap(newModel);
  m_ui->m_bookView->setModel(m_model.get());
  for(auto i = 0; i < BookViewModel::COLUMN_COUNT; ++i) {
    m_ui->m_bookView->setColumnWidth(i, widths[i]);
  }
  ConnectModel();
  m_eventHandler.emplace();
  if(m_security == Security()) {
    return;
  }
  auto bboQuery = make_current_query(security);
  bboQuery.set_interruption_policy(InterruptionPolicy::IGNORE_CONTINUE);
  m_userProfile->GetClients().get_market_data_client().query(
    bboQuery, m_eventHandler->get_slot<BboQuote>(
      std::bind_front(&BookViewPanel::OnBbo, this, security)));
}

void BookViewPanel::resizeEvent(QResizeEvent* event) {
  QWidget::resizeEvent(event);
  m_ui->m_bookView->setColumnWidth(BookViewModel::MPID_COLUMN, MPID_WIDTH);
  m_ui->m_bookView->setColumnWidth(BookViewModel::PRICE_COLUMN, PRICE_WIDTH);
  m_ui->m_bookView->setColumnWidth(BookViewModel::SIZE_COLUMN, SIZE_WIDTH);
}

void BookViewPanel::ConnectModel() {
  m_rowsAboutToBeModifiedConnection = connect(m_model.get(),
    &BookViewModel::rowsAboutToBeInserted, this,
    &BookViewPanel::OnRowsAboutToBeModified);
  m_rowsModifiedConnection = connect(m_model.get(),
    &BookViewModel::rowsInserted, this, &BookViewPanel::OnRowsModified);
  m_rowsAboutToBeRemovedConnection = connect(m_model.get(),
    &BookViewModel::rowsAboutToBeRemoved, this,
    &BookViewPanel::OnRowsAboutToBeModified);
  m_rowsRemovedConnection = connect(m_model.get(), &BookViewModel::rowsRemoved,
    this, &BookViewPanel::OnRowsModified);
}

void BookViewPanel::DisconnectModel() {
  if(!m_model) {
    return;
  }
  disconnect(m_rowsRemovedConnection);
  disconnect(m_rowsAboutToBeRemovedConnection);
  disconnect(m_rowsModifiedConnection);
  disconnect(m_rowsAboutToBeModifiedConnection);
}

void BookViewPanel::OnBbo(const Security& security, const BboQuote& bbo) {
  if(security != m_security) {
    return;
  }
  if(m_side == Side::ASK) {
    m_bestQuote = bbo.m_ask;
  } else {
    m_bestQuote = bbo.m_bid;
  }
  if(m_bestQuote.m_price == Money::ZERO) {
    m_ui->m_bboSeparatorLabel->setText(tr("N/A"));
    m_ui->m_bboPriceLabel->clear();
    m_ui->m_bboQuantityLabel->clear();
    return;
  }
  m_ui->m_bboPriceLabel->setText(m_itemDelegate->displayText(
    QVariant::fromValue(m_bestQuote.m_price), QLocale()));
  m_ui->m_bboSeparatorLabel->setText("/");
  auto quantity = m_bestQuote.m_size;
  if(m_bestQuote.m_size == 0) {
    quantity = 0;
  } else {
    quantity = std::max<Quantity>(1, floor(m_bestQuote.m_size / 100));
  }
  m_ui->m_bboQuantityLabel->setText(
    QString::number(static_cast<int>(quantity)));
}

void BookViewPanel::OnRowsAboutToBeModified(
    const QModelIndex& parent, int start, int end) {
  m_topRow = m_ui->m_bookView->rowAt(0);
  auto currentIndex = m_ui->m_bookView->currentIndex();
  if(currentIndex.isValid()) {
    m_currentRow = currentIndex.row();
  } else {
    m_currentRow = -1;
  }
}

void BookViewPanel::OnRowsModified(
    const QModelIndex& parent, int start, int end) {
  if(m_currentRow != -1) {
    auto index = m_ui->m_bookView->model()->index(m_currentRow, 0);
    if(index.isValid() && index != m_ui->m_bookView->currentIndex()) {
      m_ui->m_bookView->setCurrentIndex(index);
    }
  }
  if(m_topRow != -1) {
    auto index = m_ui->m_bookView->model()->index(m_topRow, 0);
    if(index.isValid() && m_ui->m_bookView->rowAt(0) != m_topRow) {
      m_ui->m_bookView->scrollTo(index, QAbstractItemView::PositionAtTop);
    }
  }
}
