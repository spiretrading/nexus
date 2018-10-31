/** Identifies a country by its ISO 3166-1 numeric code. */
export class CountryCode {

  /** Represents no country. */
  public static readonly NONE = new CountryCode(~0);

  /** Parses a CountryCode from JSON. */
  public static fromJson(value: any): CountryCode {
    return new CountryCode(value);
  }

  /** Constructs a CountryCode from its ISO 3166-1 numeric code. */
  constructor(code: number) {
    this._code = code;
  }

  /** Returns the ISO 3166-1 numeric code. */
  public get code(): number {
    return this._code;
  }

  /** Tests if two CountryCodes represent the same country. */
  public equals(other: CountryCode): boolean {
    return other && this._code === other._code;
  }

  /** Converts this object to JSON. */
  public toJson(): any {
    return this._code;
  }

  /** Returns a hash of this value. */
  public hash(): number {
    return this._code;
  }

  private _code: number;
}
