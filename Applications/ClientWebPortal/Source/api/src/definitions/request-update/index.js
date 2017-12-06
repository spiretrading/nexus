import DirectoryEntry from '../directory-entry';

class RequestUpdate {
  constructor(status, account, sequenceNumber, timestamp) {
    this.status = status;
    this.account = account;
    this.sequenceNumber = sequenceNumber;
    this.timestamp = timestamp;
  }

  toData() {
    return {
      status: this.status,
      account: this.account.toData(),
      sequence_number: this.sequenceNumber,
      timestamp: this.timestamp
    };
  }

  clone() {
    return new RequestUpdate(
      this.status,
      this.account.clone(),
      this.sequenceNumber,
      this.timestamp
    );
  }
}

RequestUpdate.fromData = (data) => {
  return new RequestUpdate(
    data.status,
    DirectoryEntry.fromData(data.account),
    data.sequenceNumber,
    data.timestamp
  );
};

export default RequestUpdate;
