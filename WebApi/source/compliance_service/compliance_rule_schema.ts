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

  /** Returns the title case of a ComplianceRuleSchema. */
  public static toTitleCase(name: string) {
    const split = name.replace(/_/g, ' ').split(' ');
    for(let i = 0; i < split.length; ++i) {
      split[i] = split[i].charAt(0).toUpperCase() + split[i].substring(1);
    }
    return split.join(' '); 
  }

  /**
   * Constructs a ComplianceRuleSchema.
   * @param name The name of the rule.
   * @param parameters The list of parameters.
   */
  constructor(name: string, parameters: ComplianceParameter[]) {
    this._rawName = name;
    this._rawParameters = parameters.slice();
    Object.freeze(this._rawParameters);
    this._applicability = (() => {
      if(this._rawName === ComplianceRuleSchema.PER_ACCOUNT_NAME) {
        return ComplianceRuleSchema.Applicability.PER_ACCOUNT;
      }
      return ComplianceRuleSchema.Applicability.CONSOLIDATED;
    })();
    if(this._applicability === ComplianceRuleSchema.Applicability.PER_ACCOUNT) {
      this._parameters = [];
      for(const parameter of this._rawParameters) {
        if(parameter.name === 'name') {
          this._name = parameter.value.value as string;
        } else if(parameter.name.startsWith('\\')) {
          this._parameters.push(new ComplianceParameter(
            parameter.name.substring(1), parameter.value));
        }
      }
    } else {
      this._name = this._rawName;
      this._parameters = this._rawParameters;
    }
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
    const parameters = [];
    parameters.push(name);
    for(const parameter of this._rawParameters) {
      parameters.push(
        new ComplianceParameter('\\' + parameter.name, parameter.value));
    }
    return new ComplianceRuleSchema(
      ComplianceRuleSchema.PER_ACCOUNT_NAME, parameters);
  }

  /** Converts this object to JSON. */
  public toJson(): any {
    return {
      name: this._rawName,
      parameters: Beam.arrayToJson(this._rawParameters)
    };
  }

  private _rawName: string;
  private _rawParameters: ComplianceParameter[];
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

/**
 * Determines if a ComplianceRuleSchema is a wrapped schema.
 * @param schema The ComplianceRuleSchema to check.
 * @return True iff the schema is wrapped.
 */
export function isWrapped(schema: ComplianceRuleSchema): boolean {
  if(schema.parameters.length < 2) {
    return false;
  }
  const name = schema.parameters[schema.parameters.length - 2];
  const args = schema.parameters[schema.parameters.length - 1];
  return name.name === 'name' &&
    name.value.type === ComplianceValue.Type.STRING &&
    args.name === 'arguments' && args.value.type === ComplianceValue.Type.LIST;
}

/**
 * Gets the fully unwrapped name of a ComplianceRuleSchema.
 * @param schema The ComplianceRuleSchema to get the name from.
 * @return The unwrapped name of the schema.
 */
export function getUnwrappedName(schema: ComplianceRuleSchema): string {
  if(isWrapped(schema)) {
    return getUnwrappedName(unwrap(schema));
  }
  return schema.name;
}

/**
 * Wraps a ComplianceRuleSchema for use by a higher-order
 * ComplianceRuleSchema.
 * @param name The name of the higher order ComplianceRuleSchema.
 * @param parametersOrSchema The parameters used by the higher-order
 *        ComplianceRuleSchema, or the schema to wrap if only 2 arguments.
 * @param schema The ComplianceRuleSchema to wrap (optional if 2 arguments).
 * @return A higher-order ComplianceRuleSchema.
 */
export function wrap(name: string,
    parametersOrSchema: ComplianceParameter[] | ComplianceRuleSchema,
    schema?: ComplianceRuleSchema): ComplianceRuleSchema {
  if(parametersOrSchema instanceof ComplianceRuleSchema && !schema) {
    return wrap(name, [], parametersOrSchema);
  }
  const parameters = parametersOrSchema as ComplianceParameter[];
  const wrappedSchema = schema as ComplianceRuleSchema;
  const newParameters = parameters.slice();
  newParameters.push(new ComplianceParameter('name',
    new ComplianceValue(ComplianceValue.Type.STRING, wrappedSchema.name)));
  const args: ComplianceValue[] = [];
  for(const parameter of wrappedSchema.parameters) {
    args.push(new ComplianceValue(ComplianceValue.Type.LIST, [
      new ComplianceValue(ComplianceValue.Type.STRING, parameter.name),
      parameter.value]));
  }
  newParameters.push(new ComplianceParameter(
    'arguments', new ComplianceValue(ComplianceValue.Type.LIST, args)));
  return new ComplianceRuleSchema(name, newParameters);
}

/**
 * Unwraps a higher-order ComplianceRuleSchema, returning the wrapped
 * schema.
 * @param schema The higher-order ComplianceRuleSchema to unwrap.
 * @return The ComplianceRuleSchema that was wrapped.
 */
export function unwrap(schema: ComplianceRuleSchema): ComplianceRuleSchema {
  let name = '';
  let args: ComplianceValue[] = [];
  for(const parameter of schema.parameters) {
    if(parameter.name === 'name') {
      name = parameter.value.value as string;
    } else if(parameter.name === 'arguments') {
      args = parameter.value.value as ComplianceValue[];
    }
  }
  const parameters: ComplianceParameter[] = [];
  for(const argument of args) {
    const parameter = argument.value as ComplianceValue[];
    if(parameter.length !== 2) {
      throw new Error('Invalid ComplianceParameter specified.');
    }
    parameters.push(
      new ComplianceParameter(parameter[0].value as string, parameter[1]));
  }
  return new ComplianceRuleSchema(name, parameters);
}
