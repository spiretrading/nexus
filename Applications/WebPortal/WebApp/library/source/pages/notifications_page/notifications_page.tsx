import { css, StyleSheet } from 'aphrodite/no-important';
import * as Beam from 'beam';
import * as Nexus from 'nexus';
import * as React from 'react';
import { Checkbox, DateInput, EmptyMessage, FilterChip, FilterInput,
  IconLabelButton, PageLayout, Pagination, SegmentButton, SegmentedControl
  } from '../../components';
import { NotificationItem } from './notification_item';
import { NotificationItemPlaceholder } from './notification_item_placeholder';
import { getNotificationUrl } from './notification_url';
import { NotificationsFilterModal } from './notifications_filter_modal';
import { NotificationsFilter } from './notifications_model';

interface Properties {

  /** The initial read status filter. */
  readStatus: Nexus.Notification.ReadState;

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
  filter: NotificationsFilter;
  isFilterModalOpen: boolean;
}

/** Displays the notifications page. */
export class NotificationsPage extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {
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
        <main ref={this._mainRef} className={css(STYLES.main)}>
          <Toolbar
            readStatus={this.props.readStatus}
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
          <NotificationsContent
            displayStatus={this.props.displayStatus}
            notifications={this.props.notifications}
            selected={this.props.selected}
            readStatus={this.props.readStatus}
            hasFilters={this.state.filter.query.length > 0 ||
              this.state.filter.categories.size > 0 ||
              this.state.filter.startDate != null ||
              this.state.filter.endDate != null}
            query={this.state.filter.query}
            filteredCount={this.props.filteredCount}
            pageIndex={this.props.pageIndex}
            onSelectionChange={this.props.onSelectionChange}
            onMarkAsRead={this.props.onMarkAsRead}
            onMarkAsUnread={this.props.onMarkAsUnread}
            onPageNavigate={this.onPageNavigate}
            onNotificationClick={this.props.onNotificationClick}/>
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
    this.submit(this.props.readStatus, filter);
  };

  private onReadStatusChange = (readStatus: Nexus.Notification.ReadState) => {
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
    this.submit(this.props.readStatus, filter);
  };

  private onStartDateChange = (startDate: Beam.Date) => {
    const filter = {...this.state.filter, startDate};
    this.setState({filter});
    this.submit(this.props.readStatus, filter);
  };

  private onEndDateChange = (endDate: Beam.Date) => {
    const filter = {...this.state.filter, endDate};
    this.setState({filter});
    this.submit(this.props.readStatus, filter);
  };

  private onOpenFilterModal = () => {
    this.setState({isFilterModalOpen: true});
  };

  private onCloseFilterModal = () => {
    this.setState({isFilterModalOpen: false});
  };

  private onFilterSubmit = (submitted: NotificationsFilter) => {
    this.setState({filter: submitted, isFilterModalOpen: false});
    this.submit(this.props.readStatus, submitted);
  };

  private onPageNavigate = (pageIndex: number) => {
    let element = this._mainRef.current?.parentElement;
    while(element && element.scrollTop === 0 &&
        element !== document.documentElement) {
      element = element.parentElement;
    }
    element?.scrollTo(0, 0);
    this.props.onSubmit?.(this.props.readStatus, this.state.filter, pageIndex);
  };

  private _mainRef = React.createRef<HTMLElement>();
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
          aria-label='Filters'
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
  return (
    <div className={css(STYLES.wideToolbar)}>
      <FilterBar
        readStatus={props.readStatus}
        filter={props.filter}
        onQueryChange={props.onQueryChange}
        onReadStatusChange={props.onReadStatusChange}
        onStartDateChange={props.onStartDateChange}
        onEndDateChange={props.onEndDateChange}/>
      <div className={css(STYLES.wideCategoriesGap)}/>
      <Categories
        categories={props.filter.categories}
        onCategoryChange={props.onCategoryChange}/>
    </div>);
}

