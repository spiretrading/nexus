import * as Beam from 'beam';
import { CountryCode } from './country';
import { Ticker } from './ticker';
import { Venue } from './venue';

/** Represents a group of trading locations. */
export class Region {

  /** The global Region sentinel. */
  public static readonly GLOBAL: Region = Region.makeGlobal('');

  /** Makes a value from a JSON object. */
  public static fromJson(value: any): Region {
    const region = new Region();
    region._name = value.name;
    region._isGlobal = value.is_global;
    region._countries =
      Beam.Set.fromJson<CountryCode>(CountryCode, value.countries);
    region._venues = Beam.Set.fromJson<Venue>(Venue, value.venues);
    region._tickers = Beam.Set.fromJson<Ticker>(Ticker, value.tickers);
    return region;
  }

  /**
   * Constructs a global named Region.
   * @param name The name of the Region.
   */
  public static makeGlobal(name: string): Region {
    const region = new Region();
    region._isGlobal = true;
    region._name = name;
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
   * Constructs a Region consisting of a single Ticker.
   * @param ticker The Ticker to represent.
   */
  constructor(ticker: Ticker);

  constructor(arg?: string | CountryCode | Venue | Ticker) {
    this._name = '';
    this._isGlobal = false;
    this._countries = new Beam.Set<CountryCode>();
    this._venues = new Beam.Set<Venue>();
    this._tickers = new Beam.Set<Ticker>();
    if(typeof arg === 'string') {
      this._name = arg;
    } else if(isCountryCode(arg)) {
      this._countries.add(arg);
    } else if(isVenue(arg)) {
      this._venues.add(arg as Venue);
    } else if(isTicker(arg)) {
      this._tickers.add(arg as Ticker);
    }
  }

  /** Returns the name of this Region. */
  public get name(): string {
    return this._name;
  }

  /** Returns true iff this is the global Region. */
  public get isGlobal(): boolean {
    return this._isGlobal;
  }

  /** Returns the countries in this Region. */
  public get countries(): Beam.Set<CountryCode> {
    return this._countries.clone();
  }

  /** Returns the venues in this Region. */
  public get venues(): Beam.Set<Venue> {
    return this._venues.clone();
  }

  /** Returns the Tickers in this Region. */
  public get tickers(): Beam.Set<Ticker> {
    return this._tickers.clone();
  }

  /**
   * Combines this Region with another.
   * @param region The Region to combine.
   * @returns The combined Region (this).
   */
  public add(region: Region): Region {
    if(region.isGlobal) {
      this._isGlobal = true;
      this._countries = new Beam.Set<CountryCode>();
      this._venues = new Beam.Set<Venue>();
      this._tickers = new Beam.Set<Ticker>();
    } else if(!this.isGlobal) {
      for(const country of region._countries) {
        this._countries.add(country);
      }
      for(const venue of region._venues) {
        this._venues.add(venue);
      }
      for(const ticker of region._tickers) {
        this._tickers.add(ticker);
      }
    }
    return this;
  }

  /** Returns a clone of this object. */
  public clone(): Region {
    const region = new Region();
    region._name = this._name;
    region._isGlobal = this._isGlobal;
    region._countries = this._countries.clone();
    region._venues = this._venues.clone();
    region._tickers = this._tickers.clone();
    return region;
  }

  /** Converts this object to JSON. */
  public toJson(): any {
    return {
      name: this._name,
      is_global: this._isGlobal,
      countries: this._countries.toJson(),
      venues: this._venues.toJson(),
      tickers: this._tickers.toJson()
    };
  }

  /** Returns a hash of this value. */
  public hash(): number {
    if(this.isGlobal) {
      return 0x9e3779b9;
    }
    const COUNTRY_SALT = 0x1bd11bda;
    const VENUE_SALT = 0x3c79ac49;
    const TICKER_SALT = 0x1f123bb5;
    const mix = (x: number): number => {
      let y = (x + 0x9e3779b9) | 0;
      y = ((y ^ (y >>> 30)) * 0xbf58476d) | 0;
      y = ((y ^ (y >>> 27)) * 0x94d049bb) | 0;
      return (y ^ (y >>> 31)) | 0;
    };
    const rotate_left = (x: number, r: number): number => {
      return ((x << r) | (x >>> (32 - r))) | 0;
    };
    const set_hash = (set: Beam.Set<any>, salt: number): number => {
      let sum = 0;
      let xors = 0;
      for(const element of set) {
        const hv = mix((element.hash() + salt) | 0);
        sum = (sum + hv) | 0;
        xors ^= rotate_left(hv, 23);
      }
      return mix((sum + rotate_left(xors, 17)) | 0);
    };
    const countries_hash = set_hash(this._countries, COUNTRY_SALT);
    const venues_hash = set_hash(this._venues, VENUE_SALT);
    const tickers_hash = set_hash(this._tickers, TICKER_SALT);
    const hash = (countries_hash + rotate_left(venues_hash, 21) +
      rotate_left(tickers_hash, 42)) | 0;
    return mix(hash);
  }

  /** Returns the string representation */
  public toString(): string {
    if(this.name.length != 0) {
      return this.name;
    }
    return '';
  }

  private _name: string;
  private _isGlobal: boolean;
  private _countries: Beam.Set<CountryCode>;
  private _venues: Beam.Set<Venue>;
  private _tickers: Beam.Set<Ticker>;
}

function isCountryCode(x: any): x is CountryCode {
  return typeof x?.code === 'number';
}

function isTicker(x: any): x is Ticker {
  return typeof x?.symbol === 'string';
}

function isVenue(x: any): x is Venue {
  return !isTicker(x) && typeof x?.toString === 'string';
}
