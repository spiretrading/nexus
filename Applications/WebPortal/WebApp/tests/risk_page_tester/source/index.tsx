import * as Beam from 'beam';
import * as Nexus from 'nexus';
import * as React from 'react';
import * as ReactDOM from 'react-dom';
import * as WebPortal from 'web_portal';

class TestRiskModel extends WebPortal.LocalRiskModel {
  public shouldFail = false;

  public async submit(comment: string,
      riskParameters: Nexus.RiskParameters,
      effectiveDate: Beam.Date): Promise<void> {
    if(this.shouldFail) {
      throw Error('Not Saved');
    }
  }
}

interface State {
  roles: Nexus.AccountRoles;
  shouldFail: boolean;
}

class TestApp extends React.Component<{}, State> {
  constructor(props: {}) {
    super(props);
    const roles = new Nexus.AccountRoles();
    roles.set(Nexus.AccountRoles.Role.ADMINISTRATOR);
    this.state = {
      roles,
      shouldFail: false
    };
  }

  public render(): JSX.Element {
    const isAdmin =
      this.state.roles.test(Nexus.AccountRoles.Role.ADMINISTRATOR);
    return (
      <div style={STYLE.wrapper}>
        <WebPortal.RiskController
          currencyDatabase={Nexus.buildDefaultCurrencyDatabase()}
          roles={this.state.roles}
          model={this.model}/>
        <div style={STYLE.toolbar}>
          <label style={STYLE.toggle}>
            <input type='checkbox' checked={isAdmin}
              onChange={this.onToggleAdmin}/>
            Admin
          </label>
          <label style={STYLE.toggle}>
            <input type='checkbox' checked={this.state.shouldFail}
              onChange={this.onToggleError}/>
            Error
          </label>
        </div>
      </div>);
  }

  private onToggleAdmin = () => {
    const roles = this.state.roles.clone();
    if(roles.test(Nexus.AccountRoles.Role.ADMINISTRATOR)) {
      roles.unset(Nexus.AccountRoles.Role.ADMINISTRATOR);
    } else {
      roles.set(Nexus.AccountRoles.Role.ADMINISTRATOR);
    }
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
}

const STYLE: Record<string, React.CSSProperties> = {
  wrapper: {
    width: '100%',
    height: '100%',
    display: 'flex',
    flexDirection: 'column'
  },
  toolbar: {
    display: 'flex',
    flexDirection: 'row',
    gap: '10px',
    position: 'absolute'
  },
  toggle: {
    display: 'flex',
    alignItems: 'center',
    gap: '4px',
    cursor: 'pointer'
  }
};

ReactDOM.render(<TestApp/>, document.getElementById('main'));
