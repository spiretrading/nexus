#ifndef SPIRE_SECURITY_INPUT_LINE_EDIT_HPP
#define SPIRE_SECURITY_INPUT_LINE_EDIT_HPP
#include "Beam/Pointers/Ref.hpp"
#include "Nexus/Definitions/Security.hpp"
#include "Spire/SecurityInput/SecurityInput.hpp"
#include "Spire/Spire/QtPromise.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/DropDownList.hpp"
#include "Spire/Ui/TextInputWidget.hpp"

namespace Spire {

  //! Displays a line edit with a drop down list of securities.
  class SecurityInputLineEdit : public TextInputWidget {
    public:

      //! Constructs a SecurityInputLineEdit.
      /*
        \param initial_text The initial text to display.
        \param model The security input model.
        \param is_icon_visible True if the search icon should be displayed.
        \param parent The parent widget.
      */
      SecurityInputLineEdit(const QString& initial_text,
        Beam::Ref<SecurityInputModel> model, bool is_icon_visible,
        QWidget* parent = nullptr);

      //! Constructs a SecurityInputLineEdit.
      /*
        \param security Sets the security that is committed if the line edit
                        is submitted with an empty string.
        \param model The security input model.
        \param is_icon_visible True if the search icon should be displayed.
        \param parent The parent widget.
      */
      SecurityInputLineEdit(Nexus::Security security,
        Beam::Ref<SecurityInputModel> model, bool is_icon_visible,
        QWidget* parent = nullptr);

      //! Returns the selected security, or an empty Security if there was no
      //! selection.
      const Nexus::Security& get_security() const;

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      void focusInEvent(QFocusEvent* event) override;
      void keyPressEvent(QKeyEvent* event) override;
      void paintEvent(QPaintEvent* event) override;
      void showEvent(QShowEvent* event) override;
  
    private:
      SecurityInputModel* m_model;
      DropDownList* m_securities;
      QtPromise<std::vector<Nexus::SecurityInfo>> m_completions;
      Nexus::Security m_security;
      bool m_is_icon_visible;
      CustomVariantItemDelegate m_item_delegate;
      bool m_is_suggestion_disabled;
      Qt::Key m_last_key;

      void on_activated(const Nexus::Security& security);
      void on_commit(const Nexus::Security& security);
      void on_text_edited();
  };
}

#endif
