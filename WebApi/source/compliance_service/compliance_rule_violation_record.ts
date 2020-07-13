import * as Beam from 'beam';

/** Stores a single compliance rule violation. */
export class ComplianceRuleViolationRecord {

  /** Represents an invalid value. */
  public static readonly INVALID = new ComplianceRuleViolationRecord(
    Beam.DirectoryEntry.INVALID, 0, 0, '', '', Beam.DateTime.NOT_A_DATE_TIME);

  /** Parses a ComplianceRuleViolationRecord from JSON. */
  public static fromJson(value: any): ComplianceRuleViolationRecord {
    return new ComplianceRuleViolationRecord(
      Beam.DirectoryEntry.fromJson(value.account), value.order_id,
      value.rule_id, value.schema_name, value.reason,
      Beam.DateTime.fromJson(value.timestamp));
  }

  /**
   * Constructs a ComplianceRuleViolationRecord.
   * @param account - The account that produced the violation.
   * @param orderId - The OrderId the violation is attributed to.
   * @param ruleId - The id of the rule that was violated.
   * @param schemaName - The name of the schema that was violated.
   * @param reason - The reason for the violation.
   * @param timestamp - The time of the violation.
   */
  constructor(account: Beam.DirectoryEntry, orderId: number, ruleId: number,
      schemaName: string, reason: string, timestamp: Beam.DateTime) {
    this._account = account;
    this._orderId = orderId;
    this._ruleId = ruleId;
    this._schemaName = schemaName;
    this._reason = reason;
    this._timestamp = timestamp;
  }

  /** Returns the account that produced the violation. */
  public get account(): Beam.DirectoryEntry {
    return this._account;
  }

  /** Returns the Order id the violation is attributed to. */
  public get orderId(): number {
    return this._orderId;
  }

  /** Returns the id of the rule that was violated. */
  public get ruleId(): number {
    return this._ruleId;
  }

  /** Returns the name of the schema that was violated. */
  public get schemaName(): string {
    return this._schemaName;
  }

  /** Returns the reason for the violation. */
  public get reason(): string {
    return this._reason;
  }

  /** Returns the time of the violation. */
  public get timestamp(): Beam.DateTime {
    return this._timestamp;
  }

  /** Converts this object to JSON. */
  public toJson(): any {
    return {
      account: this._account.toJson(),
      order_id: this._orderId,
      rule_id: this._ruleId,
      schema_name: this._schemaName,
      reason: this._reason,
      timestamp: this._timestamp.toJson()
    };
  }

  private _account: Beam.DirectoryEntry;
  private _orderId: number;
  private _ruleId: number;
  private _schemaName: string;
  private _reason: string;
  private _timestamp: Beam.DateTime;
}
