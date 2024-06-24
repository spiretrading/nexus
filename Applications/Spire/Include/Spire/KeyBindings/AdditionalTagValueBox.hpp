#ifndef SPIRE_ADDITIONAL_TAG_VALUE_BOX_HPP
#define SPIRE_ADDITIONAL_TAG_VALUE_BOX_HPP
#include <QWidget>
#include "Spire/KeyBindings/AdditionalTagsBox.hpp"
#include "Spire/KeyBindings/AdditionalTagSchema.hpp"
#include "Spire/KeyBindings/KeyBindings.hpp"
#include "Spire/Spire/ValueModel.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /** Models an AdditionalTagSchema. */
  using AdditionalTagSchemaModel =
    ValueModel<std::shared_ptr<AdditionalTagSchema>>;

  /** A local value model over an AdditionalTagSchema. */
  using LocalAdditionalTagSchemaModel =
    LocalValueModel<std::shared_ptr<AdditionalTagSchema>>;

  /** Represents the value of an additional tag. */
  class AdditionalTagValueBox : public QWidget {
    public:

      /**
       * Signals the submission of a value.
       * @param submission The value being submitted.
       */
      using SubmitSignal =
        Signal<void (const boost::optional<Nexus::Tag::Type>& submission)>;

      /**
       * Constructs an AdditionalTagValueBox.
       * @param current The current tag value to represent.
       * @param schema Provides the definition of the tag value represented.
       * @param parent The parent widget.
       */
      AdditionalTagValueBox(std::shared_ptr<AdditionalTagValueModel> current,
        std::shared_ptr<AdditionalTagSchemaModel> schema,
        QWidget* parent = nullptr);

      /** Returns the current tag value represented. */
      const std::shared_ptr<AdditionalTagValueModel>& get_current() const;

      /** Returns <code>true</code> iff this box is read-only. */
      bool is_read_only() const;

      /**
       * Sets the read-only state.
       * @param is_read_only <code>true</code> iff the box should be read-only.
       */
      void set_read_only(bool is_read_only);

      /** Connects a slot to the SubmitSignal. */
      boost::signals2::connection connect_submit_signal(
        const SubmitSignal::slot_type& slot) const;

    private:
      mutable SubmitSignal m_submit_signal;
      std::shared_ptr<AdditionalTagValueModel> m_current;
      std::shared_ptr<AdditionalTagSchemaModel> m_schema;
      boost::signals2::scoped_connection m_schema_connection;

      const AnyInputBox& get_input_box() const;
      AnyInputBox& get_input_box();
      void update_schema(
        const std::shared_ptr<AdditionalTagSchema>& schema, bool is_read_only);
      void on_schema(const std::shared_ptr<AdditionalTagSchema>& schema);
  };
}

#endif
