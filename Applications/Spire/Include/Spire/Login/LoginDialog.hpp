#ifndef SPIRE_LOGINDIALOG_HPP
#define SPIRE_LOGINDIALOG_HPP
#include <memory>
#include <Beam/Network/IpAddress.hpp>
#include <Beam/Pointers/Ref.hpp>
#include <Beam/Queues/RoutineTaskQueue.hpp>
#include <Beam/Queues/TaskQueue.hpp>
#include <Beam/ServiceLocator/ApplicationDefinitions.hpp>
#include <Beam/ServiceLocator/ServiceLocator.hpp>
#include <Beam/Threading/Threading.hpp>
#include <QDialog>
#include <QPoint>
#include <QTimer>
#include "Spire/Spire/Spire.hpp"

class Ui_LoginDialog;

namespace Spire {
  class LoginDialog : public QDialog {
    public:
      struct ServerInstance {
        std::string m_name;
        Beam::Network::IpAddress m_address;
        ServerInstance() = default;
        ServerInstance(std::string name, Beam::Network::IpAddress address);
      };

      LoginDialog(std::vector<ServerInstance> instances,
        Beam::RefType<Beam::Network::SocketThreadPool> socketThreadPool,
        Beam::RefType<Beam::Threading::TimerThreadPool> timerThreadPool);
      virtual ~LoginDialog();
      std::unique_ptr<ServiceClients> GetServiceClients();

    protected:
      void mousePressEvent(QMouseEvent* event) override;
      void mouseMoveEvent(QMouseEvent* event) override;

    private:
      enum class State {
        READY,
        LOGGING_IN
      };
      std::unique_ptr<Ui_LoginDialog> m_ui;
      QPoint m_mousePoint;
      std::vector<ServerInstance> m_instances;
      Beam::Network::SocketThreadPool* m_socketThreadPool;
      Beam::Threading::TimerThreadPool* m_timerThreadPool;
      std::unique_ptr<ServiceClients> m_serviceClients;
      State m_state;
      QTimer m_updateTimer;
      int m_loginCount;
      Beam::TaskQueue m_slotHandler;
      Beam::RoutineTaskQueue m_tasks;

      void SetReadyState(const QString& response);
      void UpdatePasswordColor();
      void HandleLoginButtonClicked();
      void HandleAuthenticationError();
      void HandleConnectionError();
      void HandleSuccess();
      void HandlePasswordTextChanged(const QString& text);
  };
}

#endif
