#ifndef SPIRE_CATALOGICONSELECTIONDIALOG_HPP
#define SPIRE_CATALOGICONSELECTIONDIALOG_HPP
#include <memory>
#include <QDialog>
#include "Spire/Catalog/Catalog.hpp"

class QListWidgetItem;
class Ui_CatalogIconSelectionDialog;

namespace Spire {

  /*! \class CatalogIconSelectionDialog
      \brief Allows selection of a CatalogEntry's icon.
   */
  class CatalogIconSelectionDialog : public QDialog {
    public:

      //! Constructs a CatalogIconSelectionDialog.
      /*!
        \param parent The parent widget.
        \param flags Qt flags passed to the parent widget.
      */
      CatalogIconSelectionDialog(QWidget* parent = nullptr,
        Qt::WindowFlags flags = Qt::WindowFlags());

      virtual ~CatalogIconSelectionDialog();

      //! Returns the path to the selection icon.
      const std::string& GetIconPath() const;

    private:
      std::unique_ptr<Ui_CatalogIconSelectionDialog> m_ui;
      std::string m_iconPath;

      void OnActivated(QListWidgetItem* item);
      void OnAccept();
  };
}

#endif
