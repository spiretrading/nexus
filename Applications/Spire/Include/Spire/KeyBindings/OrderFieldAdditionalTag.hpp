#ifndef SPIRE_ORDER_FIELD_ADDITIONAL_TAG_HPP
#define SPIRE_ORDER_FIELD_ADDITIONAL_TAG_HPP 
#include "Spire/KeyBindings/AdditionalTag.hpp"
#include "Spire/KeyBindings/OrderFieldInfoTip.hpp"

namespace Spire {

  /** Implements an AdditionalTag by the information of the order field. */
  class OrderFieldAdditionalTag : public AdditionalTag {
    public:

      /**
       * Constructs an OrderFieldAdditionalTag.
       * @param tag The key/value pair of the tag.
       * @param model The details of the order field.
       */
      OrderFieldAdditionalTag(Nexus::Tag tag, OrderFieldInfoTip::Model model);

      /** Returns the details of the order field. */
      const OrderFieldInfoTip::Model& get_model() const;

      const std::shared_ptr<AdditionalTagValueModel>&
        make_value() const override;

      QWidget* make_view(
        const std::shared_ptr<AdditionalTagValueModel>& value) const override;

    private:
      OrderFieldInfoTip::Model m_model;
      std::shared_ptr<AdditionalTagValueModel> m_value;
  };
}

#endif
