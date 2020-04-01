import { css, StyleSheet } from 'aphrodite';
import * as Beam from 'beam';
import * as Dali from 'dali';
import * as Nexus from 'nexus';
import * as React from 'react';
import * as ReactDOM from 'react-dom';
import * as WebPortal from 'web_portal';

 /** Determines the size to render components at. */
interface Properties {
  displaySize: WebPortal.DisplaySize;
}

interface State {
  roles: Nexus.AccountRoles;
  status: string,
  isSubmitEnabled: boolean,
  isError: boolean,
}

class TestApp extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {
      roles: new Nexus.AccountRoles(8),
      status: '',
      isSubmitEnabled: false,
      isError: false,
    };
    this.onSubmit = this.onSubmit.bind(this);
    this.onToggleIsAdmin = this.onToggleIsAdmin.bind(this);
    this.onToggleEnabled = this.onToggleEnabled.bind(this);
    this.onToggleError = this.onToggleError.bind(this);
    this.onSubmit = this.onSubmit.bind(this);
    this.clearStatus = this.clearStatus.bind(this);
  }

  public render(): JSX.Element {
    const parameters = new Nexus.RiskParameters(
      Nexus.DefaultCurrencies.CAD,
      Nexus.Money.ONE.multiply(100000),
      new Nexus.RiskState(Nexus.RiskState.Type.ACTIVE),
      Nexus.Money.ONE.multiply(1000), 100,
      Beam.Duration.HOUR.multiply(5).add(Beam.Duration.MINUTE.multiply(30)).add(
      Beam.Duration.SECOND.multiply(15)));
    const toggleAdminButtonText = (() => {
      if(this.state.roles.test(Nexus.AccountRoles.Role.ADMINISTRATOR)) {
        return 'Admin';
      }
      return 'Not Admin';
    })();
    const toggleEnabledText = (() => {
      if(this.state.isSubmitEnabled) {
        return 'Enabled';
      }
      return 'Not Enabled';
    })();
    const toggleErrorText = (() => {
      if(this.state.isError) {
        return 'Error';
      }
      return 'No Error';
    })();
    return (
      <Dali.VBoxLayout width='100%' height='100%'>
        <WebPortal.RiskPage
          displaySize={this.props.displaySize}
          parameters={parameters}
          currencyDatabase={Nexus.buildDefaultCurrencyDatabase()}
          roles={this.state.roles}
          status={this.state.status}
          isError={this.state.isError}
          isSubmitEnabled={this.state.isSubmitEnabled}
          onSubmit={this.onSubmit}/>
        <div className={css(TestApp.STYLE.buttonWrapper)}>
          <button className={css(TestApp.STYLE.button)} onClick={
              this.onToggleIsAdmin}>
            {toggleAdminButtonText}
          </button>
          <button onClick={this.onToggleEnabled}>
            {toggleEnabledText}
          </button>
          <button onClick={this.onToggleError}>
            {toggleErrorText}
          </button>
          <button onClick={this.clearStatus}>
            Clear Status
          </button>
       </div>
      </Dali.VBoxLayout>);
  }

  private onToggleIsAdmin() {
    const roles = (() => {
      if(!this.state.roles.test(Nexus.AccountRoles.Role.ADMINISTRATOR)) {
        return new Nexus.AccountRoles(8);
      }
      return new Nexus.AccountRoles();
    })();
    this.setState({ roles: roles });
  }

  private onToggleEnabled() {
    this.setState({isSubmitEnabled: !this.state.isSubmitEnabled});
  }

  private onToggleError() {
    this.setState({isError: !this.state.isError});
  }

  private onSubmit(comments: string, parameters: Nexus.RiskParameters) {
    if(this.state.isError) {
      this.setState({status: 'Not Saved'});
    } else {
      this.setState({status: 'Saved'});
    }
    return;
  }

  private clearStatus() {
    this.setState({status: ''});
  }

  private static STYLE = StyleSheet.create({
    outerContainer: {
      position: 'relative' as 'relative'
    },
    buttonWrapper: {
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      position: 'absolute' as 'absolute'
    },
    button: {
      //position: 'absolute' as 'absolute'
    }
  });
}

const ResponsivePage = WebPortal.displaySizeRenderer(TestApp);
ReactDOM.render(<ResponsivePage/>, document.getElementById('main'));
