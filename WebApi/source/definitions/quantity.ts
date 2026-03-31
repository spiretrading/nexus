/** Used to represent a quantity up to 15 significant decimal places. */
export class Quantity {

  /** The number of decimal places that can be represented. */
  public static readonly DECIMAL_PLACES = 6;

  /** The multiplier used. */
  public static readonly MULTIPLIER = 1000000;

  /** A value of 0. */
  public static readonly ZERO = Quantity.from_representation(0);

  /** A value of 1. */
  public static readonly ONE = Quantity.from_representation(
    Quantity.MULTIPLIER);

  /** Constructs a Quantity value from its raw internal representation. */
  public static from_representation(value: number): Quantity {
    const quantity = new Quantity('0');
    quantity._value = value;
    return quantity;
  }

  /** Makes a value from a JSON object. */
  public static fromJson(value: any): Quantity {
    return Quantity.from_representation(value);
  }

  /** Parses a value from a string. */
  public static parse(value: string): Quantity {
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
    let multiplier = Math.pow(10, Quantity.DECIMAL_PLACES);
    if(hasDecimals) {
      let decimalPlaces = 0;
      while(i != value.length && decimalPlaces < Quantity.DECIMAL_PLACES) {
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
    let finalValue = sign * (leftHand * Math.pow(10, Quantity.DECIMAL_PLACES) +
      rightHand * multiplier);
    return Quantity.from_representation(Math.trunc(finalValue));
  }

  /** Constructs a Quantity value from a string. */
  constructor(value: string) {
    const parsed = Quantity.parse(value);
    if(!parsed) {
      throw new Error(`Invalid Quantity value: "${value}".`);
    }
    this._value = parsed._value;
  }

  /** Adds two values together. */
  public add(value: Quantity): Quantity {
    return Quantity.from_representation(this._value + value._value);
  }

  /** Subtracts two values from each other. */
  public subtract(value: Quantity): Quantity {
    return Quantity.from_representation(this._value - value._value);
  }

  /** Multiplies this value by a scalar. */
  public multiply(value: number): Quantity {
    return Quantity.from_representation(value * this._value);
  }

  /** Divides this value by a scalar. */
  public divide(value: number): Quantity {
    return Quantity.from_representation(this._value / value);
  }

  /** Compares two values. */
  public compare(other: Quantity): number {
    return this._value - other._value;
  }

  /** Tests two values for equality. */
  public equals(other: Quantity): boolean {
    return other && this._value === other._value;
  }

  /** Returns the string representation. */
  public toString(): string {
    let buffer = '';
    if(this._value < 0) {
      buffer += '-';
    }
    const whole = Math.trunc(Math.abs(this._value) / Quantity.MULTIPLIER);
    buffer += whole.toString();
    let fractionalPart = Math.abs(this._value) - (Quantity.MULTIPLIER * whole);
    if(fractionalPart != 0) {
      buffer += '.';
      for(let i = 0; i < Quantity.DECIMAL_PLACES && fractionalPart != 0; ++i) {
        const factor = Math.pow(10, Quantity.DECIMAL_PLACES - i - 1);
        const code = 48 + Math.trunc(fractionalPart / factor);
        buffer += String.fromCharCode(code);
        fractionalPart %= factor;
      }
    }
    return buffer;
  }

  /** Converts this value to JSON. */
  public toJson(): any {
    return this._value;
  }

  private _value: number;
}
