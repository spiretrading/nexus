import DirectoryEntry from '../directory-entry';

class EntitlementModification {
  constructor(directoryEntries) {
    this.entitlements = directoryEntries;
  }

  toData() {
    return {
      entitlements: this.entitlements.map((value) => {
        return value.toData();
      })
    };
  }

  clone() {
    return new EntitlementModification(this.entitlements.map((value) => {
      return value.clone();
    }));
  }
}

EntitlementModification.fromData = (data) => {
  return new EntitlementModification(data.entitlements.map(directoryEntry => {
    return DirectoryEntry.fromData(directoryEntry);
  }));
};

export default EntitlementModification;
