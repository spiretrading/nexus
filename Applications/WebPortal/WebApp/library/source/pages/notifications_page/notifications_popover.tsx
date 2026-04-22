import { css, StyleSheet } from 'aphrodite/no-important';
import * as React from 'react';
import { EmptyMessage, IconLabelButton, Link } from '../../components';
import { NotificationItem } from './notification_item';
import { Notification } from './notifications_model';

interface Properties {

  /** The id of the popover element. */
  id?: string;

  /** The list of notifications to display. */
  notifications: Notification[];

  /** The current date for relative date display. */
  today?: Date;

  /** Called when the dismiss all button is clicked. */
  onDismissAll?: () => void;

  /** Called when the popover is opened. */
  onOpen?: () => void;

  /** Called when the popover is closed. */
  onClose?: () => void;

  /** An additional class name to apply to the popover element. */
  className?: string;
}

interface State {
  contentHeight: number | undefined;
}

/** Displays a popover with a list of notifications. */
export class NotificationsPopover extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {
      contentHeight: undefined
    };
    this.popoverRef = React.createRef();
    this.contentRef = React.createRef();
  }

  public componentDidMount(): void {
    if(this.popoverRef.current) {
      this.popoverRef.current.addEventListener('toggle', this.onToggle);
    }
    if(this.contentRef.current) {
      this.observer = new ResizeObserver((entries) => {
        for(const entry of entries) {
          this.setState({contentHeight: entry.target.scrollHeight});
        }
      });
      this.observer.observe(this.contentRef.current);
    }
  }

  public componentWillUnmount(): void {
    if(this.popoverRef.current) {
      this.popoverRef.current.removeEventListener('toggle', this.onToggle);
    }
    this.observer?.disconnect();
  }

  public render(): JSX.Element {
    const unreadNotifications = this.props.notifications.filter(
      (notification) => !notification.isRead);
    const unreadCount = unreadNotifications.length;
    const hasNotifications = this.props.notifications.length > 0;
    const viewAllHref = (() => {
      if(unreadCount > 0) {
        return '/notifications';
      }
      return '/notifications?status=all';
    })();
    return (
      <div ref={this.popoverRef} id={this.props.id}
          {...{popover: 'auto'} as any}
          className={`${css(STYLES.popover)} ${this.props.className || ''}`}>
        <div className={css(STYLES.header)}>
          <h2 className={css(STYLES.title)}>Notifications</h2>
        </div>
        <div className={css(STYLES.heightWrapper)}
            style={{height: this.state.contentHeight}}>
          <div ref={this.contentRef}>
            {!hasNotifications &&
              <div className={css(STYLES.emptyMessage)}>
                <EmptyMessage message='No notifications.'/>
              </div>}
            {hasNotifications && unreadCount === 0 &&
              <>
                <div className={css(STYLES.emptyMessage)}>
                  <EmptyMessage
                    message='There are no new notifications at the moment.'/>
                </div>
                <div className={css(STYLES.spacer)}/>
                <div className={css(STYLES.footer)}>
                  <IconLabelButton icon='' label='Dismiss All'
                    variant={IconLabelButton.Variant.LABEL} disabled/>
                  <Link label='View All' href={viewAllHref}/>
                </div>
              </>}
            {unreadCount > 0 &&
              <>
                <ul className={css(STYLES.list)}>
                  {unreadNotifications.map((notification, index) =>
                    <li key={index}>
                      <NotificationItem
                        description={notification.description}
                        timestamp={notification.timestamp.toDate()}
                        url={`/notifications/${notification.id}`}
                        isUnread={true}
                        hideIndicator={true}
                        today={this.props.today}
                        style={index === unreadNotifications.length - 1 ?
                          {borderBottomColor: 'transparent'} : undefined}/>
                    </li>)}
                </ul>
                <div className={css(STYLES.spacer)}/>
                <div className={css(STYLES.footer)}>
                  <IconLabelButton icon='' label='Dismiss All'
                    variant={IconLabelButton.Variant.LABEL}
                    onClick={this.props.onDismissAll}/>
                  <Link label='View All' href={viewAllHref}/>
                </div>
              </>}
          </div>
        </div>
      </div>);
  }

  private popoverRef: React.RefObject<HTMLDivElement>;
  private contentRef: React.RefObject<HTMLDivElement>;
  private observer: ResizeObserver | undefined;
  private onToggle = (event: Event) => {
    if((event as any).newState === 'open') {
      this.props.onOpen?.();
    } else {
      this.props.onClose?.();
    }
  };
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
    inset: 'unset',
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
    padding: '0 18px',
    height: '224px',
    boxSizing: 'border-box'
  }
});
