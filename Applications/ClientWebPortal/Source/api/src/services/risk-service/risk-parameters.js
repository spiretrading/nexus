import CurrencyId from '../../definitions/currency/id';
import Money from '../../definitions/money';
import RiskState from './risk-state';

class RiskParameters {
  constructor(currencyId, buyingPower, allowedState, netLoss, transitionTime) {
    this.currencyId = currencyId;
    this.buyingPower = buyingPower;
    this.allowedState = allowedState;
    this.netLoss = netLoss;
    this.transitionTime = transitionTime;
  }

  toData() {
    return {
      currency: this.currencyId.toData(),
      buying_power: this.buyingPower.toData(),
      allowed_state: this.allowedState.toData(),
      net_loss: this.netLoss.toData(),
      transition_time: this.transitionTime
    };
  }
}

RiskParameters.fromData = (data) => {
  let currencyId = CurrencyId.fromNumber(data.currency);
  let buyingPower = Money.fromValue(data.buying_power);
  let allowedState = RiskState.fromData(data.allowed_state);
  let netLoss = Money.fromValue(data.net_loss);
  let transitionTime = data.transition_time;

  return new RiskParameters(
    currencyId,
    buyingPower,
    allowedState,
    netLoss,
    transitionTime
  );
}

export default RiskParameters;
