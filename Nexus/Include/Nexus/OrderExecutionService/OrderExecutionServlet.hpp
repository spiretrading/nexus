#ifndef NEXUS_ORDER_EXECUTION_SERVLET_HPP
#define NEXUS_ORDER_EXECUTION_SERVLET_HPP
#include <algorithm>
#include <iostream>
#include <Beam/Collections/SynchronizedMap.hpp>
#include <Beam/Collections/SynchronizedSet.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Queries/IndexedSubscriptions.hpp>
#include <Beam/Queues/RoutineTaskQueue.hpp>
#include <Beam/Serialization/JsonSender.hpp>
#include <Beam/Threading/Sync.hpp>
#include <Beam/Utilities/ReportException.hpp>
#include <boost/functional/factory.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/optional/optional.hpp>
#include <boost/throw_exception.hpp>
#include "Nexus/Accounting/ShortingModel.hpp"
#include "Nexus/AdministrationService/AdministrationClient.hpp"
#include "Nexus/AdministrationService/TradingGroup.hpp"
#include "Nexus/Definitions/Destination.hpp"
#include "Nexus/Definitions/Venue.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionDriver.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionServices.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionSession.hpp"
#include "Nexus/OrderExecutionService/OrderSubmissionRegistry.hpp"
#include "Nexus/OrderExecutionService/PrimitiveOrder.hpp"
#include "Nexus/OrderExecutionService/StandardQueries.hpp"
#include "Nexus/Queries/EvaluatorTranslator.hpp"
#include "Nexus/Queries/ShuttleQueryTypes.hpp"

namespace Nexus {

  /**
   * Implements the servlet handling order submissions and cancellations.
   * @param <C> The container instantiating this servlet.
   * @param <T> The type of TimeClient used for timestamps.
   * @param <S> The type of ServiceLocatorClient used.
   * @param <U> The type of UidClient generating unique order ids.
   * @param <A> The type of AdministrationClient to use.
   * @param <O> The OrderExecutionDriver submitting and receiving the execution
   *        messaging.
   * @param <D> The type of OrderExecutionDataStore to use.
   */
  template<typename C, typename T, typename S, typename U, typename A,
    typename O, typename D> requires
      Beam::IsTimeClient<Beam::dereference_t<T>> &&
        Beam::IsServiceLocatorClient<Beam::dereference_t<S>> &&
          Beam::IsUidClient<Beam::dereference_t<U>> &&
            IsAdministrationClient<Beam::dereference_t<A>> &&
              IsOrderExecutionDriver<Beam::dereference_t<O>> &&
                IsOrderExecutionDataStore<Beam::dereference_t<D>>
  class OrderExecutionServlet {
    public:

      /** The type of TimeClient used for timestamps. */
      using TimeClient = Beam::dereference_t<T>;

      /** The type of ServiceLocatorClient used. */
      using ServiceLocatorClient = Beam::dereference_t<S>;

      /** The type of UidClient generating unique order ids. */
      using UidClient = Beam::dereference_t<U>;

      /** The type of AdministrationClient used to check permissions. */
      using AdministrationClient = Beam::dereference_t<A>;

      /**
       * The OrderExecutionDriver submitting and receiving the execution
       * messaging.
       */
      using OrderExecutionDriver = Beam::dereference_t<O>;

      /** The type of OrderExecutionDataStore used. */
      using OrderExecutionDataStore = Beam::dereference_t<D>;

      using Container = C;
      using ServiceProtocolClient = typename Container::ServiceProtocolClient;

      /**
       * Constructs an OrderExecutionServlet.
       * @param session_start_time The time when the current trading session
       *        started.
       * @param venues The VenueDatabase to use.
       * @param destinations The DestinationDatabase to use.
       * @param time_client Initializes the TimeClient.
       * @param service_locator_client Initializes the ServiceLocatorClient.
       * @param uid_client Initializes the UidClient.
       * @param administration_client Initializes the AdministrationClient.
       * @param driver Initializes the OrderExecutionDriver.
       * @param data_store Initializes the OrderExecutionDataStore.
       */
      template<Beam::Initializes<T> TF, Beam::Initializes<S> SF,
        Beam::Initializes<U> UF, Beam::Initializes<A> AF,
        Beam::Initializes<O> OF, Beam::Initializes<D> DF>
      OrderExecutionServlet(boost::posix_time::ptime session_start_time,
        VenueDatabase venues, DestinationDatabase destinations,
        TF&& time_client, SF&& service_locator_client, UF&& uid_client,
        AF&& administration_client, OF&& driver, DF&& data_store);

