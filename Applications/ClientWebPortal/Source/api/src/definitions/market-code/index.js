class MarketCode {
  constructor(code) {
    if (typeof code !== 'string' || code.length > 4) {
      throw new Error('Invalid market code.');
    }
    this.value = code;
  }

  equals(operand) {
    return this.value === operand.value;
  }

  toCode() {
    return this.value;
  }

  isEmpty() {
    return this.value == '';
  }

  clone() {
    return MarketCode.fromCode(this.value);
  }
}

MarketCode.fromCode = (code) => {
  return new MarketCode(code);
};

export default MarketCode;
