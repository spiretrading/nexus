class MessageBody {
  constructor(contentType, body) {
    this.contentType = contentType;
    this.body = body;
  }

  toData() {
    return {
      content_type: this.contentType,
      body: this.body
    };
  }

  clone() {
    return new MessageBody(this.contentType, this.body);
  }
}

MessageBody.fromData = (data) => {
  return new MessageBody(data.content_type, data.body);
};

export default MessageBody;