      void register_services(
        Beam::Out<Beam::ServiceSlots<ServiceProtocolClient>> slots);
      void handle_accept(ServiceProtocolClient& client);
      void handle_close(ServiceProtocolClient& client);
      void close();

    private:
      using SyncShortingModel = Beam::Sync<ShortingModel>;
      boost::posix_time::ptime m_session_start_time;
      VenueDatabase m_venues;
      DestinationDatabase m_destinations;
      Beam::local_ptr_t<T> m_time_client;
      Beam::local_ptr_t<S> m_service_locator_client;
      Beam::local_ptr_t<U> m_uid_client;
      Beam::local_ptr_t<A> m_administration_client;
      Beam::local_ptr_t<O> m_driver;
      Beam::local_ptr_t<D> m_data_store;
      OrderSubmissionRegistry m_registry;
      Beam::IndexedSubscriptions<OrderRecord, Beam::DirectoryEntry,
        ServiceProtocolClient> m_submission_subscriptions;
      Beam::IndexedSubscriptions<ExecutionReport, Beam::DirectoryEntry,
        ServiceProtocolClient> m_order_subscriptions;
      Beam::IndexedSubscriptions<ExecutionReport, Beam::DirectoryEntry,
        ServiceProtocolClient> m_execution_report_subscriptions;
      std::vector<std::shared_ptr<PrimitiveOrder>> m_rejected_orders;
      Beam::SynchronizedUnorderedMap<Beam::DirectoryEntry,
        std::shared_ptr<SyncShortingModel>> m_shorting_models;
      Beam::SynchronizedUnorderedSet<OrderId> m_live_orders;
      Beam::OpenState m_open_state;
      Beam::RoutineTaskQueue m_tasks;

      OrderExecutionServlet(const OrderExecutionServlet&) = delete;
      OrderExecutionServlet& operator =(const OrderExecutionServlet&) = delete;
      void recover(const Beam::DirectoryEntry& account);
      void recover_trading_session();
      void on_execution_report(const ExecutionReport& report,
        const Beam::DirectoryEntry& account, SyncShortingModel& shorting_model);
      void on_load_order_by_id_request(Beam::RequestToken<
        ServiceProtocolClient, LoadOrderByIdService>& request, OrderId id);
      void on_query_order_submissions_request(Beam::RequestToken<
        ServiceProtocolClient, QueryOrderSubmissionsService>& request,
        const AccountQuery& query);
      void on_query_execution_reports_request(Beam::RequestToken<
        ServiceProtocolClient, QueryExecutionReportsService>& request,
        const AccountQuery& query);
      void on_new_order_single_request(Beam::RequestToken<
        ServiceProtocolClient, NewOrderSingleService>& request,
        const OrderFields& fields);
      void on_update_order_request(ServiceProtocolClient& client, OrderId id,
        const ExecutionReport& report);
      void on_cancel_order(ServiceProtocolClient& client, OrderId id);
  };

  template<typename T, typename S, typename U, typename A, typename O,
    typename D>
  struct MetaOrderExecutionServlet {
    using Session = OrderExecutionSession;

    template<typename C>
    struct apply {
      using type = OrderExecutionServlet<C, T, S, U, A, O, D>;
    };
  };

