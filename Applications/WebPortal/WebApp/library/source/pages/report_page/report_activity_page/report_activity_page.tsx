import { css, StyleSheet } from 'aphrodite/no-important';
import * as React from 'react';
import { Button, EmptyMessage, ErrorMessage, PageLayout, Pagination,
  SortableTableHeaderCell } from '../../../components';
import { ActivityTable } from './activity_table';
import { ReportActivityModel } from './report_activity_model';
import { ReportActivityStatusTag } from './report_activity_status_tag';

interface Properties extends ReportActivityModel.Submission {

  /** The response for the current ordering and page. */
  response: ReportActivityModel.Response;

  /** Requests a page of report activity. */
  onSubmit?: (submission: ReportActivityModel.Submission) => void;

  /** Navigates to the Create Report Page. */
  onNewReport?: () => void;

  /** Cancels the identified report jobs. */
  onCancel?: (ids: readonly string[]) => void;

  /** Retries the identified failed report jobs. */
  onRetry?: (ids: readonly string[]) => void;
}

interface State {
  selected: ReadonlySet<string>;
  response: ReportActivityModel.Response;
  displayStatus: DisplayStatus;
}

/** Displays report jobs with sorting, pagination, cancellation, and retry. */
export class ReportActivityPage extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {selected: new Set(), response: props.response,
      displayStatus: getDisplayStatus(props.response)};
    this.timer = null;
    this.layout = React.createRef();
  }

  public render(): JSX.Element {
    const actions = {
      disabled: this.state.selected.size === 0 || this.props.response.status !==
        ReportActivityModel.ResponseStatus.READY,
      retryEnabled: this.isRetryEnabled(),
      onCancel: this.onCancel, onRetry: this.onRetry};
    return <PageLayout ref={this.layout}>
      <Main actions={actions} onNewReport={this.props.onNewReport}
        response={this.state.response} displayStatus={this.state.displayStatus}
        pageIndex={this.props.pageIndex} selected={this.state.selected}
        sort={this.props.sort} onSelectionChange={this.onSelectionChange}
        onSort={this.onSort} onNavigate={this.onNavigate}
        onReload={this.onReload}/>
    </PageLayout>;
  }

  public componentDidMount(): void {
    this.updateResponse(false);
  }

  public componentDidUpdate(previous: Properties): void {
    if(previous.response !== this.props.response ||
        previous.pageIndex !== this.props.pageIndex) {
      const newPage = previous.pageIndex !== this.props.pageIndex;
      if(newPage) {
        this.layout.current?.scrollToTop();
      }
      this.updateResponse(newPage);
    }
  }

  public componentWillUnmount(): void {
    window.clearTimeout(this.timer);
  }

  private updateResponse(newPage: boolean): void {
    if(this.props.response.status ===
        ReportActivityModel.ResponseStatus.IN_PROGRESS) {
      if(newPage) {
        window.clearTimeout(this.timer);
        this.timer = null;
        this.setState({displayStatus: null});
      }
      if(this.timer === null &&
          this.state.displayStatus !== DisplayStatus.IN_PROGRESS) {
        const LOADING_DELAY = 500;
        this.timer = window.setTimeout(this.onLoadingTimeout, LOADING_DELAY);
      }
      return;
    }
    window.clearTimeout(this.timer);
    this.timer = null;
    const response = this.props.response;
    this.setState(state => ({response,
      selected: (() => {
        if(response.status === ReportActivityModel.ResponseStatus.READY &&
            !response.isEmpty) {
          return state.selected;
        }
        return new Set<string>();
      })(), displayStatus: getDisplayStatus(response)}));
  }

  private isRetryEnabled(): boolean {
    return this.state.selected.size > 0 && this.props.response.status ===
      ReportActivityModel.ResponseStatus.READY &&
      [...this.state.selected].every(id => this.state.response.activities.
        some(entry => entry.id === id &&
          entry.status === ReportActivityStatusTag.Status.FAILED));
  }

  private submit(submission: ReportActivityModel.Submission): void {
    this.setState({selected: new Set()});
    this.props.onSubmit?.(submission);
  }

  private onSort = (column: ActivityTable.Column,
      order: SortableTableHeaderCell.SortOrder) => {
    this.submit({sort: {column, order}, pageIndex: 0});
  };

  private onNavigate = (pageIndex: number) => {
    if(pageIndex !== this.props.pageIndex && this.props.onSubmit) {
      this.layout.current?.scrollToTop();
      window.clearTimeout(this.timer);
      this.timer = null;
      this.setState({displayStatus: null});
    }
    this.submit({sort: this.props.sort, pageIndex});
  };

  private onReload = () => {
    this.submit({sort: this.props.sort, pageIndex: this.props.pageIndex});
  };

  private onSelectionChange = (selected: ReadonlySet<string>) => {
    this.setState({selected});
  };

  private onCancel = () => {
    if(this.state.selected.size > 0 && this.props.response.status ===
        ReportActivityModel.ResponseStatus.READY) {
      const ids = [...this.state.selected];
      this.setState({selected: new Set()});
      this.props.onCancel?.(ids);
    }
  };

  private onRetry = () => {
    if(this.isRetryEnabled()) {
      const ids = [...this.state.selected];
      this.setState({selected: new Set()});
      this.props.onRetry?.(ids);
    }
  };

  private onLoadingTimeout = () => {
    this.timer = null;
    this.setState({displayStatus: DisplayStatus.IN_PROGRESS,
      selected: new Set()});
  };

  private timer: number;
  private layout: React.RefObject<PageLayout>;
}

