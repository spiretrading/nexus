class DirectoryEntry {
  static get ROOT() {
    return new DirectoryEntry(1, 0, 'root');
  }

  static get STAR() {
    return new DirectoryEntry(0, 1, '*');
  }

  constructor(id, type, name) {
    this.id = id;
    this.type = type;
    this.name = name;
  }

  compare(operand) {
    return this.id - operand.id;
  }

  equals(operand) {
    return this.id === operand.id;
  }

  toData() {
    return {
      id: this.id,
      type: this.type,
      name: this.name
    };
  }
}

DirectoryEntry.fromData = (data) => {
  let directoryEntry = new DirectoryEntry();
  directoryEntry.id = data.id;
  directoryEntry.type = data.type;
  directoryEntry.name = data.name;
  return directoryEntry;
};

export default DirectoryEntry;
