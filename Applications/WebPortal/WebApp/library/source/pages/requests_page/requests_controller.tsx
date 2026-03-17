import * as Beam from 'beam';
import * as Nexus from 'nexus';
import * as React from 'react';
import * as Router from 'react-router-dom';
import { LoadingPage } from '../loading_page';
import { RequestDetailPage } from './request_detail_page';
import { RequestDirectoryPage } from './request_directory_page';
import { RequestsModel } from './requests_model';
import { RequestsPage } from './requests_page';

type Type = Nexus.AccountModificationRequest.Type;
const Type = Nexus.AccountModificationRequest.Type;

interface Properties extends Router.RouteComponentProps {

  /** The account's roles. */
  roles: Nexus.AccountRoles;

  /** The model to use. */
  model: RequestsModel;
}

enum DetailStatus {
  LOADING,
  LOADED,
  ERROR
}

interface State {
  redirect: string;
  displayStatus: RequestDirectoryPage.DisplayStatus;
  requestState: RequestsModel.RequestState;
  filters: RequestsModel.Filters;
  filterCount: number;
  pageIndex: number;
  response: RequestsModel.Response;
  detailStatus: DetailStatus;
  detail: RequestsModel.RequestDetail;
  isSubmitting: boolean;
}

/** Implements a controller for the RequestsPage. */
export class RequestsController extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    const parsed = parseSearch(props.location.search);
    this.state = {
      redirect: null,
      displayStatus: RequestDirectoryPage.DisplayStatus.IN_PROGRESS,
      requestState: parsed.requestState,
      filters: parsed.filters,
      filterCount: computeFilterCount(parsed.filters),
      pageIndex: parsed.pageIndex,
      response: {
        status: RequestsModel.ResponseStatus.IN_PROGRESS,
        facetCounts: {pending: 0, approved: 0, rejected: 0},
        requestList: []
      },
      detailStatus: DetailStatus.LOADING,
      detail: null,
      isSubmitting: false
    };
  }

  public render(): JSX.Element {
    if(this.state.redirect) {
      return <Router.Redirect push to={this.state.redirect}/>;
    }
    const requestId = this.parseRequestId();
    if(requestId !== null) {
      if(this.state.detailStatus === DetailStatus.LOADING) {
        return <LoadingPage/>;
      }
      if(this.state.detailStatus === DetailStatus.ERROR ||
          !this.state.detail) {
        return <div/>;
      }
      const detail = this.state.detail;
      return (
        <div style={DETAIL_STYLE}>
          <RequestDetailPage
            id={detail.id}
            category={detail.category}
            state={detail.state}
            createdTime={detail.createdTime}
            updateTime={detail.updateTime}
            account={detail.account}
            requester={detail.requester}
            effectiveDate={detail.effectiveDate}
            changes={detail.changes}
            activityList={detail.activityList}
            accessRole={detail.accessRole}
            isSubmitting={this.state.isSubmitting}
            onClickAccount={this.onClickAccount}
            onApprove={this.onApprove}
            onReject={this.onReject}/>
        </div>);
    }
    const page = this.currentPage();
    return (
      <RequestsPage
        roles={this.props.roles}
        current={page}
        displayStatus={this.state.displayStatus}
        requestState={this.state.requestState}
        filters={this.state.filters}
        filterCount={this.state.filterCount}
        pageIndex={this.state.pageIndex}
        response={this.state.response}
        onSubmit={this.onSubmit}
        onClickRequest={this.onClickRequest}
        onNavigate={this.onNavigate}/>);
  }

  public async componentDidMount(): Promise<void> {
    await this.props.model.load();
    const requestId = this.parseRequestId();
    if(requestId !== null) {
      await this.loadDetail(requestId);
    } else {
      await this.loadDirectory();
    }
  }

  public componentDidUpdate(prevProps: Properties): void {
    if(this.state.redirect) {
      this.setState({redirect: null});
    }
    if(prevProps.location.pathname !== this.props.location.pathname) {
      const requestId = this.parseRequestId();
      if(requestId !== null) {
        this.loadDetail(requestId);
      } else {
        this.resetAndLoad();
      }
    }
  }

  private currentPage(): RequestsPage.Page {
    if(this.props.location.pathname.endsWith('/group')) {
      return RequestsPage.Page.GROUP_REQUESTS;
    }
    return RequestsPage.Page.YOUR_REQUESTS;
  }

  private onClickRequest = (id: number) => {
    const prefix = this.parseUrlPrefix();
    this.setState({redirect: `${prefix}/${id}`});
  }

  private onClickAccount = (account: Beam.DirectoryEntry) => {
    this.setState({redirect: `/account/${account.id}/profile`});
  }

  private parseRequestId(): number | null {
    const url = this.props.location.pathname;
    const segment = url.substring(url.lastIndexOf('/') + 1);
    const id = parseInt(segment, 10);
    if(isNaN(id)) {
      return null;
    }
    return id;
  }

  private onNavigate = (page: RequestsPage.Page) => {
    const prefix = this.parseUrlPrefix();
    if(page === RequestsPage.Page.YOUR_REQUESTS) {
      this.setState({redirect: `${prefix}/you`});
    } else {
      this.setState({redirect: `${prefix}/group`});
    }
  }

  private parseUrlPrefix(): string {
    const url = this.props.location.pathname;
    const prefix = url.substring(0, url.lastIndexOf('/'));
    if(prefix === '') {
      return url;
    }
    return prefix;
  }

  private async resetAndLoad(): Promise<void> {
    await this.props.model.load();
    const parsed = parseSearch(this.props.location.search);
    this.setState({
      displayStatus: RequestDirectoryPage.DisplayStatus.IN_PROGRESS,
      requestState: parsed.requestState,
      filters: parsed.filters,
      filterCount: computeFilterCount(parsed.filters),
      pageIndex: parsed.pageIndex
    });
    this.loadDirectory({
      scope: this.currentPage() === RequestsPage.Page.YOUR_REQUESTS ?
        RequestsModel.Scope.YOU : RequestsModel.Scope.GROUP,
      requestState: parsed.requestState,
      filters: parsed.filters,
      pageIndex: parsed.pageIndex
    });
  }

  private onSubmit = (submission: RequestsModel.Submission) => {
    this.setState({
      displayStatus: RequestDirectoryPage.DisplayStatus.IN_PROGRESS,
      requestState: submission.requestState,
      filters: submission.filters,
      filterCount: computeFilterCount(submission.filters),
      pageIndex: submission.pageIndex
    });
    const search = submissionToSearch(submission);
    this.props.history.replace({
      pathname: this.props.location.pathname,
      search
    });
    this.loadDirectory(submission);
  }

  private onApprove = async (effectiveDate: Beam.Date, comment: string) => {
    const requestId = this.parseRequestId();
    if(requestId === null) {
      return;
    }
    this.setState({isSubmitting: true});
    try {
      await this.props.model.approve(requestId,
        new Beam.DateTime(effectiveDate), comment);
      await this.loadDetail(requestId);
    } catch {
    } finally {
      this.setState({isSubmitting: false});
    }
  }

  private onReject = async (comment: string) => {
    const requestId = this.parseRequestId();
    if(requestId === null) {
      return;
    }
    this.setState({isSubmitting: true});
    try {
      await this.props.model.reject(requestId, comment);
      await this.loadDetail(requestId);
    } catch {
    } finally {
      this.setState({isSubmitting: false});
    }
  }

  private async loadDetail(id: number): Promise<void> {
    this.setState({detailStatus: DetailStatus.LOADING, detail: null});
    try {
      const detail = await this.props.model.loadRequestDetail(id);
      this.setState({detailStatus: DetailStatus.LOADED, detail});
    } catch {
      this.setState({detailStatus: DetailStatus.ERROR});
    }
  }

  private async loadDirectory(
      submission?: RequestsModel.Submission): Promise<void> {
    const sub = submission ?? {
      scope: this.currentPage() === RequestsPage.Page.YOUR_REQUESTS ?
        RequestsModel.Scope.YOU : RequestsModel.Scope.GROUP,
      requestState: this.state.requestState,
      filters: this.state.filters,
      pageIndex: this.state.pageIndex
    };
    try {
      const response = await this.props.model.loadRequestDirectory(sub);
      this.setState({
        displayStatus: response.requestList.length > 0 ?
          RequestDirectoryPage.DisplayStatus.READY :
          RequestDirectoryPage.DisplayStatus.EMPTY,
        response
      });
    } catch {
      this.setState({
        displayStatus: RequestDirectoryPage.DisplayStatus.ERROR
      });
    }
  }
}

