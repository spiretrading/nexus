import * as Beam from 'beam';
import * as Nexus from 'nexus';
import * as React from 'react';
import * as ReactDOM from 'react-dom';
import * as WebPortal from 'web_portal';

const parameters = new Nexus.RiskParameters(
  Nexus.DefaultCurrencies.CAD, Nexus.Money.ONE.multiply(100000),
  new Nexus.RiskState(Nexus.RiskState.Type.ACTIVE),
  Nexus.Money.ONE.multiply(1000), 100,
  Beam.Duration.HOUR.multiply(5).add(Beam.Duration.MINUTE.multiply(30)).add(
  Beam.Duration.SECOND.multiply(15)));
const onCurrencyChange = (currency: Nexus.Currency) => {
	console.log(currency);
};

const onBuyingPowerChange = (value: Nexus.Money) => {
	console.log(value);
};

const onNetLossChange = (value: Nexus.Money) => {
	console.log(value);
};

ReactDOM.render(<WebPortal.RiskParametersView parameters={parameters}
  currencyDatabase={Nexus.buildDefaultCurrencyDatabase()}
  onCurrencyChange={onCurrencyChange}
  onBuyingPowerChange={onBuyingPowerChange}
  onNetLossChange={onNetLossChange}
  />,
  document.getElementById('main'));