enum DisplayStatus {
  IN_PROGRESS,
  READY,
  EMPTY,
  ERROR
}

function getDisplayStatus(response: ReportActivityModel.Response):
    DisplayStatus {
  if(response.status === ReportActivityModel.ResponseStatus.IN_PROGRESS) {
    return null;
  } else if(response.status === ReportActivityModel.ResponseStatus.ERROR) {
    return DisplayStatus.ERROR;
  } else if(response.isEmpty) {
    return DisplayStatus.EMPTY;
  }
  return DisplayStatus.READY;
}

interface ActionProperties {
  disabled: boolean;
  retryEnabled: boolean;
  onCancel: () => void;
  onRetry: () => void;
}

interface ContentProperties {
  response: ReportActivityModel.Response;
  displayStatus: DisplayStatus;
  pageIndex: number;
  selected: ReadonlySet<string>;
  sort: ReportActivityModel.Sort;
  actions: ActionProperties;
  onSelectionChange: (selected: ReadonlySet<string>) => void;
  onSort: (column: ActivityTable.Column,
    order: SortableTableHeaderCell.SortOrder) => void;
  onNavigate: (pageIndex: number) => void;
  onReload: () => void;
}

interface ToolbarProperties {
  actions: ActionProperties;
  onNewReport: () => void;
}

function Main(props: ToolbarProperties & ContentProperties): JSX.Element {
  return <main className={css(STYLES.main)}>
    <Toolbar {...props}/>
    <ReportActivityContent {...props}/>
  </main>;
}

class Toolbar extends React.Component<ToolbarProperties> {
  public render(): JSX.Element {
    return <form aria-label='Report Activity Actions'
        className={css(STYLES.toolbar)} onSubmit={this.onSubmit}>
      <div className={css(STYLES.toolbarActions)}>
        <Button type='button' label='Cancel' variant={Button.Variant.SECONDARY}
          disabled={this.props.actions.disabled}
          onClick={this.props.actions.onCancel} style={{width: '100px'}}/>
        <Button type='button' label='Retry' variant={Button.Variant.SECONDARY}
          disabled={!this.props.actions.retryEnabled}
          onClick={this.props.actions.onRetry} style={{width: '100px'}}/>
      </div>
      <div className={css(STYLES.newReport)}>
        <Button label='New Report' type='button' style={{width: '100%'}}
          onClick={this.props.onNewReport}/>
      </div>
    </form>;
  }

  private onSubmit = (event: React.FormEvent) => {
    event.preventDefault();
  };
}

