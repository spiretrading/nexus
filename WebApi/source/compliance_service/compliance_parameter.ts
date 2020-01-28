import { ComplianceValue } from './compliance_value';

/** Stores a single parameter used by a compliance rule. */
export class ComplianceParameter {

  /** Parses a ComplianceParameter from JSON. */
  public static fromJson(value: any): ComplianceParameter {
    return new ComplianceParameter(value.name,
      ComplianceValue.fromJson(value.value));
  }

  /** Constructs a ComplianceParameter. */
  constructor(name: string, value: ComplianceValue) {
    this._name = name;
    this._value = value;
  }

  /** Returns the name of the parameter. */
  public get name(): string {
    return this._name;
  }

  /** Returns the parameter's value. */
  public get value(): ComplianceValue {
    return this._value;
  }

  /** Converts this object to JSON. */
  public toJson(): any {
    return {
      name: this._name,
      value: this._value.toJson()
    };
  }

  private _name: string;
  private _value: ComplianceValue;
}