  template<typename C, typename T, typename S, typename U, typename A,
    typename O, typename D> requires
      Beam::IsTimeClient<Beam::dereference_t<T>> &&
        Beam::IsServiceLocatorClient<Beam::dereference_t<S>> &&
          Beam::IsUidClient<Beam::dereference_t<U>> &&
            IsAdministrationClient<Beam::dereference_t<A>> &&
              IsOrderExecutionDriver<Beam::dereference_t<O>> &&
                IsOrderExecutionDataStore<Beam::dereference_t<D>>
  template<Beam::Initializes<T> TF, Beam::Initializes<S> SF,
    Beam::Initializes<U> UF, Beam::Initializes<A> AF, Beam::Initializes<O> OF,
    Beam::Initializes<D> DF>
  OrderExecutionServlet<C, T, S, U, A, O, D>::OrderExecutionServlet(
      boost::posix_time::ptime session_start_time, VenueDatabase venues,
      DestinationDatabase destinations, TF&& time_client,
      SF&& service_locator_client, UF&& uid_client, AF&& administration_client,
      OF&& driver, DF&& data_store)
    : m_session_start_time(session_start_time),
      m_venues(std::move(venues)),
      m_destinations(std::move(destinations)),
      m_time_client(std::forward<TF>(time_client)),
      m_service_locator_client(std::forward<SF>(service_locator_client)),
      m_uid_client(std::forward<UF>(uid_client)),
      m_administration_client(std::forward<AF>(administration_client)),
      m_driver(std::forward<OF>(driver)),
      m_data_store(std::forward<DF>(data_store)) {
    try {
      auto accounts = m_service_locator_client->load_all_accounts();
      for(auto& account : accounts) {
        m_registry.add(account);
      }
      recover_trading_session();
    } catch(const std::exception&) {
      close();
      throw;
    }
  }

  template<typename C, typename T, typename S, typename U, typename A,
    typename O, typename D> requires
      Beam::IsTimeClient<Beam::dereference_t<T>> &&
        Beam::IsServiceLocatorClient<Beam::dereference_t<S>> &&
          Beam::IsUidClient<Beam::dereference_t<U>> &&
            IsAdministrationClient<Beam::dereference_t<A>> &&
              IsOrderExecutionDriver<Beam::dereference_t<O>> &&
                IsOrderExecutionDataStore<Beam::dereference_t<D>>
  void OrderExecutionServlet<C, T, S, U, A, O, D>::register_services(
      Beam::Out<Beam::ServiceSlots<ServiceProtocolClient>> slots) {
    Nexus::register_query_types(Beam::out(slots->get_registry()));
    register_order_execution_services(out(slots));
    register_order_execution_messages(out(slots));
    LoadOrderByIdService::add_request_slot(out(slots), std::bind_front(
      &OrderExecutionServlet::on_load_order_by_id_request, this));
    QueryOrderSubmissionsService::add_request_slot(out(slots), std::bind_front(
      &OrderExecutionServlet::on_query_order_submissions_request, this));
    QueryExecutionReportsService::add_request_slot(out(slots), std::bind_front(
      &OrderExecutionServlet::on_query_execution_reports_request, this));
    NewOrderSingleService::add_request_slot(out(slots), std::bind_front(
      &OrderExecutionServlet::on_new_order_single_request, this));
    UpdateOrderService::add_slot(out(slots),
      std::bind_front(&OrderExecutionServlet::on_update_order_request, this));
    Beam::add_message_slot<CancelOrderMessage>(out(slots),
      std::bind_front(&OrderExecutionServlet::on_cancel_order, this));
  }

  template<typename C, typename T, typename S, typename U, typename A,
    typename O, typename D> requires
      Beam::IsTimeClient<Beam::dereference_t<T>> &&
        Beam::IsServiceLocatorClient<Beam::dereference_t<S>> &&
          Beam::IsUidClient<Beam::dereference_t<U>> &&
            IsAdministrationClient<Beam::dereference_t<A>> &&
              IsOrderExecutionDriver<Beam::dereference_t<O>> &&
                IsOrderExecutionDataStore<Beam::dereference_t<D>>
  void OrderExecutionServlet<C, T, S, U, A, O, D>::handle_accept(
      ServiceProtocolClient& client) {
    auto& session = client.get_session();
    m_registry.add(session.get_account());
    session.grant_permission(session.get_account());
    if(m_administration_client->check_administrator(session.get_account())) {
      session.set_administrator(true);
    }
    auto trading_group_directories =
      m_administration_client->load_managed_trading_groups(
        session.get_account());
    for(auto& trading_group_directory : trading_group_directories) {
      auto trading_group = m_administration_client->load_trading_group(
        trading_group_directory);
      for(auto& trader : trading_group.get_traders()) {
        m_registry.add(trader);
        session.grant_permission(trader);
      }
    }
  }

