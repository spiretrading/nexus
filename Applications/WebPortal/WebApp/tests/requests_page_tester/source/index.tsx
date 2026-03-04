import * as Nexus from 'nexus';
import * as React from 'react';
import * as ReactDOM from 'react-dom';
import * as WebPortal from 'web_portal';

interface State {
  current: WebPortal.RequestsPage.Page;
}

class App extends React.Component<{}, State> {
  constructor(props: {}) {
    super(props);
    this.state = {
      current: WebPortal.RequestsPage.Page.YOUR_REQUESTS
    };
  }

  public render(): JSX.Element {
    return (
      <WebPortal.RequestsPage
        roles={App.ROLES}
        current={this.state.current}
        onNavigate={(page) => this.setState({current: page})}/>);
  }

  private static ROLES = (() => {
    const roles = new Nexus.AccountRoles();
    roles.set(Nexus.AccountRoles.Role.ADMINISTRATOR);
    return roles;
  })();
}

ReactDOM.render(<App/>, document.getElementById('main'));
