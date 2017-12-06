import DirectoryEntry from '../directory-entry';

class EntitlementModification {
  constructor(directoryEntries) {
    this.entitlements = directoryEntries;
  }

  toData() {
    return this.entitlements.map((value) => {
      return value.toData();
    });
  }

  toString() {
    return this.entitlements.map((value) => {
      return value.toString();
    });
  }

  clone() {
    return new EntitlementModification(this.entitlements.map((value) => {
      return value.clone();
    }));
  }
}

EntitlementModification.fromData = (data) => {
  return new EntitlementModification(data.map(directoryEntry => {
    return DirectoryEntry.fromData(directoryEntry);
  }));
};

export default EntitlementModification;
