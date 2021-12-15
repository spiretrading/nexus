import * as Beam from 'beam';
import { ComplianceValue } from '.';
import { ComplianceParameter } from './compliance_parameter';

/** Provides a description for a single compliance rule. */
export class ComplianceRuleSchema {

  /** The raw name used to indicate a schema applied per account. */
  public static readonly PER_ACCOUNT_NAME = 'per_account';

  /** Parses a ComplianceRuleSchema from JSON. */
  public static fromJson(value: any): ComplianceRuleSchema {
    return new ComplianceRuleSchema(value.name,
      Beam.arrayFromJson(ComplianceParameter, value.parameters));
  }

  /**
   * Constructs a ComplianceRuleSchema.
   * @param name The name of the rule.
   * @param parameters The list of parameters.
   */
  constructor(name: string, parameters: ComplianceParameter[]) {
    this._rawName = name;
    this._parameters = parameters.slice();
    Object.freeze(this._parameters);
    this._applicability = (() => {
      if(this._rawName === ComplianceRuleSchema.PER_ACCOUNT_NAME) {
        return ComplianceRuleSchema.Applicability.PER_ACCOUNT;
      }
      return ComplianceRuleSchema.Applicability.CONSOLIDATED;
    })();
    if(this._applicability === ComplianceRuleSchema.Applicability.PER_ACCOUNT) {
      for(const parameter of this._parameters) {
        if(parameter.name === 'name') {
          this._name = parameter.value.value as string;
          break;
        }
      }
    } else {
      this._name = this._rawName;
    }
  }

  /** Returns the name of the rule. */
  public get name(): string {
    return this._name;
  }

  /** Returns the rule's parameters. */
  public get parameters(): ComplianceParameter[] {
    return this._parameters;
  }

  /** Returns the rule's applicability. */
  public get applicability(): ComplianceRuleSchema.Applicability {
    return this._applicability;
  }

  /**
   * Returns a copy of this schema with a specified applicability.
   * @param applicability The applicability that the schema should be set to.
   * @return A clone of this schema with the specified applicability.  If
   *         the applicability is the same as the current applicability then
   *         this is a no-op.
   */
  public toApplicability(
      applicability: ComplianceRuleSchema.Applicability): ComplianceRuleSchema {
    if(applicability === this._applicability) {
      return this;
    } else if(
        applicability === ComplianceRuleSchema.Applicability.CONSOLIDATED) {
      const parameters = this._parameters.slice();
      const nameIndex =
        parameters.findIndex(parameter => parameter.name === 'name');
      if(nameIndex !== -1) {
        parameters.splice(nameIndex);
      }
      return new ComplianceRuleSchema(this.name, parameters);
    }
    const name = new ComplianceParameter('name',
      new ComplianceValue(ComplianceValue.Type.STRING, this.name));
    const parameters = this._parameters.slice();
    parameters.splice(0, 0, name);
    return new ComplianceRuleSchema(
      ComplianceRuleSchema.PER_ACCOUNT_NAME, parameters);
  }

  /** Converts this object to JSON. */
  public toJson(): any {
    return {
      name: this._rawName,
      parameters: Beam.arrayToJson(this._parameters)
    };
  }

  private _rawName: string;
  private _parameters: ComplianceParameter[];
  private _applicability: ComplianceRuleSchema.Applicability;
  private _name: string;
}

export namespace ComplianceRuleSchema {

  /** Determines how a rule is applied across multiple accounts. */
  export enum Applicability {

    /** The rule is applied to each account independently. */
    PER_ACCOUNT,

    /** The rule is applied by consolidating all accounts together. */
    CONSOLIDATED
  }
}
