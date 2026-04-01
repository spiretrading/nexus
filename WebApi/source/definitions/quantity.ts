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
    const quantity = Object.create(Quantity.prototype) as Quantity;
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
    var absolute = Math.abs(this._value);
    var whole = Math.trunc(absolute / Quantity.MULTIPLIER);
    var fraction = Math.trunc(absolute % Quantity.MULTIPLIER);
    if(fraction === 0) {
      if(this._value < 0) {
        return '-' + whole.toString();
      }
      return whole.toString();
    }
    var fractionalString =
      fraction.toString().padStart(Quantity.DECIMAL_PLACES, '0');
    var trimmed = fractionalString.length;
    while(trimmed > 0 && fractionalString[trimmed - 1] === '0') {
      --trimmed;
    }
    fractionalString = fractionalString.substring(0, trimmed);
    if(this._value < 0) {
      return '-' + whole.toString() + '.' + fractionalString;
    }
    return whole.toString() + '.' + fractionalString;
  }

  /** Converts this value to JSON. */
  public toJson(): any {
    return this._value;
  }

  private _value: number;
}
