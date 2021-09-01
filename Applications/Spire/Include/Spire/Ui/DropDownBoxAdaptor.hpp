#ifndef SPIRE_DROP_DOWN_BOX_ADAPTOR_HPP
#define SPIRE_DROP_DOWN_BOX_ADAPTOR_HPP
#include "Spire/Ui/DropDownBox.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /**
   * Implements a wrapper over a DropDownBox to provide different interfaces.
   */
  class DropDownBoxAdaptor : public DropDownBox {
    public:

      /**
       * Constructs a DropDownBoxAdaptor using a LocalValueModel.
       * @param list_model The model of values to display.
       * @param parent The parent widget.
       */
      explicit DropDownBoxAdaptor(std::shared_ptr<ListModel> list_model,
        QWidget* parent = nullptr);

      /**
       * Constructs a DropDownBoxAdaptor.
       * @param list_model The model of values to display.
       * @param current_model The current value's model.
       * @param parent The parent widget.
       */
      DropDownBoxAdaptor(std::shared_ptr<ListModel> list_model,
        std::shared_ptr<ValueModel<boost::optional<int>>> current_model,
        QWidget* parent = nullptr);

      /** Returns the list model. */
      const std::shared_ptr<ListModel>& get_list_model() const;

      /** Returns the current model. */
      const std::shared_ptr<ValueModel<boost::optional<int>>>&
        get_current_model() const;

    private:
      ListView* m_list_view;

      ListView* make_list_view(std::shared_ptr<ListModel> list_model,
        std::shared_ptr<ValueModel<boost::optional<int>>> current_model);
  };
}

#endif
