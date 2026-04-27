import { css, StyleSheet } from 'aphrodite/no-important';
import * as Beam from 'beam';
import * as Nexus from 'nexus';
import * as React from 'react';
import { DateInput, FilterChip, FilterInput, IconLabelButton, PageLayout,
  SegmentButton, SegmentedControl } from '../../components';
import { NotificationsFilterModal } from './notifications_filter_modal';
import { NotificationsFilter } from './notifications_model';

interface Properties {

  /** The current display status. */
  displayStatus: NotificationsPage.DisplayStatus;

  /** The notifications to display. */
  notifications: Nexus.Notification[];

  /** The total number of notifications matching the current filters. */
  filteredCount: number;

  /** The current page index. */
  pageIndex: number;

  /** The set of selected notification ids. */
  selected: Set<Nexus.Notification.Id>;

  /** Called when the user submits a query with filters. */
  onSubmit?: (readStatus: Nexus.Notification.ReadState,
    filter: NotificationsFilter, pageIndex: number) => void;

  /** Called when the selection changes. */
  onSelectionChange?: (selected: Set<Nexus.Notification.Id>) => void;

  /** Called to mark the selected notifications as read. */
  onMarkAsRead?: () => void;

  /** Called to mark the selected notifications as unread. */
  onMarkAsUnread?: () => void;

  /** Called when a notification is clicked. */
  onNotificationClick?: (notification: Nexus.Notification) => void;
}

interface State {
  readStatus: Nexus.Notification.ReadState;
  filter: NotificationsFilter;
  isFilterModalOpen: boolean;
}

/** Displays the notifications page. */
export class NotificationsPage extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {
      readStatus: Nexus.Notification.ReadState.UNREAD,
      filter: {
        query: '',
        categories: new Set<Nexus.Notification.Category>(),
        startDate: null,
        endDate: null
      },
      isFilterModalOpen: false
    };
  }

  public render(): JSX.Element {
    return (
      <PageLayout>
        <main className={css(STYLES.main)}>
          <Toolbar
            readStatus={this.state.readStatus}
            filter={this.state.filter}
            isFilterModalOpen={this.state.isFilterModalOpen}
            onQueryChange={this.onQueryChange}
            onReadStatusChange={this.onReadStatusChange}
            onCategoryChange={this.onCategoryChange}
            onStartDateChange={this.onStartDateChange}
            onEndDateChange={this.onEndDateChange}
            onOpenFilterModal={this.onOpenFilterModal}
            onCloseFilterModal={this.onCloseFilterModal}
            onFilterSubmit={this.onFilterSubmit}/>
          <div className={css(STYLES.contentGap)}/>
          <NotificationsContent/>
        </main>
      </PageLayout>);
  }

  private submit(readStatus: Nexus.Notification.ReadState,
      filter: NotificationsFilter) {
    this.props.onSubmit?.(readStatus, filter, 0);
  }

  private onQueryChange = (query: string) => {
    const filter = {...this.state.filter, query};
    this.setState({filter});
    this.submit(this.state.readStatus, filter);
  };

  private onReadStatusChange = (readStatus: Nexus.Notification.ReadState) => {
    this.setState({readStatus});
    this.submit(readStatus, this.state.filter);
  };

  private onCategoryChange = (
      category: Nexus.Notification.Category, isChecked: boolean) => {
    const categories = new Set(this.state.filter.categories);
    if(isChecked) {
      categories.add(category);
    } else {
      categories.delete(category);
    }
    const filter = {...this.state.filter, categories};
    this.setState({filter});
    this.submit(this.state.readStatus, filter);
  };

  private onStartDateChange = (startDate: Beam.Date) => {
    const filter = {...this.state.filter, startDate};
    this.setState({filter});
    this.submit(this.state.readStatus, filter);
  };

  private onEndDateChange = (endDate: Beam.Date) => {
    const filter = {...this.state.filter, endDate};
    this.setState({filter});
    this.submit(this.state.readStatus, filter);
  };

  private onOpenFilterModal = () => {
    this.setState({isFilterModalOpen: true});
  };

  private onCloseFilterModal = () => {
    this.setState({isFilterModalOpen: false});
  };

  private onFilterSubmit = (submitted: NotificationsFilter) => {
    this.setState({filter: submitted, isFilterModalOpen: false});
    this.submit(this.state.readStatus, submitted);
  };
}

