class CountryDatabaseEntry {
  constructor(marketCode, countryCode, timeZone, currencyId, boardLot, description, displayName) {
    this.marketCode = marketCode;
    this.countryCode = countryCode;
    this.timeZone = timeZone;
    this.currencyId = currencyId;
    this.boardLot = boardLot;
    this.description = description;
    this.displayName = displayName;
  }
}

export default CountryDatabaseEntry;