  template<typename C, typename T, typename S, typename U, typename A,
    typename O, typename D> requires
      Beam::IsTimeClient<Beam::dereference_t<T>> &&
        Beam::IsServiceLocatorClient<Beam::dereference_t<S>> &&
          Beam::IsUidClient<Beam::dereference_t<U>> &&
            IsAdministrationClient<Beam::dereference_t<A>> &&
              IsOrderExecutionDriver<Beam::dereference_t<O>> &&
                IsOrderExecutionDataStore<Beam::dereference_t<D>>
  void OrderExecutionServlet<C, T, S, U, A, O, D>::handle_close(
      ServiceProtocolClient& client) {
    m_execution_report_subscriptions.remove_all(client);
    m_order_subscriptions.remove_all(client);
    m_submission_subscriptions.remove_all(client);
  }

  template<typename C, typename T, typename S, typename U, typename A,
    typename O, typename D> requires
      Beam::IsTimeClient<Beam::dereference_t<T>> &&
        Beam::IsServiceLocatorClient<Beam::dereference_t<S>> &&
          Beam::IsUidClient<Beam::dereference_t<U>> &&
            IsAdministrationClient<Beam::dereference_t<A>> &&
              IsOrderExecutionDriver<Beam::dereference_t<O>> &&
                IsOrderExecutionDataStore<Beam::dereference_t<D>>
  void OrderExecutionServlet<C, T, S, U, A, O, D>::close() {
    if(m_open_state.set_closing()) {
      return;
    }
    m_tasks.close();
    m_tasks.wait();
    m_data_store->close();
    m_driver->close();
    m_shorting_models.clear();
    m_open_state.close();
  }

  template<typename C, typename T, typename S, typename U, typename A,
    typename O, typename D> requires
      Beam::IsTimeClient<Beam::dereference_t<T>> &&
        Beam::IsServiceLocatorClient<Beam::dereference_t<S>> &&
          Beam::IsUidClient<Beam::dereference_t<U>> &&
            IsAdministrationClient<Beam::dereference_t<A>> &&
              IsOrderExecutionDriver<Beam::dereference_t<O>> &&
                IsOrderExecutionDataStore<Beam::dereference_t<D>>
  void OrderExecutionServlet<C, T, S, U, A, O, D>::recover(
      const Beam::DirectoryEntry& account) {
    auto recovery_query = AccountQuery();
    recovery_query.set_index(account);
    recovery_query.set_range(m_session_start_time, Beam::Sequence::LAST);
    recovery_query.set_snapshot_limit(Beam::SnapshotLimit::UNLIMITED);
    auto session_orders = m_data_store->load_order_records(recovery_query);
    auto live_orders =
      m_data_store->load_order_records(make_live_orders_query(account));
    auto orders = std::vector<SequencedOrderRecord>();
    std::set_union(session_orders.begin(), session_orders.end(),
      live_orders.begin(), live_orders.end(), std::back_inserter(orders),
      Beam::SequenceComparator());
    for(auto& order_record : orders) {
      auto& shorting_model = *m_shorting_models.get_or_insert(
        order_record->m_info.m_fields.m_account,
        boost::factory<std::shared_ptr<SyncShortingModel>>());
      shorting_model.with([&] (auto& shorting_model) {
        shorting_model.submit(
          order_record->m_info.m_id, order_record->m_info.m_fields);
        for(auto& report : order_record->m_execution_reports) {
          shorting_model.update(report);
        }
      });
      m_live_orders.insert(order_record->m_info.m_id);
      auto order = std::shared_ptr<Order>();
      try {
        order = m_driver->recover(Beam::SequencedValue(Beam::IndexedValue(
          *order_record, account), order_record.get_sequence()));
      } catch(const std::exception&) {
        try {
          std::throw_with_nested(
            std::runtime_error("Unable to recover order: " +
              boost::lexical_cast<std::string>(order_record->m_info.m_id)));
        } catch(const std::exception&) {
          std::cout << BEAM_REPORT_CURRENT_EXCEPTION() << std::flush;
          continue;
        }
      }
      order->get_publisher().with([&] {
        auto existing_reports = boost::optional<std::vector<ExecutionReport>>();
        order->get_publisher().monitor(m_tasks.get_slot<ExecutionReport>(
          std::bind(&OrderExecutionServlet::on_execution_report, this,
            std::placeholders::_1, order->get_info().m_fields.m_account,
            std::ref(shorting_model))), Beam::out(existing_reports));
        if(existing_reports) {
          existing_reports->erase(
            existing_reports->begin(), existing_reports->begin() +
              order_record->m_execution_reports.size());
          for(auto& report : *existing_reports) {
            m_tasks.push(std::bind_front(
              &OrderExecutionServlet::on_execution_report, this, report,
              order->get_info().m_fields.m_account, std::ref(shorting_model)));
          }
        }
      });
    }
  }