function FilterBar(props: {
    readStatus: Nexus.Notification.ReadState;
    filter: NotificationsFilter;
    onQueryChange?: (query: string) => void;
    onReadStatusChange?: (readStatus: Nexus.Notification.ReadState) => void;
    onStartDateChange?: (date: Beam.Date) => void;
    onEndDateChange?: (date: Beam.Date) => void;
  }): JSX.Element {
  return (
    <div className={css(STYLES.filterBar)}>
      <QueryAndDate
        filter={props.filter}
        onQueryChange={props.onQueryChange}
        onStartDateChange={props.onStartDateChange}
        onEndDateChange={props.onEndDateChange}/>
      <div className={css(STYLES.filterBarFlexGap)}/>
      <div className={css(STYLES.filterBarFixedGap)}/>
      <div className={css(STYLES.filterBarReadStatus)}>
        <ReadStatusSegment name='read-status-wide'
          readStatus={props.readStatus}
          onSelectStatus={props.onReadStatusChange}/>
      </div>
    </div>);
}

function QueryAndDate(props: {
    filter: NotificationsFilter;
    onQueryChange?: (query: string) => void;
    onStartDateChange?: (date: Beam.Date) => void;
    onEndDateChange?: (date: Beam.Date) => void;
  }): JSX.Element {
  const isInvalid = props.filter.startDate && props.filter.endDate &&
    props.filter.startDate.toDate() > props.filter.endDate.toDate();
  return (
    <div className={css(STYLES.queryAndDate)}>
      <FilterInput value={props.filter.query}
        placeholder='Filter notifications'
        onChange={props.onQueryChange}/>
      <div className={css(STYLES.queryDateGap)}/>
      <DateCreated
        startDate={props.filter.startDate}
        endDate={props.filter.endDate}
        error={isInvalid}
        onStartDateChange={props.onStartDateChange}
        onEndDateChange={props.onEndDateChange}/>
      <ErrorSection isInvalid={isInvalid}/>
    </div>);
}

function Categories(props: {
    categories: Set<Nexus.Notification.Category>;
    onCategoryChange?: (
      category: Nexus.Notification.Category, isChecked: boolean) => void;
  }): JSX.Element {
  return (
    <div className={css(STYLES.chipRow)}>
      <FilterChip label='Account Modification'
        isChecked={props.categories.has(
          Nexus.Notification.Category.ACCOUNT_MODIFICATION)}
        onChange={(isChecked) => props.onCategoryChange?.(
          Nexus.Notification.Category.ACCOUNT_MODIFICATION, isChecked)}/>
      <div className={css(STYLES.chipSpacing)}/>
      <FilterChip label='Report'
        isChecked={props.categories.has(Nexus.Notification.Category.REPORT)}
        onChange={(isChecked) => props.onCategoryChange?.(
          Nexus.Notification.Category.REPORT, isChecked)}/>
    </div>);
}

