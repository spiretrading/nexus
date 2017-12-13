class CurrencyDatabaseEntry {
  constructor(id, code, sign) {
    this.id = id;
    this.code = code;
    this.sign = sign;
  }

  clone() {
    return new CountryDatabaseEntry(
      this.id.clone(),
      this.code,
      this.sign
    );
  }
}

export default CurrencyDatabaseEntry;