  template<typename C, typename T, typename S, typename U, typename A,
    typename O, typename D> requires
      Beam::IsTimeClient<Beam::dereference_t<T>> &&
        Beam::IsServiceLocatorClient<Beam::dereference_t<S>> &&
          Beam::IsUidClient<Beam::dereference_t<U>> &&
            IsAdministrationClient<Beam::dereference_t<A>> &&
              IsOrderExecutionDriver<Beam::dereference_t<O>> &&
                IsOrderExecutionDataStore<Beam::dereference_t<D>>
  void OrderExecutionServlet<C, T, S, U, A, O, D>::recover_trading_session() {
    auto accounts = m_service_locator_client->load_all_accounts();
    auto routines = Beam::RoutineHandlerGroup();
    for(auto& account : accounts) {
      routines.spawn([=, this] {
        recover(account);
      });
    }
    routines.wait();
  }

  template<typename C, typename T, typename S, typename U, typename A,
    typename O, typename D> requires
      Beam::IsTimeClient<Beam::dereference_t<T>> &&
        Beam::IsServiceLocatorClient<Beam::dereference_t<S>> &&
          Beam::IsUidClient<Beam::dereference_t<U>> &&
            IsAdministrationClient<Beam::dereference_t<A>> &&
              IsOrderExecutionDriver<Beam::dereference_t<O>> &&
                IsOrderExecutionDataStore<Beam::dereference_t<D>>
  void OrderExecutionServlet<C, T, S, U, A, O, D>::on_execution_report(
      const ExecutionReport& report, const Beam::DirectoryEntry& account,
      SyncShortingModel& shorting_model) {
    Beam::with(shorting_model, [&] (auto& model) {
      model.update(report);
    });
    try {
      m_registry.publish(Beam::IndexedValue(report, account),
        [&] {
          return load_initial_sequences(*m_data_store, account);
        },
        [&] (const auto& sequenced_report) {
          m_data_store->store(sequenced_report);
          m_order_subscriptions.publish(sequenced_report,
            [&] (const auto& clients) {
              Beam::broadcast_record_message<OrderUpdateMessage>(
                clients, **sequenced_report);
            });
          m_execution_report_subscriptions.publish(sequenced_report,
            [&] (const auto& clients) {
              Beam::broadcast_record_message<ExecutionReportMessage>(
                clients, sequenced_report);
            });
        });
      if(is_terminal(report.m_status)) {
        m_live_orders.erase(report.m_id);
      }
    } catch(const std::exception&) {
      std::cout << BEAM_REPORT_CURRENT_EXCEPTION() << std::flush;
      std::cout << "\taccount: " << Beam::to_json(account) <<
        "\n\texecution_report:" << Beam::to_json(report) << std::endl;
    }
  }