function DateCreated(props: {
    startDate: Beam.Date;
    endDate: Beam.Date;
    error?: boolean;
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
        error={props.error}
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

function NotificationsContent(props: {
    displayStatus: NotificationsPage.DisplayStatus;
    notifications: Nexus.Notification[];
    selected: Set<Nexus.Notification.Id>;
    readStatus: Nexus.Notification.ReadState;
    hasFilters: boolean;
    query: string;
    filteredCount: number;
    pageIndex: number;
    onSelectionChange?: (selected: Set<Nexus.Notification.Id>) => void;
    onMarkAsRead?: () => void;
    onMarkAsUnread?: () => void;
    onPageNavigate?: (pageIndex: number) => void;
    onNotificationClick?: (notification: Nexus.Notification) => void;
  }): JSX.Element {
  const isFallback =
    props.displayStatus === NotificationsPage.DisplayStatus.ERROR ||
    props.displayStatus === NotificationsPage.DisplayStatus.NO_RESULTS ||
    props.displayStatus === NotificationsPage.DisplayStatus.EMPTY;
  if(isFallback) {
    return (
      <Fallback
        displayStatus={props.displayStatus}
        readStatus={props.readStatus}
        hasFilters={props.hasFilters}/>);
  }
  return (
    <>
      <NotificationsSection
        displayStatus={props.displayStatus}
        notifications={props.notifications}
        selected={props.selected}
        query={props.query}
        onSelectionChange={props.onSelectionChange}
        onMarkAsRead={props.onMarkAsRead}
        onMarkAsUnread={props.onMarkAsUnread}
        onNotificationClick={props.onNotificationClick}/>
      <PaginationSection
        displayStatus={props.displayStatus}
        filteredCount={props.filteredCount}
        pageIndex={props.pageIndex}
        onNavigate={props.onPageNavigate}/>
    </>);
}

function NotificationsSection(props: {
    displayStatus: NotificationsPage.DisplayStatus;
    notifications: Nexus.Notification[];
    selected: Set<Nexus.Notification.Id>;
    query: string;
    onSelectionChange?: (selected: Set<Nexus.Notification.Id>) => void;
    onMarkAsRead?: () => void;
    onMarkAsUnread?: () => void;
    onNotificationClick?: (notification: Nexus.Notification) => void;
  }): JSX.Element {
  const hasSelection = props.selected.size > 0;
  const isAllSelected = props.notifications.length > 0 &&
    props.notifications.every((n) => props.selected.has(n.id));
  const isIndeterminate = hasSelection && !isAllSelected;
  const onSelectAll = () => {
    if(isAllSelected) {
      props.onSelectionChange?.(new Set());
    } else {
      props.onSelectionChange?.(
        new Set(props.notifications.map((n) => n.id)));
    }
  };
  return (
    <div className={css(STYLES.notificationsSection)}>
      <div className={css(STYLES.selectionControlsContainer)}>
        <SelectionControls
          isAllSelected={isAllSelected}
          isIndeterminate={isIndeterminate}
          hasSelectedUnread={props.notifications.some(
            (n) => props.selected.has(n.id) && !n.isRead)}
          hasSelectedRead={props.notifications.some(
            (n) => props.selected.has(n.id) && n.isRead)}
          onSelectAll={onSelectAll}
          onMarkAsRead={props.onMarkAsRead}
          onMarkAsUnread={props.onMarkAsUnread}/>
      </div>
      <NotificationsList
        displayStatus={props.displayStatus}
        notifications={props.notifications}
        selected={props.selected}
        query={props.query}
        onSelectionChange={props.onSelectionChange}
        onNotificationClick={props.onNotificationClick}/>
    </div>);
}

function SelectionControls(props: {
    isAllSelected: boolean;
    isIndeterminate: boolean;
    hasSelectedUnread: boolean;
    hasSelectedRead: boolean;
    onSelectAll?: () => void;
    onMarkAsRead?: () => void;
    onMarkAsUnread?: () => void;
  }): JSX.Element {
  return (
    <div className={css(STYLES.selectionControls)}>
      <Checkbox checked={props.isAllSelected}
        indeterminate={props.isIndeterminate}
        onClick={props.onSelectAll}/>
      <div className={css(STYLES.selectionControlsGap1)}/>
      <IconLabelButton icon='' label='Mark as Read'
        variant={IconLabelButton.Variant.LABEL}
        disabled={!props.hasSelectedUnread}
        onClick={props.onMarkAsRead}/>
      <div className={css(STYLES.selectionControlsGap2)}/>
      <IconLabelButton icon='' label='Mark as Unread'
        variant={IconLabelButton.Variant.LABEL}
        disabled={!props.hasSelectedRead}
        onClick={props.onMarkAsUnread}/>
    </div>);
}

function NotificationsList(props: {
    displayStatus: NotificationsPage.DisplayStatus;
    notifications: Nexus.Notification[];
    selected: Set<Nexus.Notification.Id>;
    query: string;
    onSelectionChange?: (selected: Set<Nexus.Notification.Id>) => void;
    onNotificationClick?: (notification: Nexus.Notification) => void;
  }): JSX.Element {
  const isLoading =
    props.displayStatus === NotificationsPage.DisplayStatus.IN_PROGRESS;
  if(isLoading) {
    return (
      <ul className={css(STYLES.notificationsList)}>
        {Array.from({length: 5}, (_, i) =>
          <li key={i}><NotificationItemPlaceholder/></li>)}
      </ul>);
  }
  const onSelect = (id: Nexus.Notification.Id, isSelected: boolean) => {
    const next = new Set(props.selected);
    if(isSelected) {
      next.add(id);
    } else {
      next.delete(id);
    }
    props.onSelectionChange?.(next);
  };
  const lastIndex = props.notifications.length - 1;
  return (
    <ul className={css(STYLES.notificationsList)}>
      {props.notifications.map((notification, i) =>
        <li key={notification.id}>
          <NotificationItem
            description={notification.description}
            timestamp={notification.timestamp.toDate()}
            url={getNotificationUrl(notification)}
            isUnread={!notification.isRead}
            isSelected={props.selected.has(notification.id)}
            highlight={props.query}
            onClick={() => props.onNotificationClick?.(notification)}
            onSelect={(isSelected) =>
              onSelect(notification.id, isSelected)}
            style={i === lastIndex ?
              {borderBottomColor: 'transparent'} : undefined}/>
        </li>)}
    </ul>);
}

function PaginationSection(props: {
    displayStatus: NotificationsPage.DisplayStatus;
    filteredCount: number;
    pageIndex: number;
    onNavigate?: (pageIndex: number) => void;
  }): JSX.Element {
  const isHidden =
    props.displayStatus === NotificationsPage.DisplayStatus.IN_PROGRESS ||
    props.filteredCount <= 50;
  return (
    <div className={css(STYLES.paginationSection,
        isHidden && STYLES.paginationSectionHidden)}>
      <div className={css(STYLES.paginationGap)}/>
      <Pagination pageIndex={props.pageIndex}
        totalCount={props.filteredCount}
        onNavigate={props.onNavigate}/>
    </div>);
}

function Fallback(props: {
    displayStatus: NotificationsPage.DisplayStatus;
    readStatus: Nexus.Notification.ReadState;
    hasFilters: boolean;
  }): JSX.Element {
  const message = (() => {
    if(props.displayStatus === NotificationsPage.DisplayStatus.ERROR) {
      return 'There was an error loading your notifications.';
    }
    if(props.displayStatus === NotificationsPage.DisplayStatus.EMPTY) {
      return 'No notifications.';
    }
    if(props.readStatus === Nexus.Notification.ReadState.UNREAD &&
        !props.hasFilters) {
      return 'There are no new notifications at the moment.';
    }
    return 'No results found. Try adjusting filters.';
  })();
  return (
    <div className={css(STYLES.fallback)}>
      <EmptyMessage message={message}/>
    </div>);
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
  },
  notificationsSection: {
    containerType: 'inline-size'
  },
  notificationsList: {
    padding: 0,
    margin: 0,
    listStyle: 'none',
    containerType: 'inline-size'
  },
  selectionControlsContainer: {
    display: 'none',
    '@container (min-width: 768px)': {
      display: 'block'
    }
  },
  paginationSection: {
    padding: '0 18px'
  },
  paginationSectionHidden: {
    maxHeight: 0,
    overflow: 'hidden'
  },
  paginationGap: {
    height: '30px'
  },
  selectionControls: {
    display: 'flex',
    alignItems: 'center',
    padding: '8px 18px 7px',
    borderBottom: '1px solid #E6E6E6'
  },
  selectionControlsGap1: {
    width: '10px',
    flexShrink: 0
  },
  selectionControlsGap2: {
    width: '8px',
    flexShrink: 0
  },
  fallback: {
    padding: '0 18px',
    flex: '1 1 auto'
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
