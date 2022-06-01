#ifndef SPIRE_SECURITY_VIEW_HPP
#define SPIRE_SECURITY_VIEW_HPP
#include <QStackedWidget>
#include "Nexus/Definitions/Security.hpp"
#include "Spire/Ui/ComboBox.hpp"
#include "Spire/Ui/Ui.hpp"

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
       * @param query_model The model used to query matches.
       * @param body The component that represents the current security.
       * @param parent The parent widget.
       */
      SecurityView(std::shared_ptr<ComboBox::QueryModel> query_model,
        QWidget* body, QWidget* parent = nullptr);

      /**
       * Constructs a SecurityView.
       * @param query_model The model used to query matches.
       * @param body The component that represents the current security.
       * @param current The current value's model.
       * @param parent The parent widget.
       */
      SecurityView(std::shared_ptr<ComboBox::QueryModel> query_model,
        QWidget* body, std::shared_ptr<CurrentModel> current,
        QWidget* parent = nullptr);

      /** Returns the model used to query matches. */
      const std::shared_ptr<ComboBox::QueryModel>& get_query_model() const;

      /** Returns the current model. */
      const std::shared_ptr<CurrentModel>& get_current() const;
    
      /** Returns the body. */
      const QWidget& get_body() const;

      /** Returns the body. */
      QWidget& get_body();

    protected:
      void keyPressEvent(QKeyEvent* event) override;

    private:
      class SecuritySearchWindow;
      SecuritySearchWindow* m_search_window;
      QWidget* m_body;
      std::shared_ptr<CurrentModel> m_current;
      int m_current_index;
      TextBox* m_prompt;
      QStackedWidget* m_layers;
      std::vector<Nexus::Security> m_securities;

      void on_submit(const Nexus::Security& security);
  };
}

#endif
