class MessageBody {
  constructor(contentType, message) {
    this.contentType = contentType;
    this.message = message;
  }

  toData() {
    return {
      content_type: this.contentType,
      message: this.message
    };
  }

  clone() {
    return new MessageBody(this.contentType, this.message);
  }
}

MessageBody.fromData = (data) => {
  return new MessageBody(data.content_type, data.message);
};

export default MessageBody;
