#ifndef SPIRE_PROGRESS_BAR_HPP
#define SPIRE_PROGRESS_BAR_HPP
#include <boost/optional/optional.hpp>
#include <QWidget>
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Styles/PeriodicEvaluator.hpp"
#include "Spire/Styles/StateSelector.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /**
   * The type of model used to measure progress, represents an integer from -1
   * to 100 where -1 is an error, 0 is the starting point, and 100 is
   * completion.
   */
  using ProgressModel = ValueModel<int>;

  /** A LocalValueModel for measuring progress. */
  using LocalProgressModel = LocalValueModel<int>;

  /** Displays a horizontal progress bar. */
  class ProgressBar : public QWidget {
    public:

      /** Selects the fill bar. */
      using Fill = Styles::StateSelector<void, struct FillTag>;

      /** Constructs a ProgressBar using a LocalProgressModel starting at 0. */
      ProgressBar();

      /**
       * Constructs a ProgressBar.
       * @param current The current progress.
       */
      explicit ProgressBar(std::shared_ptr<ProgressModel> current);

      /** Returns the current progress. */
      const std::shared_ptr<ProgressModel>& get_current() const;

      QSize sizeHint() const override;

    protected:
      void resizeEvent(QResizeEvent* event) override;

    private:
      Box* m_fill;
      std::shared_ptr<ProgressModel> m_current;
      int m_last_current;
      boost::optional<Styles::PeriodicEvaluator<int>> m_fill_width_evaluator;
      boost::signals2::scoped_connection m_connection;

      void on_current(int current);
      void on_fill_width_evaluated(int width);
  };
}

#endif
