import { AccountModificationRequest } from './account_modification_request';
import { EntitlementModification } from './entitlement_modification';
import { RiskModification } from './risk_modification';

/** The modification requested by an AccountModificationRequest. */
export type Modification = EntitlementModification | RiskModification;

/** Stores the details needed to display an account modification request. */
export class AccountModificationRequestSummary {

  /** Constructs an AccountModificationRequestSummary from a JSON object. */
  public static fromJson(value: any): AccountModificationRequestSummary {
    return new AccountModificationRequestSummary(
      AccountModificationRequest.fromJson(value.request),
      AccountModificationRequest.Update.fromJson(value.status),
      value.comment_count, modificationFromJson(value.previous_state),
      modificationFromJson(value.modification));
  }

  /**
   * Constructs an AccountModificationRequestSummary.
   * @param request - The request being summarized.
   * @param status - The request's current status.
   * @param commentCount - The number of messages attached to the request.
   * @param previousState - The state prior to the request being applied.
   * @param modification - The state requested by the modification.
   */
  constructor(request: AccountModificationRequest,
      status: AccountModificationRequest.Update, commentCount: number,
      previousState: Modification, modification: Modification) {
    this._request = request;
    this._status = status;
    this._commentCount = commentCount;
    this._previousState = previousState;
    this._modification = modification;
  }

  /** Returns the request being summarized. */
  public get request(): AccountModificationRequest {
    return this._request;
  }

  /** Returns the request's current status. */
  public get status(): AccountModificationRequest.Update {
    return this._status;
  }

  /** Returns the number of messages attached to the request. */
  public get commentCount(): number {
    return this._commentCount;
  }

  /** Returns the state prior to the request being applied. */
  public get previousState(): Modification {
    return this._previousState;
  }

  /** Returns the state requested by the modification. */
  public get modification(): Modification {
    return this._modification;
  }

  /** Converts this object to JSON. */
  public toJson(): any {
    return {
      request: this._request.toJson(),
      status: this._status.toJson(),
      comment_count: this._commentCount,
      previous_state: modificationToJson(this._previousState),
      modification: modificationToJson(this._modification)
    };
  }

  private _request: AccountModificationRequest;
  private _status: AccountModificationRequest.Update;
  private _commentCount: number;
  private _previousState: Modification;
  private _modification: Modification;
}

function modificationFromJson(value: any): Modification {
  if(!value || !value.is_initialized) {
    return null;
  }
  if(value.value.which === 0) {
    return EntitlementModification.fromJson(value.value.value);
  }
  return RiskModification.fromJson(value.value.value);
}

function modificationToJson(value: Modification): any {
  if(!value) {
    return {is_initialized: false};
  }
  const which = (() => {
    if(value instanceof EntitlementModification) {
      return 0;
    }
    return 1;
  })();
  return {
    is_initialized: true,
    value: {
      which: which,
      value: value.toJson()
    }
  };
}
