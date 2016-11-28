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

  clone() {
    new CountryDatabaseEntry(
      this.marketCode.clone(),
      this.countryCode.clone(),
      this.timeZone,
      this.currencyId.clone(),
      this.boardLot,
      this.description,
      this.displayName
    );
  }
}

export default CountryDatabaseEntry;
