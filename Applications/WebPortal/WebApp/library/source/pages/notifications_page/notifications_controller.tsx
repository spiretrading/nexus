import * as Beam from 'beam';
import * as Nexus from 'nexus';
import * as React from 'react';
import * as Router from 'react-router-dom';
import { LoadingPage } from '../loading_page';
import { NotificationsFilter } from './notifications_model';
import { NotificationsModel } from './notifications_model';
import { NotificationsPage } from './notifications_page';
import { getNotificationUrl } from './notification_url';

interface Properties extends Router.RouteComponentProps {

  /** The model to use. */
  model: NotificationsModel;
}

interface State {
  isLoaded: boolean;
  cannotLoad: boolean;
  redirect: string;
  displayStatus: NotificationsPage.DisplayStatus;
  notifications: Nexus.Notification[];
  filteredCount: number;
  pageIndex: number;
  selected: Set<Nexus.Notification.Id>;
}

/** Implements a controller for the NotificationsPage. */
export class NotificationsController extends
    React.Component<Properties, State> {

  /** The maximum number of notifications displayed per page. */
  public static readonly PAGE_SIZE = 50;

  constructor(props: Properties) {
    super(props);
    const parsed = parseSearch(props.location.search);
    this.state = {
      isLoaded: false,
      cannotLoad: false,
      redirect: null,
      displayStatus: NotificationsPage.DisplayStatus.IN_PROGRESS,
      notifications: [],
      filteredCount: 0,
      pageIndex: parsed.pageIndex,
      selected: new Set()
    };
    this._readStatus = parsed.readStatus;
    this._filter = parsed.filter;
    this._pageIndex = parsed.pageIndex;
  }

  public render(): JSX.Element {
    if(this.state.redirect) {
      return <Router.Redirect push to={this.state.redirect}/>;
    }
    if(this.state.cannotLoad) {
      return <div/>;
    }
    if(!this.state.isLoaded) {
      return <LoadingPage/>;
    }
    return (
      <NotificationsPage
        displayStatus={this.state.displayStatus}
        notifications={this.state.notifications}
        filteredCount={this.state.filteredCount}
        pageIndex={this.state.pageIndex}
        selected={this.state.selected}
        onSubmit={this.onSubmit}
        onSelectionChange={this.onSelectionChange}
        onMarkAsRead={this.onMarkAsRead}
        onMarkAsUnread={this.onMarkAsUnread}
        onNotificationClick={this.onNotificationClick}/>);
  }

  public async componentDidMount(): Promise<void> {
    try {
      await this.props.model.load();
      this.setState({isLoaded: true});
      await this.loadNotifications();
    } catch {
      this.setState({cannotLoad: true});
    }
  }

  public componentDidUpdate(): void {
    if(this.state.redirect) {
      this.setState({redirect: null});
    }
  }

  private onSubmit = (readStatus: Nexus.Notification.ReadState,
      filter: NotificationsFilter, pageIndex: number) => {
    this._readStatus = readStatus;
    this._filter = filter;
    this._pageIndex = pageIndex;
    this.setState({
      displayStatus: NotificationsPage.DisplayStatus.IN_PROGRESS,
      pageIndex,
      selected: new Set()
    });
    const search = toSearch(readStatus, filter, pageIndex);
    this.props.history.replace({
      pathname: this.props.location.pathname,
      search
    });
    this.loadNotifications(readStatus, filter);
  };

  private onSelectionChange = (selected: Set<Nexus.Notification.Id>) => {
    this.setState({selected});
  };

  private onMarkAsRead = async () => {
    const ids = [...this.state.selected];
    if(ids.length === 0) {
      return;
    }
    try {
      await this.props.model.markAsRead(ids);
      this.setState({selected: new Set()});
      await this.loadNotifications();
    } catch {}
  };

  private onMarkAsUnread = async () => {
    const ids = [...this.state.selected];
    if(ids.length === 0) {
      return;
    }
    try {
      await this.props.model.markAsUnread(ids);
      this.setState({selected: new Set()});
      await this.loadNotifications();
    } catch {}
  };

  private onNotificationClick = (notification: Nexus.Notification) => {
    if(!notification.isRead) {
      this.props.model.markAsRead([notification.id]);
    }
    this.setState({redirect: getNotificationUrl(notification)});
  };

  private async loadNotifications(readStatus?: Nexus.Notification.ReadState,
      filter?: NotificationsFilter): Promise<void> {
    const effectiveReadStatus = readStatus ?? this._readStatus;
    const effectiveFilter = filter ?? this._filter;
    try {
      const allNotifications = await this.props.model.loadNotifications(
        effectiveReadStatus, effectiveFilter);
      const displayStatus = (() => {
        if(this.props.model.totalCount === 0) {
          return NotificationsPage.DisplayStatus.EMPTY;
        }
        if(allNotifications.length === 0) {
          return NotificationsPage.DisplayStatus.NO_RESULTS;
        }
        return NotificationsPage.DisplayStatus.READY;
      })();
      const start = this._pageIndex * NotificationsController.PAGE_SIZE;
      const notifications = allNotifications.slice(
        start, start + NotificationsController.PAGE_SIZE);
      this.setState({
        displayStatus,
        notifications,
        filteredCount: allNotifications.length
      });
    } catch {
      this.setState({
        displayStatus: NotificationsPage.DisplayStatus.ERROR
      });
    }
  }

  private _readStatus: Nexus.Notification.ReadState;
  private _filter: NotificationsFilter;
  private _pageIndex: number;
}

function toSearch(readStatus: Nexus.Notification.ReadState,
    filter: NotificationsFilter, pageIndex: number): string {
  const params = new URLSearchParams();
  if(readStatus !== Nexus.Notification.ReadState.UNREAD) {
    params.set('status', 'all');
  }
  if(filter.query) {
    params.set('q', filter.query);
  }
  if(filter.categories.size > 0) {
    params.set('cat', [...filter.categories].join(','));
  }
  if(filter.startDate) {
    params.set('from', filter.startDate.toJson());
  }
  if(filter.endDate) {
    params.set('to', filter.endDate.toJson());
  }
  if(pageIndex > 0) {
    params.set('page', pageIndex.toString());
  }
  const str = params.toString();
  return str ? `?${str}` : '';
}

function parseSearch(search: string): {
      readStatus: Nexus.Notification.ReadState;
      filter: NotificationsFilter;
      pageIndex: number;
    } {
  const params = new URLSearchParams(search);
  const readStatus = params.get('status') === 'all' ?
    Nexus.Notification.ReadState.ALL : Nexus.Notification.ReadState.UNREAD;
  const query = params.get('q') ?? '';
  const categories = new Set<Nexus.Notification.Category>(
    (params.get('cat') ?? '').split(',').
      map(s => parseInt(s, 10)).
      filter(n => !isNaN(n) &&
        n >= Nexus.Notification.Category.ACCOUNT_MODIFICATION &&
          n <= Nexus.Notification.Category.REPORT));
  const startDate = params.has('from') ?
    Beam.Date.fromJson(params.get('from')) : null;
  const endDate = params.has('to') ?
    Beam.Date.fromJson(params.get('to')) : null;
  const pageIndex = parseInt(params.get('page'), 10) || 0;
  return {
    readStatus,
    filter: {query, categories, startDate, endDate},
    pageIndex
  };
}