const DETAIL_STYLE: React.CSSProperties = {
  borderTop: '1px solid #E6E6E6',
  height: '100%',
  overflowY: 'auto'
};

const REQUEST_STATE_TO_PARAM: Record<RequestsModel.RequestState, string> = {
  [RequestsModel.RequestState.PENDING]: 'pending',
  [RequestsModel.RequestState.APPROVED]: 'approved',
  [RequestsModel.RequestState.REJECTED]: 'rejected'
};

const PARAM_TO_REQUEST_STATE: Record<string, RequestsModel.RequestState> = {
  'pending': RequestsModel.RequestState.PENDING,
  'approved': RequestsModel.RequestState.APPROVED,
  'rejected': RequestsModel.RequestState.REJECTED
};

const SORT_KEY_TO_PARAM: Record<RequestsModel.SortField, string> = {
  [RequestsModel.SortField.LAST_UPDATED]: 'updated',
  [RequestsModel.SortField.CREATED]: 'created',
  [RequestsModel.SortField.ACCOUNT]: 'account',
  [RequestsModel.SortField.REQUESTER]: 'requester',
  [RequestsModel.SortField.EFFECTIVE_DATE]: 'date'
};

const PARAM_TO_SORT_KEY: Record<string, RequestsModel.SortField> = {
  'updated': RequestsModel.SortField.LAST_UPDATED,
  'created': RequestsModel.SortField.CREATED,
  'account': RequestsModel.SortField.ACCOUNT,
  'requester': RequestsModel.SortField.REQUESTER,
  'date': RequestsModel.SortField.EFFECTIVE_DATE
};

