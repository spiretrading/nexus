import { css, StyleSheet } from 'aphrodite/no-important';
import * as React from 'react';
import { EmptyMessage, IconLabelButton, Link } from '../../components';
import { NotificationItem } from './notification_item';

/** A single notification entry. */
export interface Notification {

  /** The description of the notification. */
  description: string;

  /** The datetime when the notification was created. */
  timestamp: Date;

  /** The link to the page for the notification. */
  url: string;

  /** Whether the notification is unread. */
  isUnread: boolean;
}

interface Properties {

  /** The list of notifications to display. */
  notifications: Notification[];

  /** The current date for relative date display. */
  today?: Date;

  /** Whether the popover is open. */
  isOpen?: boolean;

  /** Called when the dismiss all button is clicked. */
  onDismissAll?: () => void;

  /** Called when the popover is closed. */
  onClose?: () => void;
}

/** Displays a popover with a list of notifications. */
export function NotificationsPopover(props: Properties): JSX.Element {
  const ref = React.useRef<HTMLDivElement>(null);
  const contentRef = React.useRef<HTMLDivElement>(null);
  const [contentHeight, setContentHeight] =
    React.useState<number | undefined>(undefined);
  React.useEffect(() => {
    if(!ref.current) {
      return;
    }
    if(props.isOpen) {
      (ref.current as any).showPopover();
    } else {
      (ref.current as any).hidePopover();
    }
  }, [props.isOpen]);
  React.useEffect(() => {
    if(!ref.current) {
      return;
    }
    const element = ref.current;
    const onToggle = (event: Event) => {
      if((event as any).newState === 'closed') {
        props.onClose?.();
      }
    };
    element.addEventListener('toggle', onToggle);
    return () => element.removeEventListener('toggle', onToggle);
  }, [props.onClose]);
  React.useEffect(() => {
    if(!contentRef.current) {
      return;
    }
    const observer = new ResizeObserver((entries) => {
      for(const entry of entries) {
        setContentHeight(entry.target.scrollHeight);
      }
    });
    observer.observe(contentRef.current);
    return () => observer.disconnect();
  }, []);
  const unreadNotifications = props.notifications.filter(
    (notification) => notification.isUnread);
  const unreadCount = unreadNotifications.length;
  const hasNotifications = props.notifications.length > 0;
  const viewAllHref = unreadCount > 0 ?
    '/notifications' : '/notifications?status=all';
  return (
    <div ref={ref} {...{popover: 'auto'} as any}
        className={css(STYLES.popover)}>
      <div className={css(STYLES.header)}>
        <h2 className={css(STYLES.title)}>Notifications</h2>
      </div>
      <div className={css(STYLES.heightWrapper)}
          style={{height: contentHeight}}>
        <div ref={contentRef}>
          {!hasNotifications &&
            <div className={css(STYLES.emptyMessage)}>
              <EmptyMessage message='No notifications.'/>
            </div>}
          {hasNotifications && unreadCount === 0 &&
            <>
              <div className={css(STYLES.emptyMessage)}>
                <EmptyMessage
                    message={
                      'There are no new notifications at the moment.'}/>
              </div>
              <div className={css(STYLES.spacer)}/>
              <div className={css(STYLES.footer)}>
                <IconLabelButton icon='' label='Dismiss All'
                    variant={IconLabelButton.Variant.LABEL}
                    disabled={true}/>
                <Link label='View All' href={viewAllHref}/>
              </div>
            </>}
          {unreadCount > 0 &&
            <>
              <ul className={css(STYLES.list)}>
                {unreadNotifications.map(
                    (notification, index) =>
                  <li key={index}>
                    <NotificationItem
                        description={notification.description}
                        timestamp={notification.timestamp}
                        url={notification.url}
                        isUnread={true}
                        hideIndicator={true}
                        today={props.today}
                        style={index === unreadNotifications.length - 1 ?
                          {borderBottomColor: 'transparent'} : undefined}/>
                  </li>)}
              </ul>
              <div className={css(STYLES.spacer)}/>
              <div className={css(STYLES.footer)}>
                <IconLabelButton icon='' label='Dismiss All'
                    variant={IconLabelButton.Variant.LABEL}
                    onClick={props.onDismissAll}/>
                <Link label='View All' href={viewAllHref}/>
              </div>
            </>}
        </div>
      </div>
    </div>);
}

const STYLES = StyleSheet.create({
  popover: {
    width: '320px',
    maxHeight: 'calc(100dvh - 80px)',
    display: 'flex',
    flexDirection: 'column',
    backgroundColor: '#FFFFFF',
    border: 'none',
    borderRadius: '1px',
    boxShadow: '0 0 6px rgb(0 0 0 / 0.40)',
    padding: '18px 0',
    overflow: 'hidden',
    fontSize: '0.875rem',
    color: '#333333',
    fontFamily: '"Roboto", system-ui, sans-serif',
    opacity: 0,
    transitionProperty: 'opacity, display',
    transitionDuration: '200ms',
    transitionTimingFunction: 'ease-out',
    transitionBehavior: 'allow-discrete' as any,
    ':popover-open': {
      opacity: 1
    }
  },
  header: {
    padding: '0 18px 12px'
  },
  title: {
    fontWeight: 500,
    fontSize: '0.875rem',
    color: '#4B23A0',
    margin: 0
  },
  heightWrapper: {
    overflow: 'hidden',
    transition: 'height 300ms ease'
  },
  list: {
    listStyle: 'none',
    padding: 0,
    margin: 0,
    overflowY: 'auto',
    overscrollBehavior: 'contain',
    containerType: 'inline-size',
    minHeight: '224px'
  },
  spacer: {
    height: '8px',
    flexShrink: 0
  },
  footer: {
    display: 'flex',
    alignItems: 'center',
    justifyContent: 'space-between',
    padding: '0 18px'
  },
  emptyMessage: {
    padding: '18px',
    height: '224px',
    boxSizing: 'border-box'
  }
});