  template<typename C, typename T, typename S, typename U, typename A,
    typename O, typename D> requires
      Beam::IsTimeClient<Beam::dereference_t<T>> &&
        Beam::IsServiceLocatorClient<Beam::dereference_t<S>> &&
          Beam::IsUidClient<Beam::dereference_t<U>> &&
            IsAdministrationClient<Beam::dereference_t<A>> &&
              IsOrderExecutionDriver<Beam::dereference_t<O>> &&
                IsOrderExecutionDataStore<Beam::dereference_t<D>>
  void OrderExecutionServlet<C, T, S, U, A, O, D>::on_load_order_by_id_request(
      Beam::RequestToken<ServiceProtocolClient, LoadOrderByIdService>& request,
      OrderId id) {
    auto& session = request.get_session();
    auto order = m_data_store->load_order_record(id);
    if(!order || !session.has_permission((*order)->get_index())) {
      request.set(boost::none);
      return;
    }
    if(!(**order)->m_execution_reports.empty() &&
        is_terminal((**order)->m_execution_reports.back().m_status)) {
      request.set(order);
      return;
    }
    auto query = AccountQuery();
    query.set_index((*order)->get_index());
    query.set_range(order->get_sequence(), order->get_sequence());
    query.set_snapshot_limit(Beam::SnapshotLimit::from_head(1));
    auto result = ExecutionReportQueryResult();
    result.m_id = m_order_subscriptions.init(query.get_index(),
      request.get_client(), Beam::Range::TOTAL,
      Beam::translate(Beam::ConstantExpression(true)));
    order = m_data_store->load_order_record(id);
    m_order_subscriptions.commit(
      query.get_index(), std::move(result), [&] (auto report_result) {
        auto& reports = (**order)->m_execution_reports;
        for(auto& report : report_result.m_snapshot) {
          if(report->m_id != id) {
            continue;
          }
          auto position =
            std::lower_bound(reports.begin(), reports.end(), *report,
              [] (const auto& lhs, const auto& rhs) {
                return lhs.m_sequence < rhs.m_sequence;
              });
          if(position == reports.end() ||
              position->m_sequence != report->m_sequence) {
            reports.insert(position, *report);
          }
        }
        request.set(order);
      });
  }

  template<typename C, typename T, typename S, typename U, typename A,
    typename O, typename D> requires
      Beam::IsTimeClient<Beam::dereference_t<T>> &&
        Beam::IsServiceLocatorClient<Beam::dereference_t<S>> &&
          Beam::IsUidClient<Beam::dereference_t<U>> &&
            IsAdministrationClient<Beam::dereference_t<A>> &&
              IsOrderExecutionDriver<Beam::dereference_t<O>> &&
                IsOrderExecutionDataStore<Beam::dereference_t<D>>
  void OrderExecutionServlet<C, T, S, U, A, O, D>::
      on_query_order_submissions_request(Beam::RequestToken<
        ServiceProtocolClient, QueryOrderSubmissionsService>& request,
        const AccountQuery& query) {
    auto& session = request.get_session();
    auto revised_query = query;
    if(revised_query.get_index().m_type == Beam::DirectoryEntry::Type::NONE) {
      revised_query.set_index(session.get_account());
    }
    if(!session.has_permission(revised_query.get_index())) {
      request.set(OrderSubmissionQueryResult());
      return;
    }
    auto filter = Beam::translate<EvaluatorTranslator>(
      revised_query.get_filter(), Beam::Ref(m_live_orders));
    auto submission_result = OrderSubmissionQueryResult();
    submission_result.m_id = m_submission_subscriptions.init(
      revised_query.get_index(), request.get_client(),
      revised_query.get_range(), std::move(filter));
    auto execution_report_result = ExecutionReportQueryResult();
    execution_report_result.m_id = m_order_subscriptions.init(
      revised_query.get_index(), request.get_client(), Beam::Range::TOTAL,
      Beam::translate(Beam::ConstantExpression(true)));
    submission_result.m_snapshot =
      m_data_store->load_order_records(revised_query);
    m_submission_subscriptions.commit(revised_query.get_index(),
      std::move(submission_result), [&] (auto submission_result) {
        m_order_subscriptions.commit(revised_query.get_index(),
          std::move(execution_report_result),
          [&] (auto execution_report_result) {
            for(auto& report : execution_report_result.m_snapshot) {
              auto submission_iterator =
                std::find_if(submission_result.m_snapshot.begin(),
                  submission_result.m_snapshot.end(), [&] (const auto& record) {
                    return record->m_info.m_id == report->m_id;
                  });
              if(submission_iterator == submission_result.m_snapshot.end()) {
                continue;
              }
              auto& submission = *submission_iterator;
              auto position =
                std::lower_bound(submission->m_execution_reports.begin(),
                  submission->m_execution_reports.end(), *report,
                  [] (const auto& lhs, const auto& rhs) {
                    return lhs.m_sequence < rhs.m_sequence;
                  });
              if(position == submission->m_execution_reports.end() ||
                  position->m_sequence != report->m_sequence) {
                submission->m_execution_reports.insert(position, *report);
              }
            }
            request.set(submission_result);
          });
      });
  }