function submissionToSearch(submission: RequestsModel.Submission): string {
  const params = new URLSearchParams();
  if(submission.requestState !== RequestsModel.RequestState.PENDING) {
    params.set('status', REQUEST_STATE_TO_PARAM[submission.requestState]);
  }
  if(submission.filters.query) {
    params.set('q', submission.filters.query);
  }
  if(submission.filters.categories.size > 0) {
    params.set('cat', [...submission.filters.categories].join(','));
  }
  if(submission.filters.sortKey !== RequestsModel.SortField.LAST_UPDATED) {
    params.set('sort', SORT_KEY_TO_PARAM[submission.filters.sortKey]);
  }
  if(submission.filters.startDate) {
    params.set('from', submission.filters.startDate.toJson());
  }
  if(submission.filters.endDate) {
    params.set('to', submission.filters.endDate.toJson());
  }
  if(submission.pageIndex > 0) {
    params.set('page', submission.pageIndex.toString());
  }
  const str = params.toString();
  return str ? `?${str}` : '';
}

function parseSearch(search: string): {
    requestState: RequestsModel.RequestState;
    filters: RequestsModel.Filters;
    pageIndex: number;
  } {
  const params = new URLSearchParams(search);
  const requestState =
    PARAM_TO_REQUEST_STATE[params.get('status')] ??
    RequestsModel.RequestState.PENDING;
  const query = params.get('q') ?? '';
  const categories = new Set<Type>(
    (params.get('cat') ?? '').split(',')
      .map(s => parseInt(s, 10))
      .filter(n => !isNaN(n) && n >= Type.ENTITLEMENTS && n <= Type.COMPLIANCE));
  const sortKey =
    PARAM_TO_SORT_KEY[params.get('sort')] ??
    RequestsModel.SortField.LAST_UPDATED;
  const startDate = params.has('from') ?
    Beam.Date.fromJson(params.get('from')) : undefined;
  const endDate = params.has('to') ?
    Beam.Date.fromJson(params.get('to')) : undefined;
  const pageIndex = parseInt(params.get('page'), 10) || 0;
  return {
    requestState,
    filters: {query, categories, sortKey, startDate, endDate},
    pageIndex
  };
}

function computeFilterCount(filters: RequestsModel.Filters): number {
  let count = filters.categories.size > 0 ? 1 : 0;
  if(filters.query.length > 0) {
    ++count;
  }
  if(filters.startDate || filters.endDate) {
    ++count;
  }
  return count;
}
