/** Used to represent money values. */
export class Money {

  /** The number of decimal places that can be represented. */
  public static readonly DECIMAL_PLACES = 6;

  /** The multiplier used. */
  public static readonly MULTIPLIER = 1000000;

  /** A value of 0.00 */
  public static readonly ZERO = Money.from_representation(0);

  /** The smallest unit of Money. */
  public static readonly EPSILON = Money.from_representation(1);

  /** A value of 0.0001 */
  public static readonly BIP = Money.from_representation(
    Money.MULTIPLIER / 10000);

  /** A value of 0.01 */
  public static readonly CENT = Money.from_representation(
    Money.MULTIPLIER / 100);

  /** A value of 1.00 */
  public static readonly ONE = Money.from_representation(Money.MULTIPLIER);

  /** Constructs a Money value from its raw internal representation. */
  public static from_representation(value: number): Money {
    const money = new Money('0');
    money._value = value;
    return money;
  }

  /** Makes a value from a JSON object. */
  public static fromJson(value: any): Money {
    return Money.from_representation(value);
  }

  /** Parses a value from a string. */
  public static parse(value: string): Money {
    if(value === '') {
      return null;
    }
    let i = 0;
    let sign: number;
    if(value[i] === '-') {
      sign = -1;
      ++i;
    } else {
      sign = 1;
      if(value[i] === '+') {
        ++i;
      }
    }
    let leftHand = 0;
    let hasDecimals = false;
    while(i !== value.length) {
      if(value[i] >= '0' && value[i] <= '9') {
        leftHand *= 10;
        leftHand += value.charCodeAt(i) - 48;
        ++i;
      } else if(value[i] === '.') {
        hasDecimals = true;
        ++i;
        break;
      } else {
        return null;
      }
    }
    let rightHand = 0;
    let multiplier = Math.pow(10, Money.DECIMAL_PLACES);
    if(hasDecimals) {
      let decimalPlaces = 0;
      while(i != value.length && decimalPlaces < Money.DECIMAL_PLACES) {
        if(value[i] >= '0' && value[i] <= '9') {
          rightHand *= 10;
          rightHand += value.charCodeAt(i) - 48;
          ++i;
          multiplier /= 10;
          ++decimalPlaces;
        } else {
          return null;
        }
      }
    }
    let finalValue = sign * (leftHand * Math.pow(10, Money.DECIMAL_PLACES) +
      rightHand * multiplier);
    return Money.from_representation(Math.trunc(finalValue));
  }

  /** Constructs a Money value from a string. */
  constructor(value: string) {
    const parsed = Money.parse(value);
    if(!parsed) {
      throw new Error(`Invalid Money value: "${value}".`);
    }
    this._value = parsed._value;
  }

  /** Adds two values together. */
  public add(value: Money): Money {
    return Money.from_representation(this._value + value._value);
  }

  /** Subtracts two values from each other. */
  public subtract(value: Money): Money {
    return Money.from_representation(this._value - value._value);
  }

  /** Multiplies this value by a scalar. */
  public multiply(value: number): Money {
    return Money.from_representation(value * this._value);
  }

  /** Divides this value by a scalar. */
  public divide(value: number): Money {
    return Money.from_representation(this._value / value);
  }

  /** Compares two values. */
  public compare(other: Money): number {
    return this._value - other._value;
  }

  /** Tests two values for equality. */
  public equals(other: Money): boolean {
    return other && this._value === other._value;
  }

  /** Returns the string representation. */
  public toString(): string {
    let buffer = '';
    if(this._value < 0) {
      buffer += '-';
    }
    const dollars = Math.trunc(Math.abs(this._value) / Money.MULTIPLIER);
    buffer += dollars.toString();
    buffer += '.';
    let fractionalPart = Math.abs(this._value) - (Money.MULTIPLIER * dollars);
    for(let i = 0; i < 2 || fractionalPart != 0; ++i) {
      const factor = Math.pow(10, Money.DECIMAL_PLACES - i - 1);
      const code = 48 + Math.trunc(fractionalPart / factor);
      buffer += String.fromCharCode(code);
      fractionalPart %= factor;
    }
    return buffer;
  }

  /** Converts this value to JSON. */
  public toJson(): any {
    return this._value;
  }

  private _value: number;
}
