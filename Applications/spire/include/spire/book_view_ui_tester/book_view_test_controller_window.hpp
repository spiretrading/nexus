#ifndef SPIRE_BOOK_VIEW_TEST_CONTROLLER_WINDOW
#define SPIRE_BOOK_VIEW_TEST_CONTROLLER_WINDOW
#include <QPushButton>
#include <QComboBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QWidget>
#include <Beam/Threading/TimerThreadPool.hpp>
#include <Nexus/Definitions/Security.hpp>
#include "spire/book_view/book_view.hpp"
#include "spire/book_view_ui_tester/random_book_view_model.hpp"

namespace Spire {

  //! Displays the window used to control how a book view window is tested.
  class BookViewTestControllerWindow : public QWidget {
    public:
      BookViewTestControllerWindow(BookViewWindow* window,
        Beam::Threading::TimerThreadPool& thread_timer_pool);

    private:
      BookViewWindow* m_window;
      std::shared_ptr<RandomBookViewModel> m_model;
      Beam::Threading::TimerThreadPool* m_timer_thread_pool;
      QSpinBox* m_load_time_spin_box;
      QSpinBox* m_model_update_period_spin_box;
      QComboBox* m_market_combo_box;
      QLineEdit* m_mpid_line_edit;
      QSpinBox* m_price_spin_box;
      QSpinBox* m_quantity_spin_box;
      QComboBox* m_side_combo_box;
      QPushButton* m_submit_button;

      void on_security_changed(const Nexus::Security& security);
      void on_model_period_updated();
  };
}

#endif