function ButtonGroup(props: ActionProperties): JSX.Element {
  return <div className={css(STYLES.buttons)}>
    <Button type='button' label='Cancel' variant={Button.Variant.SECONDARY}
      disabled={props.disabled} onClick={props.onCancel}
      style={{flex: '1 1 0', minWidth: 0}}/>
    {props.retryEnabled &&
      <Button type='button' label='Retry' variant={Button.Variant.SECONDARY}
        onClick={props.onRetry} style={{flex: '1 1 0', minWidth: 0}}/>}
  </div>;
}

function ActionSheet(props: ActionProperties): JSX.Element {
  if(props.disabled) {
    return null;
  }
  return <section aria-label='Report Activity Actions'
      className={css(STYLES.actionSheet)}>
    <ButtonGroup {...props}/>
  </section>;
}

function ReportActivityContent(props: ContentProperties): JSX.Element {
  return <section aria-label='Report Jobs' aria-live='polite'
      aria-busy={props.displayStatus === DisplayStatus.IN_PROGRESS}>
    {props.displayStatus === DisplayStatus.ERROR &&
      <ErrorMessage
        message='There was an error loading your report activity.'
        onRetry={props.onReload}/>}
    {props.displayStatus === DisplayStatus.EMPTY &&
      <EmptyMessage message='No report activity.'/>}
    {(props.displayStatus === DisplayStatus.READY ||
        props.displayStatus === DisplayStatus.IN_PROGRESS) &&
      <ReportJobs {...props}/>}
  </section>;
}

function ReportJobs(props: ContentProperties): JSX.Element {
  return <div className={css(STYLES.jobs)}>
    <ActivityTable activities={props.response.activities}
      selected={props.selected} className={css(STYLES.activityTable)}
      sortColumn={props.sort.column} sortOrder={props.sort.order}
      loading={props.displayStatus === DisplayStatus.IN_PROGRESS}
      onSelectionChange={props.onSelectionChange} onSort={props.onSort}/>
    <PaginationBlock {...props}/>
    <ActionSheet {...props.actions}/>
  </div>;
}

function PaginationBlock(props: ContentProperties): JSX.Element {
  if(props.displayStatus !== DisplayStatus.READY ||
      props.response.totalCount <= ReportActivityModel.PAGE_SIZE) {
    return null;
  }
  return <div style={{paddingTop: '30px'}}>
    <Pagination pageIndex={props.pageIndex}
      pageSize={ReportActivityModel.PAGE_SIZE}
      totalCount={props.response.totalCount} onNavigate={props.onNavigate}/>
  </div>;
}

const STYLES = StyleSheet.create({
  main: {
    boxSizing: 'border-box',
    backgroundColor: '#FFFFFF',
    fontFamily: 'Roboto, system-ui, sans-serif',
    fontSize: '0.875rem',
    fontWeight: 400,
    color: '#333333',
    '@media (width < 768px)': {padding: '18px 18px 122px'},
    '@media (min-width: 768px)': {padding: '18px 18px 40px'}
  },
  toolbar: {
    display: 'flex',
    alignItems: 'center',
    justifyContent: 'space-between',
    containerType: 'inline-size',
    '@media (width < 768px)': {marginBottom: '30px'},
    '@media (min-width: 768px)': {marginBottom: '18px'}
  },
  toolbarActions: {
    gap: '8px',
    '@media (width < 768px)': {display: 'none'},
    '@media (min-width: 768px)': {display: 'flex'}
  },
  newReport: {
    marginLeft: 'auto',
    '@media (min-width: 768px)': {width: '140px'}
  },
  buttons: {display: 'flex', gap: '8px', maxWidth: '424px'},
  jobs: {position: 'relative'},
  activityTable: {'@media (min-width: 768px)': {maxWidth: '796px'}},
  actionSheet: {
    position: 'fixed',
    bottom: 0,
    left: 0,
    width: '100%',
    boxSizing: 'border-box',
    padding: '18px 18px 30px',
    backgroundColor: '#FFFFFF',
    boxShadow: '0 0 6px rgb(0 0 0 / 25%)',
    zIndex: 1,
    '@media (min-width: 768px)': {display: 'none'}
  }
});
