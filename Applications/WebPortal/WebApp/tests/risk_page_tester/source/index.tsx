import {css, StyleSheet} from 'aphrodite';
import * as Beam from 'beam';
import * as Nexus from 'nexus';
import * as React from 'react';
import * as ReactDOM from 'react-dom';
import * as WebPortal from 'web_portal';

interface Properties {
}

interface State {
  breakpoint: WebPortal.DisplaySize;
  roles: Nexus.AccountRoles;
}

class TestApp extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    const roles = new Nexus.AccountRoles(8);
    this.state = {
      breakpoint: TestApp.getBreakpoint(),
      roles: roles
    };
    this.onScreenResize = this.onScreenResize.bind(this);
    this.onSubmit = this.onSubmit.bind(this);
    this.onToggleIsAdmin = this.onToggleIsAdmin.bind(this);
  }

  public componentDidMount(): void {
    window.addEventListener('resize', this.onScreenResize);
  }

  public componentWillUnmount(): void {
    window.removeEventListener('resize', this.onScreenResize);
  }

  public render(): JSX.Element {
    const parameters = new Nexus.RiskParameters(
      Nexus.DefaultCurrencies.CAD,
      Nexus.Money.ONE.multiply(100000),
      new Nexus.RiskState(Nexus.RiskState.Type.ACTIVE),
      Nexus.Money.ONE.multiply(1000), 100,
      Beam.Duration.HOUR.multiply(5).add(Beam.Duration.MINUTE.multiply(30)).add(
      Beam.Duration.SECOND.multiply(15)));

    const containerClassName = (() => {
      switch(this.state.breakpoint) {
        case WebPortal.DisplaySize.SMALL:
          return css([TestApp.CONTAINER_STYLE.small,
            TestApp.CONTAINER_STYLE.base]);
        case WebPortal.DisplaySize.MEDIUM:
          return css([TestApp.CONTAINER_STYLE.medium,
            TestApp.CONTAINER_STYLE.base]);
        case WebPortal.DisplaySize.LARGE:
          return css([TestApp.CONTAINER_STYLE.large,
            TestApp.CONTAINER_STYLE.base]);
        default:
          return css([TestApp.CONTAINER_STYLE.medium,
              TestApp.CONTAINER_STYLE.base]);
      }
    })();
    const submissionBoxPadding = (() => {
      switch(this.state.breakpoint) {
        case WebPortal.DisplaySize.SMALL:
          return <div className={css(TestApp.CONTAINER_STYLE.smallPadding)}/>;
        case WebPortal.DisplaySize.MEDIUM:
          return <div className={css(TestApp.CONTAINER_STYLE.mediumPadding)}/>;
        case WebPortal.DisplaySize.LARGE:
          return <div className={css(TestApp.CONTAINER_STYLE.largePadding)}/>;
        default:
          return <div className={css(TestApp.CONTAINER_STYLE.mediumPadding)}/>;
        }
    })();
    const toggleAdminButtonText = (() => {
      if(this.state.roles.isSet(Nexus.AccountRoles.Role.ADMINISTRATOR)) {
        return 'Admin';
      }
      return 'Not Admin';
    })();
    return (
      <WebPortal.VBoxLayout width='100%' height='100%'>
        <WebPortal.Padding size='30px'/>
        <WebPortal.HBoxLayout width='100%' className={
          css(TestApp.STYLE.outerContainer)}>
          <WebPortal.VBoxLayout className={containerClassName}>
            <WebPortal.RiskParametersView parameters={parameters}
              breakpoint={this.state.breakpoint}
              currencyDatabase={Nexus.buildDefaultCurrencyDatabase()}/>
            <WebPortal.Padding size='30px'/>
            <WebPortal.HBoxLayout width='100%'>
              <WebPortal.SubmissionBox ref={
                (ref: any) => this.submissionBox = ref}
                roles={this.state.roles} onClick={this.onSubmit}/>
            </WebPortal.HBoxLayout>
          </WebPortal.VBoxLayout>
        </WebPortal.HBoxLayout>
        <button className={css(TestApp.STYLE.button)} onClick={
            this.onToggleIsAdmin}>
          {toggleAdminButtonText}
        </button>
      </WebPortal.VBoxLayout>);
  }

  private static getBreakpoint(): WebPortal.DisplaySize {
    const screenWidth = window.innerWidth ||
      document.documentElement.clientWidth ||
      document.getElementsByTagName('body')[0].clientWidth;
    if(screenWidth <= 767) {
      return WebPortal.DisplaySize.SMALL;
    } else if(screenWidth > 767 && screenWidth <= 1035) {
      return WebPortal.DisplaySize.MEDIUM;
    } else {
      return WebPortal.DisplaySize.LARGE;
    }
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
  private onScreenResize(): void {
    const newBreakpoint = TestApp.getBreakpoint();
    if(newBreakpoint !== this.state.breakpoint) {
      this.setState({breakpoint: newBreakpoint});
    }
  }

  private onSubmit() {
    console.log(this.submissionBox.getComment());
  }
  private onToggleIsAdmin() {
    const roles = (() => {
      if(!this.state.roles.isSet(Nexus.AccountRoles.Role.ADMINISTRATOR)) {
        return new Nexus.AccountRoles(8);
      }
      return new Nexus.AccountRoles();
    })();
    this.setState({roles: roles});
  }
  private submissionBox: WebPortal.SubmissionBox;
}

ReactDOM.render(<TestApp/>, document.getElementById('main'));
