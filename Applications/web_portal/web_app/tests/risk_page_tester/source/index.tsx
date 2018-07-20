import {css, StyleSheet} from 'aphrodite';
import * as Beam from 'beam';
import * as Nexus from 'nexus';
import * as React from 'react';
import * as ReactDOM from 'react-dom';
import * as WebPortal from 'web_portal';

interface Properties {
}

interface State {
  breakpoint: WebPortal.RiskParametersView.Breakpoint
}

class TestApp extends React.Component<Properties, State>{
  constructor(props: Properties) {
    super(props);
    this.state = {
      breakpoint: TestApp.getBreakpoint()
    };
    this.onScreenResize = this.onScreenResize.bind(this);
    this.onSubmit = this.onSubmit.bind(this);
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
    const roles = new Nexus.AccountRoles(
      Nexus.AccountRoles.Role.TRADER |
      Nexus.AccountRoles.Role.MANAGER |
      Nexus.AccountRoles.Role.ADMINISTRATOR);
    console.log('upper roles: ', roles.isSet(Nexus.AccountRoles.Role.ADMINISTRATOR))
    const containerClassName = (() => {
      switch(this.state.breakpoint) {
        case WebPortal.RiskParametersView.Breakpoint.SMALL:
          return css([TestApp.CONTAINER_STYLE.small,
            TestApp.CONTAINER_STYLE.base]);
        case WebPortal.RiskParametersView.Breakpoint.MEDIUM:
          return css([TestApp.CONTAINER_STYLE.medium,
            TestApp.CONTAINER_STYLE.base]);
        case WebPortal.RiskParametersView.Breakpoint.LARGE:
          return css([TestApp.CONTAINER_STYLE.large,
            TestApp.CONTAINER_STYLE.base]);
        default:
          return css([TestApp.CONTAINER_STYLE.medium,
              TestApp.CONTAINER_STYLE.base]);
      }
    })();
    return (
      <WebPortal.VBoxLayout width='100%' height='100%'>
        <WebPortal.Padding size='30px'/>
        <WebPortal.HBoxLayout width='100%'>
          <WebPortal.Padding/>
          <WebPortal.VBoxLayout className={containerClassName}>
            <WebPortal.RiskParametersView parameters={parameters}
              breakpoint={this.state.breakpoint}
              currencyDatabase={Nexus.buildDefaultCurrencyDatabase()}/>
            <WebPortal.Padding size='30px'/>
            <WebPortal.SubmissionBox ref={
              (ref: any) => this.submissionBox = ref}
              roles={roles} onClick={this.onSubmit}/>
          </WebPortal.VBoxLayout>
          <WebPortal.Padding/>
        </WebPortal.HBoxLayout>
      </WebPortal.VBoxLayout>);
  }

  private static getBreakpoint(): WebPortal.RiskParametersView.Breakpoint {
    const screenWidth = window.innerWidth ||
      document.documentElement.clientWidth ||
      document.getElementsByTagName('body')[0].clientWidth;
    if(screenWidth <= 767) {
      return WebPortal.RiskParametersView.Breakpoint.SMALL;
    } else if(screenWidth > 767 && screenWidth <= 1035) {
      return WebPortal.RiskParametersView.Breakpoint.MEDIUM;
    } else {
      return WebPortal.RiskParametersView.Breakpoint.LARGE;
    }
  }
  private static CONTAINER_STYLE = StyleSheet.create({
    base: {
      
    },
    small: {
      width: '60%',
      minWidth: '320px',
      maxWidth: '460px'
    },
    medium: {
      width: '768px'
    },
    large: {
      width: '1036px'
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

  private submissionBox: WebPortal.SubmissionBox;
}

ReactDOM.render(<TestApp/>, document.getElementById('main'));
