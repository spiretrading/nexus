#ifndef SPIRE_SECURITY_VIEW_HPP
#define SPIRE_SECURITY_VIEW_HPP
#include <Beam/Serialization/DataShuttle.hpp>
#include <boost/signals2/connection.hpp>
#include <QStackedWidget>
#include "Spire/Spire/SecurityDeck.hpp"
#include "Spire/Ui/SecurityDialog.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /**
   * Displays securities and provides the functionality to quickly search and
   * cycle through securities.
   */
  class SecurityView : public QWidget {
    public:

      /** Stores the SecurityView's persistent state. */
      struct State {
        SecurityDeck m_securities;
      };

      /** A ValueModel over a Nexus::Security. */
      using CurrentModel = SecurityModel;

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

      /** Saves the state of this SecurityView. */
      State save_state() const;

      /** Restores the state of this SecurityView. */
      void restore(const State& state);

    protected:
      void keyPressEvent(QKeyEvent* event) override;

    private:
      SecurityDialog m_security_dialog;
      std::shared_ptr<CurrentModel> m_current;
      QWidget* m_body;
      QStackedWidget* m_layers;
      SecurityDeck m_securities;
      boost::signals2::scoped_connection m_current_connection;

      void on_current(const Nexus::Security& security);
      void on_submit(const Nexus::Security& security);
  };
}

namespace Beam::Serialization {
  template<>
  struct Shuttle<Spire::SecurityView::State> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, Spire::SecurityView::State& value,
        unsigned int version) const {
      shuttle.Shuttle("securities", value.m_securities);
    }
  };
}

#endif
