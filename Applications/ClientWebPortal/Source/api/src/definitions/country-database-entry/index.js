class CountryDatabaseEntry {
  constructor(code, name, twoLetterCode, threeLetterCode) {
    this.code = code;
    this.name = name;
    this.twoLetterCode = twoLetterCode;
    this.threeLetterCode = threeLetterCode;
  }

  clone() {
    return new CountryDatabaseEntry(
      this.code.clone(),
      this.name,
      this.twoLetterCode,
      this.threeLetterCode
    );
  }
}

export default CountryDatabaseEntry;
