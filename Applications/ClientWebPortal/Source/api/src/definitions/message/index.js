import MessageBody from '../message-body';
import DirectoryEntry from '../directory-entry';

class Message {
  constructor(id, account, timestamp, bodies) {
    this.id = id;
    this.account = account;
    this.timestamp = timestamp;
    this.bodies = bodies;
  }

  toData() {
    return {
      id: this.id,
      account: this.account.toData(),
      timestamp: this.timestamp,
      bodies: this.bodies.map(body => {
        return body.toData();
      })
    };
  }

  clone() {
    return new Message(
      this.id,
      this.account.clone(),
      this.timestamp,
      this.bodies.map(body => {
        return body.clone();
      })
    );
  }
}

Message.fromData = (data) => {
  return new Message(
    data.id,
    DirectoryEntry.fromData(data.account),
    data.timestamp,
    data.bodies.map(body => {
      return MessageBody.fromData(body);
    })
  );
};

export default Message;
