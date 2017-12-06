import DirectoryEntry from '../directory-entry';

class AccountModificationRequest {
  constructor(id, requestType, account, submissionAccount, timestamp) {
    this.id = id;
    this.requestType = requestType;
    this.account = account;
    this.submissionAccount = submissionAccount;
    this.timestamp = timestamp;
  }

  toData() {
    return {
      id: this.id,
      type: this.requestType,
      account: this.account.toData(),
      submission_account: this.submissionAccount.toData(),
      timestamp: this.timestamp
    };
  }

  clone() {
    return new AccountModificationRequest(
      this.id,
      this.requestType,
      this.account.clone(),
      this.submissionAccount.clone(),
      this.timestamp
    );
  }
}

AccountModificationRequest.fromData = (data) => {
  return new AccountModificationRequest(
    data.id,
    data.type,
    DirectoryEntry.fromData(data.account),
    DirectoryEntry.fromData(data.submission_account),
    data.timestamp
  );
};

export default EntitlementModification;
