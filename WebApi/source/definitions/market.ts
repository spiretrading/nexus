import * as Beam from 'beam';

/** Stores a market's unique identifer. */
export class MarketCode {

  /** Parses a MarketCode from JSON. */
  public static fromJson(value: any): MarketCode {
    return new MarketCode(value);
  }

  /** Constructs a MarketCode from its identifier. */
  constructor(identifier: string) {
    this.identifier = identifier;
  }

  /** Tests if two MarketCodes represent the same market. */
  public equals(other: MarketCode): boolean {
    return other && this.identifier === other.identifier;
  }

  /** Returns the market's identifier. */
  public toString(): string {
    return this.identifier;
  }

  /** Converts this object to JSON. */
  public toJson(): any {
    return this.identifier;
  }

  /** Returns a hash of this value. */
  public hash(): number {
    return Beam.hashString(this.identifier);
  }

  private identifier: string;
}
