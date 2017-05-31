import HashMap from 'hashmap';

class MarketDatabase {
  constructor() {
    this.marketCodeEntries = new HashMap();
    this.countryCodeEntries = new HashMap();
    this.displayNameEntries = new HashMap();
  }

  add(entry) {
    this.marketCodeEntries.set(entry.marketCode.toCode(), entry);

    if (this.countryCodeEntries.has(entry.countryCode.toNumber())) {
      let countryMarketCodeEntries = this.countryCodeEntries.get(entry.countryCode.toNumber());
      countryMarketCodeEntries.set(entry.marketCode.toCode(), entry);
    } else {
      let countryMarketCodeEntries = new HashMap();
      countryMarketCodeEntries.set(entry.marketCode.toCode(), entry);
      this.countryCodeEntries.set(entry.countryCode.toNumber(), countryMarketCodeEntries);
    }

    this.displayNameEntries.set(entry.displayName, entry);
  }

  entries() {
    return this.marketCodeEntries.values();
  }

  fromMarketCode(marketCode) {
    return this.marketCodeEntries.get(marketCode);
  }

  fromCountryCode(countryCode) {
    return this.countryCodeEntries.get(countryCode).values();
  }

  fromDisplayName(name) {
    return this.displayNameEntries.get(name);
  }

  delete(entry) {
    this.marketCodeEntries.remove(entry.marketCode);

    if (this.countryCodeEntries.has(entry.countryCode)) {
      let countryMarketCodeEntries = this.countryCodeEntries.get(entry.countryCode);
      countryMarketCodeEntries.remove(entry.marketCode);
      if (countryMarketCodeEntries.count() === 0) {
        this.countryCodeEntries.remove(entry.countryCode);
      }
    }

    this.displayNameEntries.remove(entry.displayName);
  }

  parseMarketCode(source) {
    if (this.displayNameEntries.has(source)) {
      return this.displayNameEntries.get(source).marketCode;
    } else if (this.marketCodeEntries.has(source)) {
      return this.marketCodeEntries.get(source).marketCode;
    } else {
      return null;
    }
  }

  parseMarketEntry(source) {
    if (this.displayNameEntries.has(source)) {
      return this.displayNameEntries.get(source);
    } else if (this.marketCodeEntries.has(source)) {
      return this.marketCodeEntries.get(source);
    } else {
      return null;
    }
  }
}

export default MarketDatabase;
