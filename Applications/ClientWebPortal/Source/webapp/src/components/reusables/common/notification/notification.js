class Notification {
  constructor(type, title, description) {
    this.type = type;
    this.title = title;
    this.description = description;
    this.isClicked = false;
  }
}

export default Notification;