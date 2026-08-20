import * as Beam from 'beam';
import { QueryType } from './query_type';

/** Accesses the members of an account modification request. */
export class AccountModificationRequestAccessor {

  /**
   * Returns an accessor for one of a query's parameters.
   * @param index - The index of the parameter.
   */
  public static fromParameter(
      index: number): AccountModificationRequestAccessor {
    return new AccountModificationRequestAccessor(new Beam.ParameterExpression(
      index, QueryType.ACCOUNT_MODIFICATION_REQUEST));
  }

  /**
   * Constructs an AccountModificationRequestAccessor.
   * @param expression - The Expression whose members are accessed.
   */
  constructor(expression: Beam.Expression) {
    this._expression = expression;
  }

  /** Returns an accessor for the id member. */
  public get id(): Beam.MemberAccessExpression {
    return new Beam.MemberAccessExpression(
      'id', Beam.QueryType.INT, this._expression);
  }

  /** Returns an accessor for the type member. */
  public get type(): Beam.MemberAccessExpression {
    return new Beam.MemberAccessExpression(
      'type', Beam.QueryType.INT, this._expression);
  }

  /** Returns an accessor for the account member. */
  public get account(): Beam.MemberAccessExpression {
    return new Beam.MemberAccessExpression(
      'account', Beam.QueryType.INT, this._expression);
  }

  /** Returns an accessor for the submission account member. */
  public get submissionAccount(): Beam.MemberAccessExpression {
    return new Beam.MemberAccessExpression(
      'submission_account', Beam.QueryType.INT, this._expression);
  }

  /** Returns an accessor for the timestamp member. */
  public get timestamp(): Beam.MemberAccessExpression {
    return new Beam.MemberAccessExpression(
      'timestamp', Beam.QueryType.DATE_TIME, this._expression);
  }

  /** Returns an accessor for the effective date member. */
  public get effectiveDate(): Beam.MemberAccessExpression {
    return new Beam.MemberAccessExpression(
      'effective_date', Beam.QueryType.DATE_TIME, this._expression);
  }

  /** Returns an accessor for the status member. */
  public get status(): Beam.MemberAccessExpression {
    return new Beam.MemberAccessExpression(
      'status', Beam.QueryType.INT, this._expression);
  }

  /** Returns an accessor for the last update timestamp member. */
  public get lastUpdateTimestamp(): Beam.MemberAccessExpression {
    return new Beam.MemberAccessExpression(
      'last_update_timestamp', Beam.QueryType.DATE_TIME, this._expression);
  }

  private _expression: Beam.Expression;
}
