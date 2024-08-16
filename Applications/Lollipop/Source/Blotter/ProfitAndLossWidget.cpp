#include "Spire/Blotter/ProfitAndLossWidget.hpp"
#include <vector>
#include <QLabel>
#include "Spire/Blotter/ProfitAndLossEntry.hpp"
#include "Spire/Blotter/ProfitAndLossEntryModel.hpp"
#include "Spire/Blotter/ProfitAndLossModel.hpp"
#include "Spire/Blotter/BlotterModel.hpp"
#include "ui_ProfitAndLossWidget.h"

using namespace Beam;
using namespace Nexus;
using namespace Spire;
using namespace std;

ProfitAndLossWidget::ProfitAndLossWidget(QWidget* parent, Qt::WindowFlags flags)
    : QWidget(parent, flags),
      m_ui(std::make_unique<Ui_ProfitAndLossWidget>()) {
  m_ui->setupUi(this);
  auto emptyLabel = new QLabel(tr("No transactions have taken place."));
  emptyLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
  m_ui->m_area->layout()->addWidget(emptyLabel);
}

ProfitAndLossWidget::~ProfitAndLossWidget() {}

void ProfitAndLossWidget::SetModel(Ref<UserProfile> userProfile,
    Ref<ProfitAndLossModel> model) {
  vector<ProfitAndLossEntryModel*> entryModels;
  auto layout = static_cast<QVBoxLayout*>(m_ui->m_area->layout());
  for(auto i = 0; i < layout->count(); ++i) {
    auto item = layout->itemAt(i);
    if(dynamic_cast<QWidgetItem*>(item) != nullptr) {
      auto widget = static_cast<QWidgetItem*>(item)->widget();
      if(dynamic_cast<ProfitAndLossEntry*>(widget) != nullptr) {
        entryModels.push_back(
          &static_cast<ProfitAndLossEntry*>(widget)->GetModel());
      }
    }
  }
  while(!entryModels.empty()) {
    OnModelRemoved(*entryModels.back());
    entryModels.pop_back();
  }
  m_userProfile = userProfile.Get();
  m_modelAddedConnection = model->ConnectProfitAndLossEntryModelAddedSignal(
    std::bind(&ProfitAndLossWidget::OnModelAdded, this, std::placeholders::_1));
  m_modelRemovedConnection = model->ConnectProfitAndLossEntryModelRemovedSignal(
    std::bind(&ProfitAndLossWidget::OnModelRemoved, this,
    std::placeholders::_1));
}

void ProfitAndLossWidget::OnModelAdded(ProfitAndLossEntryModel& model) {
  auto entry = new ProfitAndLossEntry(Ref(*m_userProfile), Ref(model));
  auto layout = static_cast<QVBoxLayout*>(m_ui->m_area->layout());
  auto i = 0;
  while(i < layout->count() - 1) {
    auto item = layout->itemAt(i);
    if(dynamic_cast<QWidgetItem*>(item) != nullptr) {
      auto widget = static_cast<QWidgetItem*>(item)->widget();
      if(dynamic_cast<ProfitAndLossEntry*>(widget) != nullptr) {
        auto existingEntry = static_cast<ProfitAndLossEntry*>(widget);
        if(existingEntry->GetModel().GetCurrency().m_code >
            model.GetCurrency().m_code) {
          break;
        }
      }
    }
    ++i;
  }
  if(layout->count() == 1) {
    auto item = layout->itemAt(0);
    layout->removeItem(item);
    delete item->widget();
    delete item;
  }
  layout->insertWidget(i, entry);
  auto line = new QFrame(this);
  line->setFrameShape(QFrame::HLine);
  line->setFrameShadow(QFrame::Sunken);
  layout->insertWidget(i + 1, line);
  if(layout->count() == 2) {
    layout->addStretch(1);
  }
}

void ProfitAndLossWidget::OnModelRemoved(ProfitAndLossEntryModel& model) {
  auto layout = static_cast<QVBoxLayout*>(m_ui->m_area->layout());
  for(auto i = 0; i < layout->count(); ++i) {
    auto item = layout->itemAt(i);
    if(dynamic_cast<QWidgetItem*>(item) != nullptr) {
      auto widget = static_cast<QWidgetItem*>(item)->widget();
      if(dynamic_cast<ProfitAndLossEntry*>(widget) != nullptr) {
        auto entry = static_cast<ProfitAndLossEntry*>(widget);
        if(&entry->GetModel() == &model) {
          auto line  = layout->itemAt(i + 1);
          layout->removeItem(item);
          delete item->widget();
          delete item;
          layout->removeItem(line);
          delete line->widget();
          delete line;
          if(layout->count() == 1) {
            auto spacer = layout->itemAt(0);
            layout->removeItem(spacer);
            delete spacer->widget();
            delete spacer;
            auto emptyLabel = new QLabel(
              tr("No transactions have taken place."));
            emptyLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
            m_ui->m_area->layout()->addWidget(emptyLabel);
          }
          return;
        }
      }
    }
  }
}
