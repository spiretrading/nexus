import HashMap from 'hashmap';

class CurrencyDatabase {
  constructor() {
    this.idEntries = new HashMap();
    this.codeEntries = new HashMap();
  }

  add(entry) {
    this.idEntries.set(entry.id, entry);
    this.codeEntries.set(entry.code, entry);
  }

  entries() {
    return this.idEntries.values();
  }

  fromId(id) {
    return this.idEntries.get(id);
  }

  fromCode(code) {
    return this.codeEntries.get(code);
  }

  delete(entry) {
    this.idEntries.remove(entry.id);
    this.codeEntries.remove(entry.code);
  }
}

export default CurrencyDatabase;
