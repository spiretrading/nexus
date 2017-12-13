class CountryCode {
  constructor(number) {
    this.value = Number(number);
    if (isNaN(this.value)) {
      throw new Error('Input must be a number.');
    } else if (this.value % 1 != 0) {
      throw new Error('Input must be an integer.');
    }
  }

  equals(operand) {
    return this.value === operand.value;
  }

  compare(operand) {
    return this.value - operand.value;
  }

  toNumber() {
    return this.value;
  }

  clone() {
    return CountryCode.fromNumber(this.value);
  }
}

const NONE = 65535;

CountryCode.NONE = NONE;

CountryCode.fromNumber = (number) => {
  return new CountryCode(number);
};

export default CountryCode;