function Toolbar(props: {
    readStatus: Nexus.Notification.ReadState;
    filter: NotificationsFilter;
    isFilterModalOpen: boolean;
    onQueryChange?: (query: string) => void;
    onReadStatusChange?: (readStatus: Nexus.Notification.ReadState) => void;
    onCategoryChange?: (
      category: Nexus.Notification.Category, isChecked: boolean) => void;
    onStartDateChange?: (date: Beam.Date) => void;
    onEndDateChange?: (date: Beam.Date) => void;
    onOpenFilterModal?: () => void;
    onCloseFilterModal?: () => void;
    onFilterSubmit?: (filter: NotificationsFilter) => void;
  }): JSX.Element {
  const onFormSubmit = (event: React.FormEvent) => {
    event.preventDefault();
  };
  return (
    <form aria-label='Notifications Filters'
        className={css(STYLES.toolbar)} onSubmit={onFormSubmit}>
      <NarrowToolbar
        readStatus={props.readStatus}
        filter={props.filter}
        onQueryChange={props.onQueryChange}
        onReadStatusChange={props.onReadStatusChange}
        onOpenFilterModal={props.onOpenFilterModal}/>
      <WideToolbar
        readStatus={props.readStatus}
        filter={props.filter}
        onQueryChange={props.onQueryChange}
        onReadStatusChange={props.onReadStatusChange}
        onCategoryChange={props.onCategoryChange}
        onStartDateChange={props.onStartDateChange}
        onEndDateChange={props.onEndDateChange}/>
      {props.isFilterModalOpen &&
        <NotificationsFilterModal
          filter={props.filter}
          onSubmit={props.onFilterSubmit}
          onClose={props.onCloseFilterModal}/>}
    </form>);
}

function NarrowToolbar(props: {
    readStatus: Nexus.Notification.ReadState;
    filter: NotificationsFilter;
    onQueryChange?: (query: string) => void;
    onReadStatusChange?: (readStatus: Nexus.Notification.ReadState) => void;
    onOpenFilterModal?: () => void;
  }): JSX.Element {
  return (
    <div className={css(STYLES.narrowToolbar)}>
      <div className={css(STYLES.narrowQueryRow)}>
        <div className={css(STYLES.narrowQueryCell)}>
          <FilterInput value={props.filter.query}
            placeholder='Filter notifications'
            onChange={props.onQueryChange}/>
        </div>
        <div className={css(STYLES.narrowFiltersGap)}/>
        <IconLabelButton
          icon='resources/notifications_page/filters.svg'
          label='Filters'
          onClick={props.onOpenFilterModal}/>
      </div>
      <div className={css(STYLES.narrowSegmentGap)}/>
      <ReadStatusSegment name='read-status-narrow'
        readStatus={props.readStatus}
        onSelectStatus={props.onReadStatusChange}/>
    </div>);
}

function WideToolbar(props: {
    readStatus: Nexus.Notification.ReadState;
    filter: NotificationsFilter;
    onQueryChange?: (query: string) => void;
    onReadStatusChange?: (readStatus: Nexus.Notification.ReadState) => void;
    onCategoryChange?: (
      category: Nexus.Notification.Category, isChecked: boolean) => void;
    onStartDateChange?: (date: Beam.Date) => void;
    onEndDateChange?: (date: Beam.Date) => void;
  }): JSX.Element {
  const isInvalid = props.filter.startDate && props.filter.endDate &&
    props.filter.startDate.toDate() > props.filter.endDate.toDate();
  return (
    <div className={css(STYLES.wideToolbar)}>
      <div className={css(STYLES.filterBar)}>
        <div className={css(STYLES.queryAndDate)}>
          <FilterInput value={props.filter.query}
            placeholder='Filter notifications'
            onChange={props.onQueryChange}/>
          <div className={css(STYLES.queryDateGap)}/>
          <DateCreated
            startDate={props.filter.startDate}
            endDate={props.filter.endDate}
            onStartDateChange={props.onStartDateChange}
            onEndDateChange={props.onEndDateChange}/>
          <ErrorSection isInvalid={isInvalid}/>
        </div>
        <div className={css(STYLES.filterBarFlexGap)}/>
        <div className={css(STYLES.filterBarFixedGap)}/>
        <div className={css(STYLES.filterBarReadStatus)}>
          <ReadStatusSegment name='read-status-wide'
            readStatus={props.readStatus}
            onSelectStatus={props.onReadStatusChange}/>
        </div>
      </div>
      <div className={css(STYLES.wideCategoriesGap)}/>
      <div className={css(STYLES.chipRow)}>
        <FilterChip label='Account Modification'
          isChecked={props.filter.categories.has(
            Nexus.Notification.Category.ACCOUNT_MODIFICATION)}
          onChange={(isChecked) => props.onCategoryChange?.(
            Nexus.Notification.Category.ACCOUNT_MODIFICATION, isChecked)}/>
        <div className={css(STYLES.chipSpacing)}/>
        <FilterChip label='Report'
          isChecked={props.filter.categories.has(
            Nexus.Notification.Category.REPORT)}
          onChange={(isChecked) => props.onCategoryChange?.(
            Nexus.Notification.Category.REPORT, isChecked)}/>
      </div>
    </div>);
}