  template<typename C, typename T, typename S, typename U, typename A,
    typename O, typename D> requires
      Beam::IsTimeClient<Beam::dereference_t<T>> &&
        Beam::IsServiceLocatorClient<Beam::dereference_t<S>> &&
          Beam::IsUidClient<Beam::dereference_t<U>> &&
            IsAdministrationClient<Beam::dereference_t<A>> &&
              IsOrderExecutionDriver<Beam::dereference_t<O>> &&
                IsOrderExecutionDataStore<Beam::dereference_t<D>>
  void OrderExecutionServlet<C, T, S, U, A, O, D>::
      on_query_execution_reports_request(Beam::RequestToken<
        ServiceProtocolClient, QueryExecutionReportsService>& request,
        const AccountQuery& query) {
    auto& session = request.get_session();
    auto revised_query = query;
    if(revised_query.get_index().m_type == Beam::DirectoryEntry::Type::NONE) {
      revised_query.set_index(session.get_account());
    }
    if(!session.has_permission(revised_query.get_index())) {
      request.set(ExecutionReportQueryResult());
      return;
    }
    auto filter =
      Beam::translate<EvaluatorTranslator>(revised_query.get_filter());
    auto result = ExecutionReportQueryResult();
    result.m_id = m_execution_report_subscriptions.init(
      revised_query.get_index(), request.get_client(),
      revised_query.get_range(), std::move(filter));
    result.m_snapshot = m_data_store->load_execution_reports(revised_query);
    m_execution_report_subscriptions.commit(
      revised_query.get_index(), std::move(result), [&] (const auto& result) {
        request.set(result);
      });
  }

