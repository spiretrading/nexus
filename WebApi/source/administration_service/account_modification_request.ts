import * as Beam from 'beam';

/** Stores a request to modify an account. */
export class AccountModificationRequest {

  /** Constructs an AccountModificationRequest from a JSON object. */
  public static fromJson(value: any): AccountModificationRequest {
    return new AccountModificationRequest(value.id, value.type,
      Beam.DirectoryEntry.fromJson(value.account),
      Beam.DirectoryEntry.fromJson(value.submission_account),
      Beam.DateTime.fromJson(value.timestamp));
  }

  /**
   * Constructs an AccountModificationRequest.
   * @param id - The id that uniquely identifies this request.
   * @param type - The type of modification requested.
   * @param account - The account to modify.
   * @param submissionAccount - The account that submitted the request.
   * @param timestamp - The timestamp when the request was received.
   */
  constructor(id: number, type: AccountModificationRequest.Type,
      account: Beam.DirectoryEntry, submissionAccount: Beam.DirectoryEntry,
      timestamp: Beam.DateTime) {
    this._id = id;
    this._type = type;
    this._account = account;
    this._submissionAccount = submissionAccount;
    this._timestamp = timestamp;
  }

  /** Returns the id that uniquely identifies this request. */
  public get id(): number {
    return this._id;
  }

  /** Returns the type of modification requested. */
  public get type(): AccountModificationRequest.Type {
    return this._type;
  }

  /** Returns the account to modify. */
  public get account(): Beam.DirectoryEntry {
    return this._account;
  }

  /** Returns the account that submitted the request. */
  public get submissionAccount(): Beam.DirectoryEntry {
    return this._submissionAccount;
  }

  /** Returns the timestamp when the request was received. */
  public get timestamp(): Beam.DateTime {
    return this._timestamp;
  }

  /** Converts this object to JSON. */
  public toJson(): any {
    return {
      id: this._id,
      type: this._type,
      account: this._account.toJson(),
      submission_account: this._submissionAccount.toJson(),
      timestamp: this._timestamp.toJson()
    };
  }

  private _id: number;
  private _type: AccountModificationRequest.Type;
  private _account: Beam.DirectoryEntry;
  private _submissionAccount: Beam.DirectoryEntry;
  private _timestamp: Beam.DateTime;
}

export module AccountModificationRequest {

  /** Lists the modifications that can be made to an account. */
  export enum Type {

    /** Modify an account's market data entitlements. */
    ENTITLEMENTS = 0,

    /** Modify an account's risk parameters. */
    RISK
  }

  /** Lists the status of a request. */
  export enum Status {

    /** No or invalid status. */
    NONE,

    /** The request is pending. */
    PENDING,

    /** The request has been reviewed by a manager. */
    REVIEWED,

    /** The request has been scheduled. */
    SCHEDULED,

    /** The request has been granted. */
    GRANTED,

    /** The request has been rejected. */
    REJECTED
  }

  /** Stores a request status update. */
  export class Update {

    /** Constructs an Update from a JSON object. */
    public static fromJson(value: any): Update {
      return new Update(value.status,
        Beam.DirectoryEntry.fromJson(value.account), value.sequence_number,
        Beam.DateTime.fromJson(value.timestamp));
    }

    /**
     * Constructs an Update.
     * @param status - The updated status.
     * @param account - The account that updated the status.
     * @param sequenceNumber - The update sequence number.
     * @param timestamp - The timestamp when the update occurred.
     */
    constructor(status: Status, account: Beam.DirectoryEntry,
        sequenceNumber: number, timestamp: Beam.DateTime) {
      this._status = status;
      this._account = account;
      this._sequenceNumber = sequenceNumber;
      this._timestamp = timestamp;
    }

    /** Returns the updated status. */
    public get status(): Status {
      return this._status;
    }

    /** Returns the account that updated the status. */
    public get account(): Beam.DirectoryEntry {
      return this._account;
    }

    /** Returns the update's sequence number. */
    public get sequenceNumber(): number {
      return this._sequenceNumber;
    }

    /** Returns the timestamp when the update occurred. */
    public get timestamp(): Beam.DateTime {
      return this._timestamp;
    }

    /** Converts this object to JSON. */
    public toJson(): any {
      return {
        status: this._status,
        account: this._account.toJson(),
        sequence_number: this._sequenceNumber,
        timestamp: this._timestamp.toJson()
      };
    }

    private _status: Status;
    private _account: Beam.DirectoryEntry;
    private _sequenceNumber: number;
    private _timestamp: Beam.DateTime;
  }
}