function DateCreated(props: {
    startDate: Beam.Date;
    endDate: Beam.Date;
    onStartDateChange?: (date: Beam.Date) => void;
    onEndDateChange?: (date: Beam.Date) => void;
  }): JSX.Element {
  return (
    <fieldset className={css(STYLES.dateCreated)}>
      <label htmlFor='start-date' className={css(STYLES.dateLabel)}>
        Start
      </label>
      <div className={css(STYLES.dateLabelGap)}/>
      <DateInput id='start-date' value={props.startDate}
        onChange={props.onStartDateChange}
        className={css(STYLES.dateInput)}/>
      <div className={css(STYLES.dateFieldGap)}/>
      <label htmlFor='end-date' className={css(STYLES.dateLabel)}>
        End
      </label>
      <div className={css(STYLES.dateLabelGap)}/>
      <DateInput id='end-date' value={props.endDate}
        onChange={props.onEndDateChange}
        className={css(STYLES.dateInput)}/>
    </fieldset>);
}

function ErrorSection(props: {isInvalid: boolean}): JSX.Element {
  return (
    <div className={css(STYLES.errorSection,
        props.isInvalid && STYLES.errorSectionVisible)}>
      {props.isInvalid &&
        <span className={css(STYLES.errorMessage)}>
          End date must be greater than start date
        </span>}
    </div>);
}

function ReadStatusSegment(props: {
    name: string;
    readStatus: Nexus.Notification.ReadState;
    onSelectStatus?: (status: Nexus.Notification.ReadState) => void;
  }): JSX.Element {
  return (
    <SegmentedControl name={props.name}>
      <SegmentButton label='Unread'
        isChecked={props.readStatus === Nexus.Notification.ReadState.UNREAD}
        onChange={() =>
          props.onSelectStatus?.(Nexus.Notification.ReadState.UNREAD)}/>
      <SegmentButton label='All'
        isChecked={props.readStatus === Nexus.Notification.ReadState.ALL}
        onChange={() =>
          props.onSelectStatus?.(Nexus.Notification.ReadState.ALL)}/>
    </SegmentedControl>);
}

function NotificationsContent(): JSX.Element {
  return <div/>;
}

const STYLES = StyleSheet.create({
  main: {
    padding: '18px 0 40px',
    backgroundColor: '#FFFFFF',
    fontFamily: '"Roboto", system-ui, sans-serif',
    fontWeight: 400,
    color: '#333333',
    display: 'flex',
    flexDirection: 'column',
    flex: '1 1 auto'
  },
  toolbar: {
    padding: '0 18px',
    containerType: 'inline-size'
  },
  narrowToolbar: {
    '@container (min-width: 768px)': {
      display: 'none'
    }
  },
  narrowQueryRow: {
    display: 'flex',
    alignItems: 'center'
  },
  narrowQueryCell: {
    flex: '1 1 0',
    minWidth: 0
  },
  narrowFiltersGap: {
    width: '18px',
    flexShrink: 0
  },
  narrowSegmentGap: {
    height: '12px'
  },
  wideToolbar: {
    display: 'none',
    '@container (min-width: 768px)': {
      display: 'block'
    }
  },
  filterBar: {
    display: 'flex',
    alignItems: 'flex-start'
  },
  queryAndDate: {
    display: 'flex',
    flexDirection: 'column'
  },
  queryDateGap: {
    height: '12px'
  },
  filterBarFlexGap: {
    flex: '1 1 0'
  },
  filterBarFixedGap: {
    width: '18px',
    flexShrink: 0
  },
  filterBarReadStatus: {
    width: '242px',
    flexShrink: 0
  },
  wideCategoriesGap: {
    height: '12px'
  },
  chipRow: {
    display: 'flex',
    alignItems: 'center'
  },
  chipSpacing: {
    width: '10px',
    flexShrink: 0
  },
  dateCreated: {
    display: 'flex',
    alignItems: 'center',
    border: 'none',
    padding: 0,
    margin: 0
  },
  dateLabel: {
    fontSize: '0.875rem',
    color: '#333333',
    flexShrink: 0
  },
  dateLabelGap: {
    width: '8px',
    flexShrink: 0
  },
  dateInput: {
    width: '150px',
    flexShrink: 0
  },
  dateFieldGap: {
    width: '18px',
    flexShrink: 0
  },
  errorSection: {
    overflow: 'hidden',
    maxHeight: 0,
    transition: 'max-height 200ms'
  },
  errorSectionVisible: {
    maxHeight: '28px'
  },
  errorMessage: {
    display: 'block',
    paddingTop: '12px',
    fontSize: '0.875rem',
    color: '#E63F44'
  },
  contentGap: {
    height: '30px'
  }
});

export namespace NotificationsPage {

  /** The status of the displayed notifications. */
  export enum DisplayStatus {

    /** Notifications are being retrieved. */
    IN_PROGRESS,

    /** One or more notifications are displayed. */
    READY,

    /** No notifications match the current filters. */
    NO_RESULTS,

    /** The user has no notifications. */
    EMPTY,

    /** An error occurred loading notifications. */
    ERROR
  }
}
