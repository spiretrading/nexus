#ifndef SPIRE_BOOK_VIEW_TEST_CONTROLLER_WINDOW
#define SPIRE_BOOK_VIEW_TEST_CONTROLLER_WINDOW
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QWidget>
#include <Nexus/Definitions/Security.hpp>
#include "Spire/BookView/BookView.hpp"
#include "Spire/BookViewUiTester/RandomBookViewModel.hpp"

namespace Spire {

  //! Displays the window used to control how a book view window is tested.
  class BookViewTestControllerWindow : public QWidget {
    public:
      BookViewTestControllerWindow(BookViewWindow* window);

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;

    private:
      BookViewWindow* m_window;
      std::shared_ptr<RandomBookViewModel> m_model;
      QSpinBox* m_load_time_spin_box;
      QSpinBox* m_model_update_period_spin_box;
      QComboBox* m_market_combo_box;
      QLineEdit* m_mpid_line_edit;
      QDoubleSpinBox* m_price_spin_box;
      QSpinBox* m_quantity_spin_box;
      QComboBox* m_side_combo_box;
      QPushButton* m_submit_button;

      void on_security_changed(const Nexus::Security& security);
      void on_model_period_updated();
      void on_submit();
  };
}

#endif
