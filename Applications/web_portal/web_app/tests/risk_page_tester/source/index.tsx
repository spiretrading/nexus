import {css, StyleSheet} from 'aphrodite';
import * as Beam from 'beam';
import * as Nexus from 'nexus';
import * as React from 'react';
import * as ReactDOM from 'react-dom';
import * as WebPortal from 'web_portal';

class TestApp extends React.Component{
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
          currencyDatabase={Nexus.buildDefaultCurrencyDatabase()}/>
      </WebPortal.VBoxLayout>);
  }
}

ReactDOM.render(<TestApp/>, document.getElementById('main'));
