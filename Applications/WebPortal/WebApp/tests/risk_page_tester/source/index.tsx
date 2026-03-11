import { css, StyleSheet } from 'aphrodite';
import * as Beam from 'beam';
import * as Dali from 'dali';
import * as Nexus from 'nexus';
import * as React from 'react';
import * as ReactDOM from 'react-dom';
import * as WebPortal from 'web_portal';

class TestRiskModel extends WebPortal.LocalRiskModel {
  public shouldFail = false;

  public async submit(comment: string,
      riskParameters: Nexus.RiskParameters): Promise<void> {
    if(this.shouldFail) {
      throw Error('Not Saved');
    }
  }
}

interface State {
  roles: Nexus.AccountRoles;
  shouldFail: boolean;
}

/** Displays and tests the RiskPage. */
class TestApp extends React.Component<{}, State> {
  constructor(props: {}) {
    super(props);
    this.state = {
      roles: new Nexus.AccountRoles(8),
      shouldFail: false,
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
      if(this.state.shouldFail) {
        return 'Error';
      }
      return 'No Error';
    })();
    return (
      <Dali.VBoxLayout width='100%' height='100%'>
        <WebPortal.RiskController
          currencyDatabase={Nexus.buildDefaultCurrencyDatabase()}
          roles={this.state.roles}
          model={this.model}/>
        <div className={css(TestApp.STYLE.buttonWrapper)}>
          <button onClick={this.onToggleIsAdmin}>
            {toggleAdminButtonText}
          </button>
          <button onClick={this.onToggleError}>
            {toggleErrorText}
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
    this.setState({roles});
  }

  private onToggleError = () => {
    this.model.shouldFail = !this.state.shouldFail;
    this.setState({shouldFail: !this.state.shouldFail});
  }

  private model = new TestRiskModel(
    new Beam.DirectoryEntry(Beam.DirectoryEntry.Type.ACCOUNT, 123, 'test'),
    new Nexus.RiskParameters(Nexus.DefaultCurrencies.CAD,
      Nexus.Money.ONE.multiply(100000),
      new Nexus.RiskState(Nexus.RiskState.Type.ACTIVE),
      Nexus.Money.ONE.multiply(1000),
      Beam.Duration.HOUR.multiply(5).add(
        Beam.Duration.MINUTE.multiply(30)).add(
          Beam.Duration.SECOND.multiply(15))));

  private static STYLE = StyleSheet.create({
    buttonWrapper: {
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      position: 'absolute' as 'absolute'
    }
  });
}

ReactDOM.render(<TestApp/>, document.getElementById('main'));
