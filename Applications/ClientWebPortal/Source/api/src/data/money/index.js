class Money {
  constructor(number) {
    this.value = Number(number);
    if (isNaN(this.value)) {
      throw new 'Input must be a number.';
    }
  }

  toNumber() {
    return this.value / MULTIPLIER;
  }

  toString() {
    return (this.value / MULTIPLIER).toString();
  }

  equals(operand) {
    return this.value === operand.value;
  }

  compare(operand) {
    return this.value - operand.value;
  }

  add(operand) {
    return new Money(this.value + operand.value);
  }

  subtract(operand) {
    return new Money(this.value - operand.value);
  }

  multiply(operand) {
    let result = this.value * operand;
    result = Math.trunc(result);
    return new Money(result);
  }

  divide(operand) {
    let result = this.value / operand;
    result = Math.trunc(result);
    return new Money(result);
  }
}

Money.fromRepresentation = (value) => {
  return new Money(value);
}

Money.fromValue = (value) => {
  return new Money(value * MULTIPLIER);
}

const MULTIPLIER = 1000000;
const ZERO = Money.fromRepresentation(0);
const ONE = Money.fromRepresentation(MULTIPLIER);
const CENT = Money.fromRepresentation(MULTIPLIER / 100);
const BIP = Money.fromRepresentation(MULTIPLIER / 10000);
const EPSILON = Money.fromRepresentation(1);

Money.ZERO = ZERO;
Money.ONE = ONE;
Money.CENT = CENT;
Money.BIP = BIP;
Money.EPSILON = EPSILON;

export default Money;

