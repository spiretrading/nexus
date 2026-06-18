#ifndef SPIRE_PREVIEW_ORDER_DISPLAY_VALUE_MODEL_HPP
#define SPIRE_PREVIEW_ORDER_DISPLAY_VALUE_MODEL_HPP
#include "Spire/BookView/BookViewModel.hpp"
#include "Spire/BookView/BookViewProperties.hpp"
#include "Spire/Spire/ValueModel.hpp"

namespace Spire {

  /**
   * Toggles between displaying the preview order and hiding it depending on the
   * BookViewProperties.
   */
  class PreviewOrderDisplayValueModel :
      public BookViewModel::PreviewOrderModel {
    public:

      /**
       * Constructs a PreviewOrderDisplayValueModel.
       * @param preview The preview order to display or hide.
       * @param properties The BookViewProperties used to determine whether the
       *        <i>preview</i> is displayed.
       */
      PreviewOrderDisplayValueModel(
        std::shared_ptr<BookViewModel::PreviewOrderModel> preview,
        std::shared_ptr<BookViewPropertiesModel> properties);

      const Type& get() const override;

      boost::signals2::connection connect_update_signal(
        const UpdateSignal::slot_type& slot) const override;

    private:
      mutable UpdateSignal m_update_signal;
      std::shared_ptr<BookViewModel::PreviewOrderModel> m_preview;
      std::shared_ptr<BookViewPropertiesModel> m_properties;
      bool m_has_preview;
      bool m_is_displayed;
      boost::signals2::scoped_connection m_preview_connection;
      boost::signals2::scoped_connection m_properties_connection;

      void on_preview(const Type& preview);
      void on_properties(const BookViewProperties& properties);
  };
}

#endif
