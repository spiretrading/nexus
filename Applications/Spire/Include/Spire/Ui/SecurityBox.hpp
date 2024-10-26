#ifndef SPIRE_SECURITY_BOX_HPP
#define SPIRE_SECURITY_BOX_HPP
#include "Nexus/Definitions/Security.hpp"
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Spire/QueryModel.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /** Represents a ValueModel for a Security. */
  using SecurityModel = ValueModel<Nexus::Security>;

  /** Represents a LocalValueModel for a Security. */
  using LocalSecurityModel = LocalValueModel<Nexus::Security>;

  /** Represents the QueryModel used for SecurityInfo objects. */
  using SecurityInfoQueryModel = QueryModel<Nexus::SecurityInfo>;

  /** Displays a security over an open set of security values. */
  class SecurityBox : public QWidget {
    public:

      /** A ValueModel over a Nexus::Security. */
      using CurrentModel = SecurityModel;

      /**
       * Signals that the value was submitted.
       * @param submission The submitted value.
       */
      using SubmitSignal = Signal<void (const Nexus::Security& submission)>;

      /**
       * Constructs a SecurityBox using a default local model.
       * @param securities The set of securities that can be queried.
       * @param parent The parent widget.
       */
      explicit SecurityBox(std::shared_ptr<SecurityInfoQueryModel> securities,
        QWidget* parent = nullptr);

      /**
       * Constructs a SecurityBox.
       * @param securities The set of securities that can be queried.
       * @param current The current security displayed.
       * @param parent The parent widget.
       */
      SecurityBox(std::shared_ptr<SecurityInfoQueryModel> securities,
        std::shared_ptr<CurrentModel> current, QWidget* parent = nullptr);

      /** Returns the set of securities that can be queried. */
      const std::shared_ptr<SecurityInfoQueryModel>& get_securities() const;

      /** Returns the current model. */
      const std::shared_ptr<CurrentModel>& get_current() const;

      /** Returns the last submission. */
      const Nexus::Security& get_submission() const;

      /** Sets the placeholder value. */
      void set_placeholder(const QString& placeholder);

      /** Returns <code>true</code> iff this SecurityBox is read-only. */
      bool is_read_only() const;

      /**
       * Sets the read-only state.
       * @param is_read_only True iff the SecurityBox should be read-only.
       */
      void set_read_only(bool is_read_only);

      /** Connects a slot to the submit signal. */
      boost::signals2::connection connect_submit_signal(
        const SubmitSignal::slot_type& slot) const;

    private:
      struct SecurityQueryModel;
      mutable SubmitSignal m_submit_signal;
      std::shared_ptr<SecurityQueryModel> m_securities;
      std::shared_ptr<CurrentModel> m_current;
      ComboBox* m_combo_box;
  };
}

#endif
