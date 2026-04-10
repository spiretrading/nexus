#include "Spire/TimeAndSales/TimeAndSalesWindowSettings.hpp"
#include <Beam/Utilities/ToString.hpp>
#include "Spire/LegacyUI/UserProfile.hpp"
#include "Spire/TimeAndSales/TimeAndSalesWindow.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/TableView.hpp"
#include "Spire/Ui/TickerView.hpp"

using namespace Beam;
using namespace Nexus;
using namespace Spire;

TimeAndSalesWindowSettings::TimeAndSalesWindowSettings(
    const TimeAndSalesWindow& window)
    : m_ticker_view(window.m_ticker_view->save_state()),
      m_identifier(window.GetIdentifier()),
      m_link_identifier(window.m_link_identifier),
      m_geometry(window.saveGeometry()) {
  if(window.m_table_view) {
    auto& widths = *window.m_table_view->get_header().get_widths();
    m_column_widths.insert(m_column_widths.end(), widths.begin(), widths.end());
  }
  if(auto& ticker = window.GetDisplayedTicker()) {
    m_name = "Time And Sales - " + to_string(ticker);
  } else {
    m_name = "Time And Sales";
  }
}

std::string TimeAndSalesWindowSettings::GetName() const {
  return m_name;
}

QWidget* TimeAndSalesWindowSettings::Reopen(
    Ref<UserProfile> user_profile) const {
  auto window = new TimeAndSalesWindow(user_profile->GetTickerInfoQueryModel(),
    user_profile->GetTimeAndSalesPropertiesWindowFactory(),
    user_profile->GetTimeAndSalesModelBuilder(), m_identifier);
  window->setAttribute(Qt::WA_DeleteOnClose);
  Apply(Ref(user_profile), out(*window));
  return window;
}

void TimeAndSalesWindowSettings::Apply(
    Ref<UserProfile> user_profile, Out<QWidget> widget) const {
  auto& window = dynamic_cast<TimeAndSalesWindow&>(*widget);
  restore_geometry(window, m_geometry);
  window.m_link_identifier = m_link_identifier;
  window.m_ticker_view->restore(m_ticker_view);
  if(window.m_table_view) {
    auto& widths = *window.m_table_view->get_header().get_widths();
    for(auto i = 0; i != m_column_widths.size(); ++i) {
      widths.set(i, m_column_widths[i]);
    }
  }
}
