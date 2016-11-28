import HashMap from 'hashmap';

class CountryDatabase {
  constructor() {
    this.codeEntries = new HashMap();
    this.nameEntries = new HashMap();
    this.twoLetterEntries = new HashMap();
    this.threeLetterEntries = new HashMap();
  }

  add(entry) {
    this.codeEntries.set(entry.code, entry);
    this.nameEntries.set(entry.name, entry);
    this.twoLetterEntries.set(entry.twoLetterCode, entry);
    this.threeLetterEntries.set(entry.threeLetterCode, entry);
  }

  entries() {
    return this.codeEntries.values();
  }

  fromCode(code) {
    return this.codeEntries.get(code);
  }

  fromName(name) {
    return this.codeEntries.get(name);
  }

  fromTwoLetterCode(code) {
    return this.twoLetterCode.get(code);
  }

  fromThreeLetterCode(code) {
    return this.threeLetterCode.get(code);
  }

  delete(entry) {
    this.codeEntries.remove(entry.code);
    this.nameEntries.remove(entry.name);
    this.twoLetterEntries.remove(entry.twoLetterCode);
    this.threeLetterEntries.remove(entry.threeLetterCode);
  }
}

export default CountryDatabase;
