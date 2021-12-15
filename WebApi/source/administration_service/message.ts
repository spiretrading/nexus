import * as Beam from 'beam';

/** Stores the details of a single message. */
export class Message {

  /** MIME-type for a plain text message. */
  public static readonly PLAIN_TEXT = 'text/plain';

  /** Constructs a Message from a JSON object. */
  public static fromJson(value: any): Message {
    return new Message(value.id, Beam.DirectoryEntry.fromJson(value.account),
      Beam.DateTime.fromJson(value.timestamp),
      Beam.arrayFromJson(Message.Body, value.bodies));
  }

  /**
   * Constructs a plain text message from a string.
   * @param message: The plain text message.
   */
  public static fromPlainText(message: string): Message {
    return new Message(-1, Beam.DirectoryEntry.INVALID,
      Beam.DateTime.NOT_A_DATE_TIME,
      [Message.Body.fromPlainText(message)]);
  }

  /**
   * Constructs a Message.
   * @param id - The message's unique id.
   * @param account - The account that sent the message.
   * @param timestamp - The timestamp when the message was received.
   * @param bodies - The list of message bodies.
   */
  constructor(id?: number, account?: Beam.DirectoryEntry,
      timestamp?: Beam.DateTime, bodies?: Array<Message.Body>) {
    if(id === undefined) {
      this._id = -1;
      this._account = Beam.DirectoryEntry.INVALID;
      this._timestamp = Beam.DateTime.NOT_A_DATE_TIME;
      this._bodies = [Message.Body.EMPTY];
    } else {
      this._id = id;
      this._account = account;
      this._timestamp = timestamp;
      if(bodies.length === 0) {
        bodies = new Array<Message.Body>();
        bodies.push(Message.Body.EMPTY);
      } else {
        this._bodies = bodies.slice();
      }
    }
    Object.freeze(this._bodies);
  }

  /** Returns the message's unique id. */
  public get id(): number {
    return this._id;
  }

  /** Returns the account that sent the message. */
  public get account(): Beam.DirectoryEntry {
    return this._account;
  }

  /** Returns the timestamp when the message was received. */
  public get timestamp(): Beam.DateTime {
    return this._timestamp;
  }

  /** Returns the list of message bodies. */
  public get bodies(): Array<Message.Body> {
    return this._bodies;
  }

  /** Converts this object to JSON. */
  public toJson(): any {
    return {
      id: this._id,
      account: this._account.toJson(),
      timestamp: this._timestamp.toJson(),
      bodies: Beam.arrayToJson(this._bodies)
    };
  }

  private _id: number;
  private _account: Beam.DirectoryEntry;
  private _timestamp: Beam.DateTime;
  private _bodies: Array<Message.Body>;
}

export module Message {

  /** Stores the body of a message. */
  export class Body {

    /** Represents an empty body. */
    public static readonly EMPTY = new Body(Message.PLAIN_TEXT, '');

    /** Constructs a Body from a JSON object. */
    public static fromJson(value: any): Body {
      return new Body(value.content_type, value.message);
    }

    /**
     * Constructs a plain text body.
     * @param message - The contents of the message.
     */
    public static fromPlainText(message: string): Body {
      return new Body(Message.PLAIN_TEXT, message);
    }

    /**
     * Constructs a message body.
     * @param contentType - The MIME-type of the message.
     * @param message - The contents of the message.
     */
    constructor(contentType: string = Message.PLAIN_TEXT,
        message: string = '') {
      this._contentType = contentType;
      this._message = message;
    }

    /** Returns the message MIME-type. */
    public get contentType(): string {
      return this._contentType;
    }

    /** Returns the contents of the message. */
    public get message(): string {
      return this._message;
    }

    /** Converts this object to JSON. */
    public toJson(): any {
      return {
        content_type: this._contentType,
        message: this._message
      };
    }

    private _contentType: string;
    private _message: string;
  }
}
