import * as Beam from 'beam';
import { Money, Quantity, Ticker, Venue } from '../definitions';
import { QueryType } from './query_type';

/** Stores a quantity. */
export class QuantityValue extends Beam.NativeValue<Quantity> {

  /** Constructs a QuantityValue from a JSON object. */
  public static fromJson(value: any): QuantityValue {
    return new QuantityValue(Quantity.fromJson(value.value));
  }

  /**
   * Constructs a QuantityValue.
   * @param value - The value to store.
   */
  constructor(value: Quantity) {
    super(value);
  }

  public get type(): Beam.QueryType {
    return QueryType.QUANTITY;
  }

  protected get name(): string {
    return QUANTITY_NAME;
  }

  protected valueToJson(): any {
    return this.value.toJson();
  }
}

/** Stores a monetary value. */
export class MoneyValue extends Beam.NativeValue<Money> {

  /** Constructs a MoneyValue from a JSON object. */
  public static fromJson(value: any): MoneyValue {
    return new MoneyValue(Money.fromJson(value.value));
  }

  /**
   * Constructs a MoneyValue.
   * @param value - The value to store.
   */
  constructor(value: Money) {
    super(value);
  }

  public get type(): Beam.QueryType {
    return QueryType.MONEY;
  }

  protected get name(): string {
    return MONEY_NAME;
  }

  protected valueToJson(): any {
    return this.value.toJson();
  }
}

/** Stores a ticker symbol. */
export class TickerValue extends Beam.NativeValue<Ticker> {

  /** Constructs a TickerValue from a JSON object. */
  public static fromJson(value: any): TickerValue {
    return new TickerValue(Ticker.fromJson(value.value));
  }

  /**
   * Constructs a TickerValue.
   * @param value - The value to store.
   */
  constructor(value: Ticker) {
    super(value);
  }

  public get type(): Beam.QueryType {
    return QueryType.TICKER;
  }

  protected get name(): string {
    return TICKER_NAME;
  }

  protected valueToJson(): any {
    return this.value.toJson();
  }
}

/** Stores a venue. */
export class VenueValue extends Beam.NativeValue<Venue> {

  /** Constructs a VenueValue from a JSON object. */
  public static fromJson(value: any): VenueValue {
    return new VenueValue(Venue.fromJson(value.value));
  }

  /**
   * Constructs a VenueValue.
   * @param value - The value to store.
   */
  constructor(value: Venue) {
    super(value);
  }

  public get type(): Beam.QueryType {
    return QueryType.VENUE;
  }

  protected get name(): string {
    return VENUE_NAME;
  }

  protected valueToJson(): any {
    return this.value.toJson();
  }
}

const QUANTITY_NAME = 'Nexus.Queries.QuantityValue';
const MONEY_NAME = 'Nexus.Queries.MoneyValue';
const TICKER_NAME = 'Nexus.Queries.TickerValue';
const VENUE_NAME = 'Nexus.Queries.VenueValue';

Beam.Value.register(QUANTITY_NAME, QuantityValue.fromJson);
Beam.Value.register(MONEY_NAME, MoneyValue.fromJson);
Beam.Value.register(TICKER_NAME, TickerValue.fromJson);
Beam.Value.register(VENUE_NAME, VenueValue.fromJson);
