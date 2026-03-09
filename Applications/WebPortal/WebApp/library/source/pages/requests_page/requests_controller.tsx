import * as Nexus from 'nexus';
import * as React from 'react';
import * as Router from 'react-router-dom';
import { RequestDirectoryPage } from './request_directory_page';
import { RequestsModel } from './requests_model';
import { RequestsPage } from './requests_page';

type Type = Nexus.AccountModificationRequest.Type;

interface Properties extends Router.RouteComponentProps {

  /** The account's roles. */
  roles: Nexus.AccountRoles;

  /** The model to use. */
  model: RequestsModel;
}

interface State {
  redirect: string;
  displayStatus: RequestDirectoryPage.DisplayStatus;
  requestState: RequestsModel.RequestState;
  filters: RequestsModel.Filters;
  filterCount: number;
  pageIndex: number;
  response: RequestsModel.Response;
}

/** Implements a controller for the RequestsPage. */
export class RequestsController extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {
      redirect: null,
      displayStatus: RequestDirectoryPage.DisplayStatus.IN_PROGRESS,
      requestState: RequestsModel.RequestState.PENDING,
      filters: {
        query: '',
        categories: new Set<Type>(),
        sortKey: RequestsModel.SortField.LAST_UPDATED
      },
      filterCount: 0,
      pageIndex: 0,
      response: {
        status: RequestsModel.ResponseStatus.IN_PROGRESS,
        facetCounts: {pending: 0, approved: 0, rejected: 0},
        requestList: []
      }
    };
  }

  public render(): JSX.Element {
    if(this.state.redirect) {
      return <Router.Redirect push to={this.state.redirect}/>;
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
        onNavigate={this.onNavigate}/>);
  }

  public async componentDidMount(): Promise<void> {
    await this.loadDirectory();
  }

  public componentDidUpdate(prevProps: Properties): void {
    if(this.state.redirect) {
      this.setState({redirect: null});
    }
    if(prevProps.location.pathname !== this.props.location.pathname) {
      const defaultFilters: RequestsModel.Filters = {
        query: '',
        categories: new Set<Type>(),
        sortKey: RequestsModel.SortField.LAST_UPDATED
      };
      this.setState({
        displayStatus: RequestDirectoryPage.DisplayStatus.IN_PROGRESS,
        requestState: RequestsModel.RequestState.PENDING,
        filters: defaultFilters,
        filterCount: 0,
        pageIndex: 0
      });
      this.loadDirectory({
        scope: this.currentPage() === RequestsPage.Page.YOUR_REQUESTS ?
          RequestsModel.Scope.YOU : RequestsModel.Scope.GROUP,
        requestState: RequestsModel.RequestState.PENDING,
        filters: defaultFilters,
        pageIndex: 0
      });
    }
  }

  private currentPage(): RequestsPage.Page {
    if(this.props.location.pathname.endsWith('/group')) {
      return RequestsPage.Page.GROUP_REQUESTS;
    }
    return RequestsPage.Page.YOUR_REQUESTS;
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

  private onSubmit = (submission: RequestsModel.Submission) => {
    this.setState({
      displayStatus: RequestDirectoryPage.DisplayStatus.IN_PROGRESS,
      requestState: submission.requestState,
      filters: submission.filters,
      filterCount: computeFilterCount(submission.filters),
      pageIndex: submission.pageIndex
    });
    this.loadDirectory(submission);
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
