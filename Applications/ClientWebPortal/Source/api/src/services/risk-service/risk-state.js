class RiskState {
  constructor(riskState, expiry) {
    this.riskState = riskState;
    this.expiry = expiry || '00000000T000000';
  }

  toData() {
    return {
      type: this.riskState,
      expiry: this.expiry
    };
  }
}

RiskState.fromData = (data) => {
  return new RiskState(data.type, data.expiry);
}

const ACTIVE = 0;
const CLOSE_ORDERS = 1;
const DISABLED = 2;

RiskState.ACTIVE = ACTIVE;
RiskState.CLOSE_ORDERS = CLOSE_ORDERS;
RiskState.DISABLED = DISABLED;

export default RiskState;
