import Money from '../../../../dist/definitions/money';

describe("Money", function() {
  beforeAll(function() {
    this.money = Money.fromNumber(12345);
  });

  it("Money object from number.", function() {
    let money = Money.fromNumber(12345);
    expect(12345000000).toBe(money.value);
  });

  it("Money object from representation.", function() {
    let money = Money.fromRepresentation(12345);
    expect(12345).toBe(money.value);
  });

  it("Money object from representation.", function() {
    let money = Money.fromRepresentation(12345);
    expect(12345).toBe(money.value);
  });

  it("Money object to number.", function() {
    expect(12345).toBe(this.money.toNumber());
  });

  it("Equals.", function() {
    let moneyB = Money.fromNumber(12345);
    let isEquals = moneyB.equals(this.money);
    expect(true).toBe(isEquals);
  });

  it("Compare.", function() {
    let pivot = Money.fromNumber(12345);
    let less = Money.fromNumber(10000);
    let more = Money.fromNumber(20000);
    let equal = Money.fromNumber(12345);

    let isLess = less.compare(pivot) < 0;
    let isMore = more.compare(pivot) > 0;
    let isEqual = equal.compare(pivot) === 0;

    expect(true).toBe(isLess);
    expect(true).toBe(isMore);
    expect(true).toBe(isEqual);
  });

  it("Add.", function() {
    let moneyB = Money.fromNumber(268457);
    let sum = moneyB.add(this.money);
    expect(280802).toBe(sum.toNumber());
  });

  it("Subtract.", function() {
    let moneyB = Money.fromNumber(268457);
    let difference = moneyB.subtract(this.money);
    expect(256112).toBe(difference.toNumber());
  });

  it("Multiplication.", function() {
    let moneyB = this.money.multiply(7);
    expect(86415).toBe(moneyB.toNumber());
  });

  it("Division.", function() {
    let moneyB = this.money.divide(3);
    expect(4115).toBe(moneyB.toNumber());
  });
});