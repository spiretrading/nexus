import './style.scss';
import React from 'react';
import UpdatableView from 'commons/updatable-view';
import deviceDetector from 'utils/device-detector';
import NotificationType from 'components/reusables/common/notification/notification-type';

class View extends UpdatableView {
  constructor(react, controller, componentModel) {
    super(react, controller, componentModel);

    this.initialize = this.initialize.bind(this);
    this.closePopup = this.closePopup.bind(this);
    this.openPopup = this.openPopup.bind(this);
  }

  /** @private */
  onIconClick(e) {
    if (!deviceDetector.isMobile()) {
      // desktop
      if (this.componentModel.isOpen) {
        this.closePopup();
      } else {
        this.openPopup();
      }
    } else {
      // mobile
      if (this.componentModel.isOpen) {
        this.controller.closePanel();
      } else {
        this.controller.openPanel();
      }
    }
  }

  /** @private */
  openPopup() {
    this.controller.openPopup();

    $(document).mouseup((e) => {
      var $container = $("#" + this.componentModel.componentId + ' .list-panel');
      var $icon = $("#" + this.componentModel.componentId + ' .button-area');

      if (!$container.is(e.target) // if the target of the click isn't the container...
        && $container.has(e.target).length === 0
        && !$icon.is(e.target)
        && $icon.has(e.target).length === 0) // ... nor a descendant of the container
      {
        this.closePopup();
      }
    });
  }

  /** @private */
  closePopup() {
    this.controller.closePopup();
    $(document).unbind('mouseup');
  }

  initialize() {
    $('#' + this.componentModel.componentId + ' .body').slimScroll({
      height: '395px',
      opacity: 0
    }).mouseover(function() {
      $(this).next('.slimScrollBar').css('opacity', 0.4);
    });
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
    let iconClass = 'icon-notifications';
    let countLabelClass = 'count-label-wrapper';
    let buttonAreaClass = 'button-area';
    if (this.componentModel.numUnchecked > 0) {
      iconClass += ' new';
      countLabelClass += ' new';
      buttonAreaClass += ' new';
    } else if (this.componentModel.isOpen) {
      iconClass += ' opened';
    }

    let panelClass = 'list-panel';
    if (deviceDetector.isMobile() || !this.componentModel.isOpen) {
      panelClass += ' hidden';
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
          <div className="read-dot unread" title="mark as unread" onClick={this.onReadDotClick.bind(this)}></div>
          <div className="read-dot read" title="mark as read" onClick={this.onReadDotClick.bind(this)}></div>
          <div className="title">
            {notification.title}
          </div>
          <div className="description">
            {notification.description}
          </div>
        </li>
      );
    }

    return (
        <div id={this.componentModel.componentId} className="top-nav-notification-container desktop">
          <span className={iconClass}></span>
          <div className={countLabelClass}>
            <div className="count-label">
              {this.componentModel.numUnchecked}
            </div>
          </div>
          <div className={panelClass}>
            <div className="pointer"></div>
            <div className="header no-select">
              Notifications
            </div>
            <div className="body">
              <ul>
                {notificationItems}
              </ul>
            </div>
          </div>
          <div className={buttonAreaClass} onClick={this.onIconClick.bind(this)}></div>
        </div>
    );
  }
}

export default View;
