#include "Spire/Catalog/CatalogTabWidget.hpp"
#include <QApplication>
#include <QEvent>
#include <QMenu>
#include <QMouseEvent>
#include <QTabBar>
#include "Spire/Catalog/CatalogWindow.hpp"
#include "Spire/Catalog/CatalogTabModel.hpp"
#include "Spire/Catalog/CatalogTabView.hpp"
#include "Spire/LegacyUI/LineInputDialog.hpp"
#include "Spire/LegacyUI/UserProfile.hpp"

using namespace Beam;
using namespace boost;
using namespace Spire;
using namespace Spire::LegacyUI;
using namespace std;

CatalogTabWidget::CatalogTabWidget(QWidget* parent)
    : QTabWidget(parent),
      m_parent(static_cast<CatalogWindow*>(parent)),
      m_fixedTabClicked(false),
      m_addTabClicked(false) {
  m_tabMenu = new QMenu(this);
  tabBar()->installEventFilter(this);
  m_renameAction = new QAction(tr("Rename"), m_tabMenu);
  m_renameAction->setEnabled(true);
  m_tabMenu->addAction(m_renameAction); 
  connect(m_renameAction, &QAction::triggered, this,
    &CatalogTabWidget::OnRenameAction);
  QMenu* deleteSubMenu = new QMenu(tr("Delete"), m_tabMenu);
  m_deleteAction = new QAction(tr("Confirm"), deleteSubMenu);
  m_deleteAction->setEnabled(true);
  deleteSubMenu->addAction(m_deleteAction);
  connect(m_deleteAction, &QAction::triggered, this,
    &CatalogTabWidget::OnDeleteAction);
  m_tabMenu->addMenu(deleteSubMenu);
}

bool CatalogTabWidget::eventFilter(QObject* object, QEvent* event) {
  if(object == tabBar()) {
    if(event->type() == QEvent::MouseButtonPress) {
      QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
      if(mouseEvent->button() == Qt::LeftButton) {
        if(OnLeftMousePress(mouseEvent)) {
          return true;
        }
      } else if(mouseEvent->button() == Qt::RightButton) {
        if(OnRightMousePress(mouseEvent)) {
          return true;
        }
      }
    } else if(event->type() == QEvent::MouseButtonRelease) {
      QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
      if(OnMouseRelease(mouseEvent)) {
        return true;
      }
    } else if(event->type() == QEvent::MouseMove) {
      QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
      if(OnMouseMove(mouseEvent)) {
        return true;
      }
    }
  }
  return QTabWidget::eventFilter(object, event);
}

bool CatalogTabWidget::OnLeftMousePress(QMouseEvent* event) {
  m_selectedTab = tabBar()->tabAt(event->pos());
  m_addTabClicked = m_selectedTab == count() - 1;
  m_fixedTabClicked = m_addTabClicked || m_selectedTab == 0;
  if(!m_fixedTabClicked) {
    m_clickPosition = event->pos();
    m_clickedTabRectangle = tabBar()->tabRect(tabBar()->tabAt(m_clickPosition));
  }
  return m_addTabClicked;
}

bool CatalogTabWidget::OnRightMousePress(QMouseEvent* event) {
  m_selectedTab = tabBar()->tabAt(event->pos());
  if(m_selectedTab != -1 && m_selectedTab != 0 &&
      m_selectedTab != tabBar()->count() - 1) {
    m_tabMenu->exec(event->globalPos());
    return true;
  }
  return false;
}

bool CatalogTabWidget::OnMouseRelease(QMouseEvent* event) {
  if(m_addTabClicked && tabBar()->tabAt(event->pos()) == count() - 1) {
    m_addTabClicked = false;
    LineInputDialog newTabDialog("New Catalog Tab", "Name:", "New Tab", this);
    if(newTabDialog.exec() == QDialog::Rejected) {
      return true;
    }
    unique_ptr<CatalogTabModel> model = std::make_unique<CatalogTabModel>();
    model->SetName(newTabDialog.GetInput());
    m_parent->GetSettings().Add(std::move(model));
    return true;
  }
  m_addTabClicked = false;
  return false;
}

bool CatalogTabWidget::OnMouseMove(QMouseEvent* event) {
  if(event->buttons() & Qt::LeftButton) {
    if(m_fixedTabClicked || event->pos().x() + m_clickedTabRectangle.right() -
        m_clickPosition.x() > tabBar()->tabRect(count() - 1).left() ||
        event->pos().x() - (m_clickPosition.x() -
        m_clickedTabRectangle.left()) < tabBar()->tabRect(0).right()) {
      return true;
    }
  }
  return false;
}

void CatalogTabWidget::OnRenameAction() {
  CatalogTabView* view = static_cast<CatalogTabView*>(widget(m_selectedTab));
  LineInputDialog renameTabDialog("Rename Tab", "Name:",
    view->GetModel().GetName(), this);
  if(renameTabDialog.exec() == QDialog::Rejected) {
    return;
  }
  view->GetModel().SetName(renameTabDialog.GetInput());
}

void CatalogTabWidget::OnDeleteAction() {
  CatalogTabView* view = static_cast<CatalogTabView*>(widget(m_selectedTab));
  m_parent->GetSettings().Remove(view->GetModel());
}
