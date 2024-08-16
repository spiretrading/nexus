#ifndef SPIRE_CATALOGTABWIDGET_HPP
#define SPIRE_CATALOGTABWIDGET_HPP
#include <QTabWidget>
#include "Spire/Catalog/Catalog.hpp"

class QMenu;

namespace Spire {

  /*! \class CatalogTabWidget
      \brief Customization of the QTabWidget used within the CatalogWindow.
   */
  class CatalogTabWidget : public QTabWidget {
    public:

      //! Constructs a CatalogTabWidget.
      /*!
        \param parent The parent widget.
      */
      CatalogTabWidget(QWidget* parent = nullptr);

    protected:
      virtual bool eventFilter(QObject* object, QEvent* event);

    private:
      friend class CatalogWindow;
      CatalogWindow* m_parent;
      bool m_fixedTabClicked;
      bool m_addTabClicked;
      QRect m_clickedTabRectangle;
      QPoint m_clickPosition;
      QAction* m_renameAction;
      QAction* m_deleteAction;
      int m_selectedTab;
      QMenu* m_tabMenu;

      bool OnLeftMousePress(QMouseEvent* event);
      bool OnRightMousePress(QMouseEvent* event);
      bool OnMouseRelease(QMouseEvent* event);
      bool OnMouseMove(QMouseEvent* event);
      void OnRenameAction();
      void OnDeleteAction();
  };
}

#endif
