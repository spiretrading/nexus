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
}

class TestApp extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {
      roles: new Nexus.AccountRoles(8)
    };
    this.onSubmit = this.onSubmit.bind(this);
    this.onToggleIsAdmin = this.onToggleIsAdmin.bind(this);
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
    return (
      <Dali.VBoxLayout width='100%' height='100%'>
        <WebPortal.RiskPage
           displaySize={this.props.displaySize}
           parameters={parameters}
           currencyDatabase={Nexus.buildDefaultCurrencyDatabase()}
           roles={this.state.roles}/>
        <button className={css(TestApp.STYLE.button)} onClick={
            this.onToggleIsAdmin}>
          {toggleAdminButtonText}
        </button>
      </Dali.VBoxLayout>);
  }

  private onSubmit(comment: string) {
    console.log(comment);
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

  private static STYLE = StyleSheet.create({
    outerContainer: {
      position: 'relative' as 'relative'
    },
    button: {
      position: 'absolute' as 'absolute'
    }
  });
  private static CONTAINER_STYLE = StyleSheet.create({
    base: {
      position: 'absolute' as 'absolute',
      left: 0,
      right: 0,
      margin: 'auto'
    },
    small: {
      width: '60%',
      minWidth: '320px',
      maxWidth: '460px'
    },
    medium: {
      width: '732px'
    },
    large: {
      width: '1000px'
    },
    smallPadding: {
      width: '20%'
    },
    mediumPadding: {
      width: 'calc(50% - 366px)'
    },
    largePadding: {
      width: 'calc(50% - 500px)'
    }
  });
}

const ResponsivePage = WebPortal.displaySizeRenderer(TestApp);
ReactDOM.render(<ResponsivePage/>, document.getElementById('main'));
