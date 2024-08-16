#ifndef SPIRE_SELECTIONLISTINPUTDIALOG_HPP
#define SPIRE_SELECTIONLISTINPUTDIALOG_HPP
#include <memory>
#include <QDialog>

class Ui_SelectionListInputDialog;

namespace Spire {

  /*! \class SelectionListInputDialog
      \brief Prompts the user to select a list of strings.
   */
  class SelectionListInputDialog : public QDialog {
    public:

      //! Constructs a SelectionListInputDialog.
      /*!
        \param availableList The list of available items.
        \param selectedList The list of selected items.
        \param parent The parent widget.
        \param flags Qt flags passed to the parent widget.
      */
      SelectionListInputDialog(const std::vector<std::string>& availableList,
        const std::vector<std::string>& selectedList, QWidget* parent = nullptr,
        Qt::WindowFlags flags = 0);

      //! Returns the list of selected items.
      std::vector<std::string> GetSelection() const;

    private:
      std::unique_ptr<Ui_SelectionListInputDialog> m_ui;
      std::vector<std::string> m_availableList;

      void OnAddItems();
      void OnRemoveItems();
  };
}

#endif
