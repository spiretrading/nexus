import NotificationType from './notification-type';
import Notification from './notification';

class NotificationRepo {
  constructor() {
    this.notifications = [];
  }

  getAll() {
    return this.notifications;
  }

  getNumUnchecked() {
    let counter = 0;
    for (let i=0; i<this.notifications.length; i++) {
      if (!this.notifications[i].isClicked) {
        counter++;
      }
    }
    return counter;
  }

  add(type, title, description) {
    this.notifications.unshift(new Notification(type, title, description));
  }

  setItemClicked(itemIndex, isClicked) {
    this.notifications[itemIndex].isClicked = isClicked;
  }
}

export default NotificationRepo;