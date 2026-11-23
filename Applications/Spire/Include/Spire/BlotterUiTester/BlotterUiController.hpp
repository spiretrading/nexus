#ifndef SPIRE_BLOTTER_UI_CONTROLLER_HPP
#define SPIRE_BLOTTER_UI_CONTROLLER_HPP
#include <unordered_map>
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QTextEdit>
#include <QWidget>
#include "Nexus/Accounting/Position.hpp"
#include "Nexus/TestEnvironment/TestEnvironment.hpp"
#include "Spire/Async/QtTaskQueue.hpp"
#include "Spire/Blotter/BlotterWindow.hpp"
#include "Spire/LegacyUI/UserProfile.hpp"
#include "Spire/Spire/UnionListModel.hpp"

namespace Spire {

  /** Creates single order tasks and controls their simulated executions. */
  class BlotterUiController : public QWidget {
    public:

      /**
       * Constructs a BlotterUiController.
       * @param environment The simulated market environment.
       * @param profile The profile used to execute tasks.
       */
      BlotterUiController(Beam::Ref<Nexus::TestEnvironment> environment,
        Beam::Ref<UserProfile> profile);

      ~BlotterUiController();

      /**
       * Runs the selected tasks and logs the request.
       * @param tasks The tasks selected to run.
       */
      void on_run_tasks(const std::vector<std::shared_ptr<Task>>& tasks);

      /**
       * Logs a request to cancel tasks.
       * @param tasks The tasks selected for cancellation.
       */
      void on_cancel_tasks(const std::vector<std::shared_ptr<Task>>& tasks);

      /**
       * Logs a request to cancel orders.
       * @param orders The orders selected for cancellation.
       */
      void on_cancel_orders(
        const std::vector<std::shared_ptr<Nexus::Order>>& orders);

      /**
       * Logs a key-bound request to cancel tasks.
       * @param operation The requested cancel operation.
       */
      void on_cancel_task_operation(
        CancelKeyBindingsModel::Operation operation);

      /**
       * Logs a key-bound request to cancel orders.
       * @param operation The requested cancel operation.
       */
      void on_cancel_order_operation(
        CancelKeyBindingsModel::Operation operation);

      /**
       * Logs a request to flatten positions.
       * @param positions The positions selected for flattening.
       */
      void on_flatten_positions(const std::vector<Nexus::Position>& positions);

    protected:
      void closeEvent(QCloseEvent* event) override;

    private:
      struct Blotter {
        std::shared_ptr<TasksModel> m_tasks;
        std::unique_ptr<BlotterWindow> m_window;
        boost::signals2::scoped_connection m_active_connection;
      };
      struct Entry {
        std::shared_ptr<Task> m_task;
        std::shared_ptr<TasksModel> m_tasks;
        std::shared_ptr<Nexus::Order> m_order;
        Task::StateEntry m_state = Task::StateEntry(Task::State::READY);
        Nexus::OrderStatus m_status = Nexus::OrderStatus::NONE;
        Nexus::Quantity m_filled = 0;
        bool m_is_pending = false;
      };
      Nexus::TestEnvironment* m_environment;
      UserProfile* m_profile;
      std::shared_ptr<BlotterCollection> m_collection;
      std::shared_ptr<UnionListModel<std::shared_ptr<Nexus::Order>>> m_orders;
      std::shared_ptr<PortfolioModel> m_portfolio;
      std::vector<Blotter> m_blotters;
      int m_active;
      std::vector<Entry> m_entries;
      std::unordered_map<Nexus::OrderId, std::shared_ptr<Nexus::Order>>
        m_submissions;
      QComboBox* m_blotter_selection;
      QLineEdit* m_name;
      QLineEdit* m_ticker;
      QLineEdit* m_currency;
      QComboBox* m_type;
      QComboBox* m_side;
      QLineEdit* m_destination;
      QSpinBox* m_quantity;
      QLineEdit* m_price;
      QComboBox* m_time_in_force;
      QComboBox* m_selection;
      QLabel* m_status;
      QLineEdit* m_fill_price;
      QSpinBox* m_fill_quantity;
      QPushButton* m_accept;
      QPushButton* m_reject;
      QPushButton* m_fill;
      QPushButton* m_partial_fill;
      QPushButton* m_cancel;
      QTextEdit* m_output;
      QtTaskQueue m_events;

      Nexus::OrderFields make_fields() const;
      void refresh();
      void set_active(int index);
      void on_new_blotter();
      void on_blotter_selection(int index);
      void on_active(int index, bool is_active);
      void on_create();
      void on_selection(int index);
      void on_submission(const std::shared_ptr<Nexus::Order>& order);
      void on_order(int index, const std::shared_ptr<Nexus::Order>& order);
      void on_report(int index, const Nexus::ExecutionReport& report);
      void on_state(int index, const Task::StateEntry& state);
      void on_action(Nexus::OrderStatus status);
  };
}

#endif
