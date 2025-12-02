import * as Beam from 'beam';
import { Currency, Money, Region, Security, Venue } from '..';

/** Used to represent one of the various types of values used by a
 *  ComplianceParameter.
 */
export class ComplianceValue {

  /** Parses a ComplianceValue from JSON. */
  public static fromJson(value: any): ComplianceValue {
    const data = (() => {
      if(value.which === ComplianceValue.Type.BOOLEAN ||
          value.which === ComplianceValue.Type.DOUBLE ||
          value.which === ComplianceValue.Type.STRING) {
        return value.value;
      } else if(value.which === ComplianceValue.Type.QUANTITY) {
        return value.value / 1000000;
      } else if(value.which === ComplianceValue.Type.DATE_TIME) {
        return Beam.DateTime.fromJson(value.value);
      } else if(value.which === ComplianceValue.Type.DURATION) {
        return Beam.Duration.fromJson(value.value);
      } else if(value.which === ComplianceValue.Type.DIRECTORY_ENTRY) {
        return Beam.DirectoryEntry.fromJson(value.value);
      } else if(value.which === ComplianceValue.Type.CURRENCY) {
        return Currency.fromJson(value.value);
      } else if(value.which === ComplianceValue.Type.MONEY) {
        return Money.fromJson(value.value);
      } else if(value.which === ComplianceValue.Type.SECURITY) {
        return Security.fromJson(value.value);
      } else if(value.which === ComplianceValue.Type.VENUE) {
        return Venue.fromJson(value.value);
      } else if(value.which === ComplianceValue.Type.REGION) {
        return Region.fromJson(value.value);
      } else if(value.which === ComplianceValue.Type.LIST) {
        return Beam.arrayFromJson(ComplianceValue, value.value);
      }
      throw RangeError('Unknown type for ComplianceValue.');
    })();
    return new ComplianceValue(value.which, data);
  }

  /**
   * Constructs a ComplianceValue.
   * @param type The type of value to store.
   * @param value The raw value.
   */
  constructor(type: ComplianceValue.Type, value: any) {
    this._type = type;
    this._value = value;
  }

  /** Returns the type of value stored. */
  public get type(): ComplianceValue.Type {
    return this._type;
  }

  /** Returns the raw value. */
  public get value(): any {
    return this._value;
  }

  /** Converts this object to JSON. */
  public toJson(): any {
    const value = (() => {
      if(this._type === ComplianceValue.Type.BOOLEAN ||
          this._type === ComplianceValue.Type.DOUBLE ||
          this._type === ComplianceValue.Type.STRING) {
        return this._value;
      } else if(this._type === ComplianceValue.Type.QUANTITY) {
        return 1000000 * this._value;
      } else if(this._type === ComplianceValue.Type.LIST) {
        return Beam.arrayToJson(this._value as ComplianceValue[]);
      } else {
        return this._value.toJson();
      }
    })();
    return {
      which: this._type,
      value: value
    };
  }

  private _type: ComplianceValue.Type;
  private _value: any;
}

export module ComplianceValue {

  /** Enumerates the types of values used by a ComplianceValue object. */
  export enum Type {

    /** An invalid value. */
    NONE = -1,

    /** Resolves to a bool. */
    BOOLEAN = 0,

    /** Resolves to a double representing a quantity. */
    QUANTITY = 1,

    /** Resolves to a generic double. */
    DOUBLE = 2,

    /** Resolves to a string. */
    STRING = 3,

    /** Resolves to a Beam.DateTime. */
    DATE_TIME = 4,

    /** Resolves to a Beam.Duration. */
    DURATION = 5,

    /** Resolves to a Beam.DirectoryEntry. */
    DIRECTORY_ENTRY = 6,

    /** Resolves to a Currency. */
    CURRENCY = 7,

    /** Resolves to a Money. */
    MONEY = 8,

    /** Resolves to a Security. */
    SECURITY = 9,

    /** Resolves to a Venue. */
    VENUE = 10,

    /** Resolves to a Region. */
    REGION = 11,

    /** Resolves to a list of ComplianceValues. */
    LIST = 12
  }
}
