import * as Beam from 'beam';
import { ComplianceRuleSchema } from './compliance_rule_schema';

/** Represents a single instance of a compliance rule. */
export class ComplianceRuleEntry {

  /** Parses a ComplianceRuleEntry from JSON. */
  public static fromJson(value: any): ComplianceRuleEntry {
    return new ComplianceRuleEntry(value.id,
      Beam.DirectoryEntry.fromJson(value.directory_entry),
      value.state as ComplianceRuleEntry.State,
      ComplianceRuleSchema.fromJson(value.schema));
  }

  /**
   * Constructs a ComplianceRuleEntry.
   * @param name The name of the rule.
   * @param parameters The list of parameters.
   */
  constructor(id: number, directoryEntry: Beam.DirectoryEntry,
      state: ComplianceRuleEntry.State, schema: ComplianceRuleSchema) {
    this._id = id;
    this._directoryEntry = directoryEntry;
    this._state = state;
    this._schema = schema;
  }

  /** Returns the id. */
  public get id(): number {
    return this._id;
  }

  /** Returns the directory entry this rule applies to. */
  public get directoryEntry(): Beam.DirectoryEntry {
    return this._directoryEntry;
  }

  /** Returns the current state of this rule. */
  public get state(): ComplianceRuleEntry.State {
    return this._state;
  }

  /** Returns the rule's schema. */
  public get schema(): ComplianceRuleSchema {
    return this._schema;
  }

  /** Converts this object to JSON. */
  public toJson(): any {
    return {
      id: this._id,
      directory_entry: this._directoryEntry.toJson(),
      state: this._state,
      schema: this._schema.toJson()
    };
  }

  private _id: number;
  private _directoryEntry: Beam.DirectoryEntry;
  private _state: ComplianceRuleEntry.State;
  private _schema: ComplianceRuleSchema;
}

export module ComplianceRuleEntry {

  /** Enumerates the states of a ComplianceRuleEntry. */
  export enum State {

    /** An invalid value. */
    NONE = -1,

    /** The rule will reject operations that fail the compliance check. */
    ACTIVE = 0,

    /** The rule will log operations that fail the compliance check. */
    PASSIVE = 1,

    /** The rule performs no validation. */
    DISABLED = 2,

    /** The rule has been deleted. */
    DELETED = 3,
  }
}
