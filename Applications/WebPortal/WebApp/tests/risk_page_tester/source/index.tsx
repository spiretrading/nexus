import { css, StyleSheet } from 'aphrodite';
import * as Beam from 'beam';
import * as Dali from 'dali';
import * as Nexus from 'nexus';
import * as React from 'react';
import * as ReactDOM from 'react-dom';
import * as WebPortal from 'web_portal';

interface Properties {

  /** Determines the size to render components at. */
  displaySize: WebPortal.DisplaySize;
}

interface State {
  comment: string;
  parameters: Nexus.RiskParameters;
  roles: Nexus.AccountRoles;
  status: string,
  isError: boolean,
}

/** Displays and tests the RiskPage. */
class TestApp extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {
      comment: '',
      parameters: new Nexus.RiskParameters(Nexus.DefaultCurrencies.CAD,
        Nexus.Money.ONE.multiply(100000),
        new Nexus.RiskState(Nexus.RiskState.Type.ACTIVE),
        Nexus.Money.ONE.multiply(1000),
        Beam.Duration.HOUR.multiply(5).add(
          Beam.Duration.MINUTE.multiply(30)).add(
            Beam.Duration.SECOND.multiply(15))),
      roles: new Nexus.AccountRoles(8),
      status: '',
      isError: false,
    };
  }

  public render(): JSX.Element {
    const toggleAdminButtonText = (() => {
      if(this.state.roles.test(Nexus.AccountRoles.Role.ADMINISTRATOR)) {
        return 'Admin';
      }
      return 'Not Admin';
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
          comment={this.state.comment}
          parameters={this.state.parameters}
          currencyDatabase={Nexus.buildDefaultCurrencyDatabase()}
          roles={this.state.roles}
          status={this.state.status}
          isError={this.state.isError}
          onComment={this.onComment}
          onParameters={this.onParameters}
          onSubmit={this.onSubmit}/>
        <div className={css(TestApp.STYLE.buttonWrapper)}>
          <button onClick={this.onToggleIsAdmin}>
            {toggleAdminButtonText}
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

  private onToggleIsAdmin = () => {
    const roles = (() => {
      if(!this.state.roles.test(Nexus.AccountRoles.Role.ADMINISTRATOR)) {
        return new Nexus.AccountRoles(8);
      }
      return new Nexus.AccountRoles();
    })();
    this.setState({ roles: roles });
  }

  private onToggleError = () => {
    this.setState({isError: !this.state.isError});
  }

  private onComment = (comment: string) => {
    this.setState({comment});
  }

  private onParameters = (parameters: Nexus.RiskParameters) => {
    this.setState({parameters});
  }

  private onSubmit = (comment: string, parameters: Nexus.RiskParameters) => {
    if(this.state.isError) {
      this.setState({status: 'Not Saved'});
    } else {
      this.setState({status: 'Saved'});
    }
    return;
  }

  private clearStatus = () => {
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
    }
  });
}

const ResponsivePage = WebPortal.displaySizeRenderer(TestApp);
ReactDOM.render(<ResponsivePage/>, document.getElementById('main'));
