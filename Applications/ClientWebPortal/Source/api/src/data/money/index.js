const SHIFT_DIGITS = 1000000;

class Money {
  constructor(number) {
    this.value = Number(number);
    if (isNaN(this.value)) {
      throw new 'Input must be a number.';
    }
    this.value *= SHIFT_DIGITS;
  }

  getAmount() {
    return this.value / SHIFT_DIGITS;
  }

  toString() {
    return this.value.toString();
  }

  equals(operand) {
    if (this.getAmount() === operand.getAmount()) {
      return true;
    } else {
      return false;
    }
  }

  compare(operand) {
    if (operand instanceof Money) {
      if (this.getAmount() > operand.getAmount()) {
        return 1;
      } else if (this.getAmount() < operand.getAmount()) {
        return -1;
      } else {
        return 0;
      }
    } else {
      if (this.getAmount() > operand) {
        return 1;
      } else if (this.getAmount() < operand) {
        return -1;
      } else {
        return 0;
      }
    }
  }

  add(operand) {
    if (operand instanceof Money) {
      return new Money(this.getAmount() + operand.getAmount());
    } else {
      return new Money(this.getAmount() + operand);
    }
  }

  subtract(operand) {
    if (operand instanceof Money) {
      return new Money(this.getAmount() - operand.getAmount());
    } else {
      return new Money(this.getAmount() - operand);
    }
  }

  multiply(operand) {
    if (operand instanceof Money) {
      return new Money(this.getAmount() * operand.getAmount());
    } else {
      return new Money(this.getAmount() * operand);
    }
  }

  divide(operand) {
    if (operand instanceof Money) {
      return new Money(this.getAmount() / operand.getAmount());
    } else {
      return new Money(this.getAmount() / operand);
    }
  }
}

Money.ZERO = new Money(0);
Money.ONE = new Money(1);
Money.CENT = new Money(0.01);
Money.BIP = new Money(0.0001);
Money.EPSILON = new Money(0.000001);

export default Money;
