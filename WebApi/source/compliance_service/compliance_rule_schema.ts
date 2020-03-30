import * as Beam from 'beam';
import { ComplianceParameter } from './compliance_parameter';

/** Provides a description for a single compliance rule. */
export class ComplianceRuleSchema {

  /** Parses a ComplianceRuleSchema from JSON. */
  public static fromJson(value: any): ComplianceRuleSchema {
    return new ComplianceRuleSchema(value.name,
      Beam.arrayFromJson(ComplianceParameter, value.parameters));
  }

  /** Constructs a ComplianceRuleSchema.
   * @param name The name of the rule.
   * @param parameters The list of parameters.
   */
  constructor(name: string, parameters: ComplianceParameter[]) {
    this._name = name;
    this._parameters = parameters.slice();
    Object.freeze(this._parameters);
  }

  /** Returns the name of the rule. */
  public get name(): string {
    return this._name;
  }

  /** Returns the rule's parameters. */
  public get parameters(): ComplianceParameter[] {
    return this._parameters;
  }

  /** Converts this object to JSON. */
  public toJson(): any {
    return {
      name: this._name,
      parameters: Beam.arrayToJson(this._parameters)
    };
  }

  private _name: string;
  private _parameters: ComplianceParameter[];
}
