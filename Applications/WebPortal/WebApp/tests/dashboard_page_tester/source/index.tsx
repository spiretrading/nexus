import * as Beam from 'beam';
import * as Nexus from 'nexus';
import * as React from 'react';
import * as ReactDOM from 'react-dom';
import * as Router from 'react-router-dom';
import * as WebPortal from 'web_portal';

const INITIAL_NOTIFICATIONS: Nexus.Notification[] = [
  new Nexus.Notification('1', Beam.DirectoryEntry.INVALID,
    'Your request to update risk controls for achen01 has been approved.',
    Nexus.Notification.Category.ACCOUNT_MODIFICATION,
    Beam.DateTime.fromDate((() => {
      const d = new Date(); d.setHours(d.getHours() - 2); return d;
    })()), false),
  new Nexus.Notification('2', Beam.DirectoryEntry.INVALID,
    'New entitlements request from jberrios01 requires your review.',
    Nexus.Notification.Category.ACCOUNT_MODIFICATION,
    Beam.DateTime.fromDate((() => {
      const d = new Date(); d.setDate(d.getDate() - 1); return d;
    })()), false),
  new Nexus.Notification('3', Beam.DirectoryEntry.INVALID,
    'Risk parameters for trodriguez have been updated.',
    Nexus.Notification.Category.REPORT,
    Beam.DateTime.fromDate((() => {
      const d = new Date(); d.setDate(d.getDate() - 3); return d;
    })()), true)
];

const roles = new Nexus.AccountRoles(0);

interface State {
  notifications: Nexus.Notification[];
}

class App extends React.Component<{}, State> {
  constructor(props: {}) {
    super(props);
    this.state = {
      notifications: INITIAL_NOTIFICATIONS
    };
  }

  public render(): JSX.Element {
    return (
      <Router.BrowserRouter>
        <WebPortal.DashboardPage roles={roles}
          notifications={this.state.notifications}
          onDismissAll={this.onDismissAll}/>
      </Router.BrowserRouter>);
  }

  private onDismissAll = () => {
    this.setState({notifications: []});
  };
}

ReactDOM.render(<App/>, document.getElementById('main'));
