import { CountryCode } from './country';
import { Security } from './security';
import { Venue } from './venue';

/** Represents a group of trading locations. */
export class Region {

  /** The global Region sentinel. */
  public static readonly GLOBAL: Region = Region.makeGlobal('');

  /** Makes a value from a JSON object. */
  public static fromJson(value: any): Region {
    return null;
  }

  /**
   * Constructs a global named Region.
   * @param name The name of the Region.
   */
  public static makeGlobal(name: string): Region {
    const region = new Region();
    region.m_isGlobal = true;
    region.m_name = name;
    return region;
  }

  /** Constructs an empty Region. */
  constructor();

  /**
   * Constructs an empty named Region.
   * @param name The name of the Region.
   */
  constructor(name: string);

  /**
   * Constructs a Region consisting of a single country.
   * @param country The country to represent.
   */
  constructor(country: CountryCode);

  /**
   * Constructs a Region consisting of a single venue.
   * @param venue The venue to represent.
   */
  constructor(venue: Venue);

  /**
   * Constructs a Region consisting of a single Security.
   * @param security The Security to represent.
   */
  constructor(security: Security);

  constructor(arg?: string | CountryCode | Venue | Security) {}

  /** Returns the name of this Region. */
  public get name(): string {
    return null as any;
  }

  /** Returns true iff this is the global Region. */
  public get isGlobal(): boolean {
    return null as any;
  }

  /** Returns true iff this Region is empty. */
  public get isEmpty(): boolean {
    return null as any;
  }

  /** Returns the countries in this Region. */
  public get countries(): Set<CountryCode> {
    return null as any;
  }

  /** Returns the venues in this Region. */
  public get venues(): Set<Venue> {
    return null as any;
  }

  /** Returns the Securities in this Region. */
  public get securities(): Set<Security> {
    return null as any;
  }

  /**
   * Returns true iff `region` is a subset of this.
   * @param region The Region to test.
   */
  public contains(region: Region): boolean {
    return null as any;
  }

  /**
   * Combines this Region with another.
   * @param region The Region to combine.
   * @returns The combined Region (this).
   */
  public add(region: Region): Region {
    return null as any;
  }

  /** Returns true iff this Region is a strict subset of another. */
  public lessThan(region: Region): boolean {
    return null as any;
  }

  /** Returns true iff this Region is a subset of another. */
  public lessThanOrEqual(region: Region): boolean {
    return null as any;
  }

  /** Returns true iff this Region is equal to another. */
  public equals(region: Region): boolean {
    return null as any;
  }

  /** Returns true iff this Region is not equal to another. */
  public notEquals(region: Region): boolean {
    return null as any;
  }

  /** Returns true iff this Region is a superset of another. */
  public greaterThanOrEqual(region: Region): boolean {
    return null as any;
  }

  /** Returns true iff this Region is a strict superset of another. */
  public greaterThan(region: Region): boolean {
    return null as any;
  }

  /** Converts this object to JSON. */
  public toJson(): any {
    return null;
  }

  /** Returns a hash of this value. */
  public hash(): number {
    return null;
  }

  /** Returns the string representation */
  public toString(): string {
    return null as any;
  }

  private m_name: string;
  private m_isGlobal: boolean;
  private m_countries: Set<CountryCode>;
  private m_venues: Set<Venue>;
  private m_securities: Set<Security>;
}
