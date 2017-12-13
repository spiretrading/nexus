class RiskState {
  constructor(type, expiry) {
    this.type = type;
    this.expiry = expiry || 'not-a-date-time';
  }

  toData() {
    return {
      type: this.type,
      expiry: this.expiry
    };
  }

  clone() {
    return new RiskState(
      this.type,
      this.expiry
    );
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
