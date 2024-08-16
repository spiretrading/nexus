#ifndef SPIRE_VALUE_LIST_INPUT_DIALOG_HPP
#define SPIRE_VALUE_LIST_INPUT_DIALOG_HPP
#include <memory>
#include <typeinfo>
#include <vector>
#include <Beam/Pointers/Ref.hpp>
#include <boost/variant/variant.hpp>
#include <QDialog>
#include "Nexus/Definitions/Security.hpp"
#include "Spire/Spire/Spire.hpp"

class Ui_ValueListInputDialog;

namespace Spire {

  /** Shows a list of values that can be edited. */
  class ValueListInputDialog : public QDialog {
    public:

      /** The types that can be edited by this dialog. */
      using Value = boost::variant<Nexus::Security>;

      /**
       * Constructs a ValueListInputDialog.
       * @param userProfile The user's profile.
       * @param type The type to edit.
       * @param parent The parent widget.
       * @param flags Qt flags passed to the widget.
       */
      ValueListInputDialog(Beam::Ref<UserProfile> userProfile,
        const std::type_info& type, QWidget* parent = nullptr,
        Qt::WindowFlags flags = Qt::WindowFlags());

      /**
       * Constructs a ValueListInputDialog.
       * @param userProfile The user's profile.
       * @param type The type to edit.
       * @param values The set of values to populate the dialog with.
       * @param parent The parent widget.
       * @param flags Qt flags passed to the widget.
       */
      ValueListInputDialog(Beam::Ref<UserProfile> userProfile,
        const std::type_info& type, const std::vector<Value>& values,
        QWidget* parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());

      ~ValueListInputDialog() override;

      /** Returns the list of values. */
      const std::vector<Value>& GetValues() const;

    protected:
      bool eventFilter(QObject* receiver, QEvent* event) override;

    private:
      std::unique_ptr<Ui_ValueListInputDialog> m_ui;
      UserProfile* m_userProfile;
      const std::type_info* m_type;
      std::vector<Value> m_values;

      void ActivateRow(int row, QKeyEvent* event);
      void AppendItem(const Value& value);
      void OnAccept();
      void OnAddItemAction();
      void OnLoadFileAction();
      void OnDeleteItemAction();
      void OnCellActivated(int row, int column);
  };
}

#endif
