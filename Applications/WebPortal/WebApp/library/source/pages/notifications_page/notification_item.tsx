import { css, StyleSheet } from 'aphrodite/no-important';
import * as React from 'react';
import { Checkbox } from '../../components';
import { RelativeDate } from '../../components';

interface Properties {

  /** The unique identifier for the notification. */
  id: number;

  /** The description of the notification. */
  description: string;

  /** The datetime when the notification was created. */
  timestamp: Date;

  /** The link to the page for the notification. */
  url: string;

  /** Whether the notification is unread. */
  isUnread?: boolean;

  /** Whether the notification is selected. */
  isSelected?: boolean;

  /** Whether the unread indicator is hidden. */
  hideIndicator?: boolean;

  /** The current date for RelativeDate. */
  today?: Date;

  /** Called when the selected state changes. */
  onSelect?: (id: number, isSelected: boolean) => void;
}

/** Displays a single notification as a linked item. */
export function NotificationItem(props: Properties): JSX.Element {
  const isUnread = props.isUnread ?? false;
  const isSelected = props.isSelected ?? false;
  const hideIndicator = props.hideIndicator ?? false;
  const onCheckboxClick = (checked: boolean) => {
    props.onSelect?.(props.id, checked);
  };
  return (
    <a href={props.url}
        className={css(STYLES.link,
          hideIndicator && STYLES.linkHideIndicator,
          isSelected && STYLES.linkSelected)}>
      <div className={css(STYLES.checkboxContainer)}>
        <Checkbox checked={isSelected} onClick={onCheckboxClick}/>
      </div>
      <div className={css(STYLES.body)}>
        <div className={css(STYLES.header)}>
          {!hideIndicator &&
            <div className={css(STYLES.indicator)}>
              {isUnread &&
                <span aria-hidden='true' className={css(STYLES.dot)}/>}
            </div>}
          <h2 className={css(STYLES.description,
              isUnread && STYLES.descriptionUnread)}>
            {props.description}
          </h2>
        </div>
        <div className={css(STYLES.dateSpacer)}/>
        <div className={css(STYLES.date)}>
          <RelativeDate
              datetime={props.timestamp}
              today={props.today}/>
        </div>
      </div>
    </a>);
}

const STYLES = StyleSheet.create({
  link: {
    display: 'flex',
    alignItems: 'center',
    backgroundColor: '#FFFFFF',
    borderBottom: '1px solid #E6E6E6',
    textDecoration: 'none',
    padding: '18px 18px 17px 8px',
    boxSizing: 'border-box',
    ':hover': {
      backgroundColor: '#F8F8F8'
    },
    ':focus-visible': {
      outlineOffset: '-1px',
      outline: '1px solid #684BC7'
    },
    '@container (min-width: 768px)': {
      padding: '10px 18px 9px'
    }
  },
  linkHideIndicator: {
    paddingInlineStart: '18px'
  },
  linkSelected: {
    '@container (min-width: 768px)': {
      backgroundColor: '#E2E0FF',
      ':hover': {
        backgroundColor: '#E2E0FF'
      }
    }
  },
  checkboxContainer: {
    display: 'none',
    flexShrink: 0,
    alignItems: 'center',
    marginRight: '18px',
    '@container (min-width: 768px)': {
      display: 'flex'
    }
  },
  body: {
    display: 'flex',
    alignItems: 'flex-start',
    flex: 1,
    minWidth: 0
  },
  header: {
    display: 'flex',
    alignItems: 'flex-start',
    flex: 1,
    minWidth: 0,
    overflow: 'hidden'
  },
  indicator: {
    flexShrink: 0,
    width: '12px',
    paddingTop: '5px',
    '@container (min-width: 768px)': {
      paddingTop: '4px'
    }
  },
  dot: {
    display: 'block',
    width: '8px',
    height: '8px',
    backgroundColor: '#007AE6',
    borderRadius: '8px'
  },
  description: {
    margin: 0,
    fontSize: '0.875rem',
    fontWeight: 400,
    color: '#333333',
    display: '-webkit-box' as any,
    WebkitLineClamp: 2,
    WebkitBoxOrient: 'vertical' as any,
    overflow: 'hidden',
    width: '100%',
    '@container (min-width: 768px)': {
      display: 'inline-block',
      whiteSpace: 'nowrap',
      textOverflow: 'ellipsis'
    }
  },
  descriptionUnread: {
    fontWeight: 500
  },
  dateSpacer: {
    flexShrink: 0,
    width: '18px'
  },
  date: {
    flexShrink: 0,
    textTransform: 'capitalize',
    textAlign: 'end',
    '@container (min-width: 768px)': {
      textTransform: 'none'
    }
  }
});
