#ifndef SPIRE_RESPONSIVE_LABEL_HPP
#define SPIRE_RESPONSIVE_LABEL_HPP
#include "Spire/Ui/CastListModel.hpp"
#include "Spire/Ui/TextBox.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /**
   * Displays a TextBox used as a label whose current value is the longest value
   * among a list of values that will fit within it's body without overflowing.
   */
  class ResponsiveLabel : public QWidget {
    public:

      /**
       * Constructs a ResponsiveLabel.
       * @param list The list of values to display.
       * @param parent The parent widget.
       */
      explicit ResponsiveLabel(std::shared_ptr<CastListModel<QString>> list,
        QWidget* parent = nullptr);

      /** Returns the list of values to display. */
      const std::shared_ptr<CastListModel<QString>>& get_list() const;

      /** Returns the model of the current's displayed value. */
      const std::shared_ptr<TextModel>& get_current() const;

      /** Returns the highlight model. */
      const std::shared_ptr<HighlightModel>& get_highlight() const;

      QSize sizeHint() const override;
  };
}

#endif
