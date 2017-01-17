import CurrencyId from '../../definitions/currency/id';
import Money from '../../definitions/money';
import RiskState from './risk-state';

class RiskParameters {
  constructor(currencyId, buyingPower, allowedState, netLoss, transitionTime, lossFromTop) {
    this.currencyId = currencyId;
    this.buyingPower = buyingPower;
    this.allowedState = allowedState;
    this.netLoss = netLoss;
    this.transitionTime = transitionTime;
    this.lossFromTop = lossFromTop;
  }

  toData() {
    return {
      currency: this.currencyId.toData(),
      buying_power: this.buyingPower.toData(),
      allowed_state: this.allowedState.toData(),
      net_loss: this.netLoss.toData(),
      transition_time: this.transitionTime,
      loss_from_top: this.lossFromTop.toData()
    };
  }

  clone() {
    return new RiskParameters(
      this.currencyId.clone(),
      this.buyingPower.clone(),
      this.allowedState.clone(),
      this.netLoss.clone(),
      this.transitionTime,
      this.lossFromTop.clone()
    );
  }
}

RiskParameters.fromData = (data) => {
  let currencyId = CurrencyId.fromNumber(data.currency);
  let buyingPower = Money.fromRepresentation(data.buying_power);
  let allowedState = RiskState.fromData(data.allowed_state);
  let netLoss = Money.fromRepresentation(data.net_loss);
  let transitionTime = data.transition_time;
  let lossFromTop = Money.fromRepresentation(data.loss_from_top);

  return new RiskParameters(
    currencyId,
    buyingPower,
    allowedState,
    netLoss,
    transitionTime,
    lossFromTop
  );
}

export default RiskParameters;
