class MarketCode {
  constructor(code) {
    if (typeof code !== 'string' || code.length > 4) {
      throw new Error('Invalid market code.');
    }
    this.value = code;

    this.toData = this.toData.bind(this);
  }

  equals(operand) {
    return this.value === operand.value;
  }

  toData() {
    return this.value;
  }

  isEmpty() {
    return this.value == '';
  }

  clone() {
    return MarketCode.fromData(this.value);
  }
}

MarketCode.fromData = (code) => {
  return new MarketCode(code);
};

export default MarketCode;
