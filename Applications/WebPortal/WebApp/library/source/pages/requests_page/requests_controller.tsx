import * as Nexus from 'nexus';
import * as React from 'react';
import { RequestDirectoryPage } from './request_directory_page';
import { RequestsModel } from './requests_model';
import { RequestsPage } from './requests_page';

type Type = Nexus.AccountModificationRequest.Type;

interface Properties {

  /** The account's roles. */
  roles: Nexus.AccountRoles;

  /** The model to use. */
  model: RequestsModel;
}

interface State {
  page: RequestsPage.Page;
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
      page: RequestsPage.Page.YOUR_REQUESTS,
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
    return (
      <RequestsPage
        roles={this.props.roles}
        current={this.state.page}
        displayStatus={this.state.displayStatus}
        requestState={this.state.requestState}
        filters={this.state.filters}
        filterCount={this.state.filterCount}
        pageIndex={this.state.pageIndex}
        response={this.state.response}
        onSubmit={this.onSubmit}/>);
  }

  public async componentDidMount(): Promise<void> {
    await this.loadDirectory();
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
      scope: this.state.page === RequestsPage.Page.YOUR_REQUESTS ?
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
