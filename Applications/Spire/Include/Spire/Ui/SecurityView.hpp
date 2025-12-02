#ifndef SPIRE_SECURITY_VIEW_HPP
#define SPIRE_SECURITY_VIEW_HPP
#include <vector>
#include <QStackedWidget>
#include "Spire/Ui/SecurityDialog.hpp"

namespace Spire {

  /**
   * Displays securities and provides the functionality to quickly search and
   * cycle through securities.
   */
  class SecurityView : public QWidget {
    public:

      /** A ValueModel over a Nexus::Security. */
      using CurrentModel = ValueModel<Nexus::Security>;

      /**
       * Constructs a SecurityView using a default local model.
       * @param securities The set of securities that can be queried.
       * @param body The component that represents the current security.
       * @param parent The parent widget.
       */
      SecurityView(std::shared_ptr<SecurityInfoQueryModel> securities,
        QWidget& body, QWidget* parent = nullptr);

      /**
       * Constructs a SecurityView.
       * @param securities The set of securities that can be queried.
       * @param current The current value's model.
       * @param body The component that represents the current security.
       * @param parent The parent widget.
       */
      SecurityView(std::shared_ptr<SecurityInfoQueryModel> securities,
        std::shared_ptr<CurrentModel> current, QWidget& body,
        QWidget* parent = nullptr);

      /** Returns the set of securities that can be queried. */
      const std::shared_ptr<SecurityInfoQueryModel>& get_securities() const;

      /** Returns the current security displayed. */
      const std::shared_ptr<CurrentModel>& get_current() const;
    
      /** Returns the body. */
      const QWidget& get_body() const;

      /** Returns the body. */
      QWidget& get_body();

    protected:
      void keyPressEvent(QKeyEvent* event) override;

    private:
      SecurityDialog m_security_dialog;
      std::shared_ptr<CurrentModel> m_current;
      QWidget* m_body;
      QStackedWidget* m_layers;
      std::vector<Nexus::Security> m_securities;
      int m_current_index;

      void on_submit(const Nexus::Security& security);
  };
}

#endif
