import * as React from 'react';
import * as ReactDOM from 'react-dom';
import * as WebPortal from 'web_portal';

interface State {
  current: string;
}

class App extends React.Component<{}, State> {
  constructor(props: {}) {
    super(props);
    this.state = {
      current: 'account'
    };
  }

  public render(): JSX.Element {
    return (
      <WebPortal.NavigationHeader
          variant={WebPortal.NavigationTab.Variant.ICON_LABEL}
          current={this.state.current}
          onNavigate={(href) => this.setState({current: href})}>
        <WebPortal.NavigationTab
          icon='resources/account/account-grey.svg'
          highlightedIcon='resources/account/account-purple.svg'
          href='account'
          label='Account'/>
        <WebPortal.NavigationTab
          icon='resources/account/compliance-grey.svg'
          highlightedIcon='resources/account/compliance-purple.svg'
          href='compliance'
          label='Compliance'/>
        <WebPortal.NavigationTab
          icon='resources/account/entitlements-grey.svg'
          highlightedIcon='resources/account/entitlements-purple.svg'
          href='entitlements'
          label='Entitlements'/>
      </WebPortal.NavigationHeader>);
  }
}

ReactDOM.render(<App/>, document.getElementById('main'));
