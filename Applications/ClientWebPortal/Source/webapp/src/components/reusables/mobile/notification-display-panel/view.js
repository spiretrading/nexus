import './style.scss';
import React from 'react';
import UpdatableView from 'commons/updatable-view';
import NotificationType from 'components/reusables/common/notification/notification-type';

class View extends UpdatableView {
  constructor(react, controller, componentModel) {
    super(react, controller, componentModel);
  }

  /** @private */
  onNotificationItemClick(e) {
    let $target = $(e.target);
    while (!$target.is('li')) {
      $target = $target.parent();
    }
    let itemIndex = $target.attr('data-index');
    this.controller.itemClicked(itemIndex);
  }

  /** @private */
  onReadDotClick(e) {
    e.stopPropagation();
    e.nativeEvent.stopImmediatePropagation();
    let $target = $(e.target).parent();
    let wasRead = !$target.hasClass('new');
    let itemIndex = $target.attr('data-index');
    this.controller.setItemClickStatus(itemIndex, !wasRead)
  }

  render() {
    let containerClass = 'notification-display-panel-container';
    if (this.componentModel.isOpen) {
      containerClass += ' opened';
    } else if (this.componentModel.hasEverBeenOpened && !this.componentModel.isOpen) {
      containerClass += ' closed';
    }

    let notificationItems = [];
    for (let i=0; i<this.componentModel.notifications.length; i++) {
      let notification = this.componentModel.notifications[i];
      let icon;
      if (notification.type === NotificationType.REPORT) {
        icon = 'icon-reports';
      }

      let notificationItemClass = 'no-select';
      if (!notification.isClicked) {
        notificationItemClass += ' new';
      }

      notificationItems.push(
        <li key={i} className={notificationItemClass} data-index={i} onClick={this.onNotificationItemClick.bind(this)}>
          <span className={icon}></span>
          <div className="title">
            {notification.title}
          </div>
          <div className="description">
            {notification.description}
          </div>
          <div className="read-button" onClick={this.onReadDotClick.bind(this)}>
            <div className="read-dot"></div>
          </div>
        </li>
      );
    }

    return (
        <div id={this.componentModel.componentId} className={containerClass}>
          <ul>
            {notificationItems}
          </ul>
        </div>
    );
  }
}

export default View;
