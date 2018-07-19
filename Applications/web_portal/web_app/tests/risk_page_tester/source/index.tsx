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
  }

  public componentDidMount() {
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
    return (
      <WebPortal.VBoxLayout width='100%' height='100%'>
        <WebPortal.Padding size='30px'/>
        <WebPortal.RiskParametersView parameters={parameters}
          breakpoint={this.state.breakpoint}
          currencyDatabase={Nexus.buildDefaultCurrencyDatabase()}/>
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

  private onScreenResize(): void {
    const newBreakpoint = TestApp.getBreakpoint();
    if(newBreakpoint !== this.state.breakpoint) {
      this.setState({breakpoint: newBreakpoint});
    }
  }
}

ReactDOM.render(<TestApp/>, document.getElementById('main'));