  template<typename C, typename T, typename S, typename U, typename A,
    typename O, typename D> requires
      Beam::IsTimeClient<Beam::dereference_t<T>> &&
        Beam::IsServiceLocatorClient<Beam::dereference_t<S>> &&
          Beam::IsUidClient<Beam::dereference_t<U>> &&
            IsAdministrationClient<Beam::dereference_t<A>> &&
              IsOrderExecutionDriver<Beam::dereference_t<O>> &&
                IsOrderExecutionDataStore<Beam::dereference_t<D>>
  void OrderExecutionServlet<C, T, S, U, A, O, D>::on_new_order_single_request(
      Beam::RequestToken<ServiceProtocolClient, NewOrderSingleService>& request,
      const OrderFields& fields) {
    auto& session = request.get_session();
    auto revised_fields = boost::optional<OrderFields>();
    auto order_fields = &fields;
    if(order_fields->m_account.m_type == Beam::DirectoryEntry::Type::NONE) {
      if(!revised_fields) {
        revised_fields.emplace(*order_fields);
        order_fields = revised_fields.get_ptr();
      }
      revised_fields->m_account = session.get_account();
    }
    if(order_fields->m_destination.empty()) {
      if(!revised_fields) {
        revised_fields.emplace(*order_fields);
        order_fields = revised_fields.get_ptr();
      }
      revised_fields->m_destination = m_destinations.get_preferred_destination(
        order_fields->m_security.get_venue()).m_id;
    }
    if(!order_fields->m_currency) {
      if(!revised_fields) {
        revised_fields.emplace(*order_fields);
        order_fields = revised_fields.get_ptr();
      }
      revised_fields->m_currency =
        m_venues.from(order_fields->m_security.get_venue()).m_currency;
    }
    auto order_id = m_uid_client->load_next_uid();
    auto shorting_model =
      m_shorting_models.get_or_insert(order_fields->m_account,
        boost::factory<std::shared_ptr<SyncShortingModel>>());
    auto shorting_flag = Beam::with(*shorting_model, [&] (auto& model) {
      return model.submit(order_id, *order_fields);
    });
    auto order_info = OrderInfo(*order_fields, session.get_account(), order_id,
      shorting_flag, m_time_client->get_time());
    auto order = [&] () -> std::shared_ptr<Order> {
      if(!session.has_permission(order_info.m_fields.m_account)) {
        auto order = make_rejected_order(
          order_info, "Insufficient permissions to execute order.");
        m_rejected_orders.push_back(order);
        return order;
      } else if(!order_info.m_fields.m_security.get_venue()) {
        auto order = make_rejected_order(order_info, "Market not specified.");
        m_rejected_orders.push_back(order);
        return order;
      } else if(order_info.m_fields.m_security.get_symbol().empty()) {
        auto order =
          make_rejected_order(order_info, "Ticker symbol not specified.");
        m_rejected_orders.push_back(order);
        return order;
      }
      return m_driver->submit(order_info);
    }();
    m_registry.publish(order_info,
      [&] {
        return load_initial_sequences(
          *m_data_store, order_info.m_fields.m_account);
      },
      [&] (const auto& info) {
        m_live_orders.insert((*info)->m_id);
        m_data_store->store(info);
        request.set(info);
        auto order_record = Beam::SequencedValue(Beam::IndexedValue(
          OrderRecord(**info, {}), info->get_index()), info.get_sequence());
        m_submission_subscriptions.publish(order_record,
          [&] (const auto& client) {
            return &client != &request.get_client();
          },
          [&] (const auto& clients) {
            Beam::broadcast_record_message<OrderSubmissionMessage>(
              clients, order_record);
          });
      });
    order->get_publisher().monitor(m_tasks.get_slot<ExecutionReport>(
      std::bind(&OrderExecutionServlet::on_execution_report, this,
        std::placeholders::_1, order_info.m_fields.m_account,
        std::ref(*shorting_model))));
  }

  template<typename C, typename T, typename S, typename U, typename A,
    typename O, typename D> requires
      Beam::IsTimeClient<Beam::dereference_t<T>> &&
        Beam::IsServiceLocatorClient<Beam::dereference_t<S>> &&
          Beam::IsUidClient<Beam::dereference_t<U>> &&
            IsAdministrationClient<Beam::dereference_t<A>> &&
              IsOrderExecutionDriver<Beam::dereference_t<O>> &&
                IsOrderExecutionDataStore<Beam::dereference_t<D>>
  void OrderExecutionServlet<C, T, S, U, A, O, D>::on_update_order_request(
      ServiceProtocolClient& client, OrderId id,
      const ExecutionReport& report) {
    auto& session = client.get_session();
    if(!session.is_administrator()) {
      boost::throw_with_location(
        Beam::ServiceRequestException("Insufficient permissions."));
    }
    auto sanitized_report = report;
    sanitized_report.m_id = id;
    if(sanitized_report.m_timestamp == boost::posix_time::not_a_date_time) {
      sanitized_report.m_timestamp = m_time_client->get_time();
    }
    m_driver->update(session, id, sanitized_report);
  }

  template<typename C, typename T, typename S, typename U, typename A,
    typename O, typename D> requires
      Beam::IsTimeClient<Beam::dereference_t<T>> &&
        Beam::IsServiceLocatorClient<Beam::dereference_t<S>> &&
          Beam::IsUidClient<Beam::dereference_t<U>> &&
            IsAdministrationClient<Beam::dereference_t<A>> &&
              IsOrderExecutionDriver<Beam::dereference_t<O>> &&
                IsOrderExecutionDataStore<Beam::dereference_t<D>>
  void OrderExecutionServlet<C, T, S, U, A, O, D>::on_cancel_order(
      ServiceProtocolClient& client, OrderId id) {
    auto& session = client.get_session();
    m_driver->cancel(session, id);
  }
}

#endif
