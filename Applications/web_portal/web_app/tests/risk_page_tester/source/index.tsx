import {css, StyleSheet} from 'aphrodite';
import * as Beam from 'beam';
import * as Nexus from 'nexus';
import * as React from 'react';
import * as ReactDOM from 'react-dom';
import * as WebPortal from 'web_portal';

interface State {
  parameters: Nexus.RiskParameters;
}

interface Properties {

}

class TestApp extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {
      parameters: new Nexus.RiskParameters(
        Nexus.DefaultCurrencies.CAD,
        Nexus.Money.ONE.multiply(100000),
        new Nexus.RiskState(Nexus.RiskState.Type.ACTIVE),
        Nexus.Money.ONE.multiply(1000), 100,
        Beam.Duration.HOUR.multiply(5).add(Beam.Duration.MINUTE.multiply(30)).add(
        Beam.Duration.SECOND.multiply(15)))
    };
    this.onCurrencyChange = this.onCurrencyChange.bind(this);
    this.onBuyingPowerChange = this.onBuyingPowerChange.bind(this);
    this.onNetLossChange = this.onNetLossChange.bind(this);
    this.onTransitionTimeChange = this.onTransitionTimeChange.bind(this);
  }

  public render(): JSX.Element {
    return (
      <WebPortal.VBoxLayout width='100%' height='100%'>
        <WebPortal.Padding size='30px'/>
        <WebPortal.RiskParametersView parameters={this.state.parameters}
          currencyDatabase={Nexus.buildDefaultCurrencyDatabase()}
          onCurrencyChange={this.onCurrencyChange}
          onBuyingPowerChange={this.onBuyingPowerChange}
          onNetLossChange={this.onNetLossChange}
          onTransitionTimeChange={this.onTransitionTimeChange}/>
      </WebPortal.VBoxLayout>);
  }

  private onCurrencyChange(value: Nexus.Currency) {
    const newParameters = this.state.parameters;
    newParameters.currency = value;
    this.setState({parameters: newParameters})
  }

  private onBuyingPowerChange(value: Nexus.Money) {
    const newParameters = this.state.parameters;
    newParameters.buyingPower = value;
    this.setState({parameters: newParameters})
  }

  private onNetLossChange(value: Nexus.Money) {
    const newParameters = this.state.parameters;
    newParameters.netLoss = value;
    this.setState({parameters: newParameters})
  }

  private onTransitionTimeChange(value: Beam.Duration) {
    const newParameters = this.state.parameters;
    newParameters.transitionTime = value;
    this.setState({parameters: newParameters})
  }
}

ReactDOM.render(<TestApp/>, document.getElementById('main'));
