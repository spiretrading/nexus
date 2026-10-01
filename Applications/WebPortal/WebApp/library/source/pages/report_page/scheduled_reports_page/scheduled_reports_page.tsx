import { css, StyleSheet } from 'aphrodite/no-important';
import * as React from 'react';
import { Button, EmptyMessage, ErrorMessage, FilterInput, PageLayout,
  Pagination } from '../../../components';
import { ScheduledReportItem } from './scheduled_report_item';
import { ScheduledReportsModel } from './scheduled_reports_model';
import { ScheduledReportItemPlaceholder } from
  './scheduled_report_item_placeholder';

interface Properties {

  /** The submitted filter criteria. */
  filters: ScheduledReportsModel.Filters;

  /** The requested zero-based page index. */
  pageIndex: number;

  /** The latest response for the submitted criteria and page. */
  response: ScheduledReportsModel.Response;

  /** Called to retrieve a page of matching scheduled reports. */
  onSubmit?: (submission: ScheduledReportsModel.Submission) => void;

  /** Called to navigate to the Create Report Page. */
  onNewReport?: () => void;

  /** Called to run the identified scheduled report immediately. */
  onRun?: (id: string) => void;

  /** Called to duplicate the identified scheduled report. */
  onDuplicate?: (id: string) => void;

  /** Called to delete the identified scheduled report. */
  onDelete?: (id: string) => void;
}

interface State {
  query: string;
  highlight: string;
  response: ScheduledReportsModel.Response;
  displayStatus: DisplayStatus;
}

/** Displays and filters scheduled reports, with loading and fallback states. */
export class ScheduledReportsPage extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {
      query: props.filters.query,
      highlight: props.filters.query,
      response: props.response,
      displayStatus: getDisplayStatus(props.response)
    };
    this.timer = null;
    this.layout = React.createRef();
  }

  public render(): JSX.Element {
    return <PageLayout ref={this.layout}>
      <Main query={this.state.query} onQueryChange={this.onQueryChange}
        onNewReport={this.props.onNewReport}
        displayStatus={this.state.displayStatus}
        response={this.state.response} pageIndex={this.props.pageIndex}
        highlight={this.state.highlight} onRetry={this.onRetry}
        onNavigate={this.onNavigate} onRun={this.props.onRun}
        onDuplicate={this.props.onDuplicate} onDelete={this.props.onDelete}/>
    </PageLayout>;
  }

  public componentDidMount(): void {
    this.updateResponse(false);
  }

  public componentDidUpdate(previous: Properties): void {
    const isNewPage = previous.pageIndex !== this.props.pageIndex &&
      previous.filters.query === this.props.filters.query;
    if(isNewPage) {
      this.layout.current?.scrollToTop();
    }
    if(previous.filters.query !== this.props.filters.query &&
        this.state.query.trim() !== this.props.filters.query) {
      this.setState({query: this.props.filters.query});
    }
    if(previous.response !== this.props.response ||
        previous.pageIndex !== this.props.pageIndex ||
        previous.filters.query !== this.props.filters.query) {
      this.updateResponse(isNewPage);
    }
  }

  public componentWillUnmount(): void {
    window.clearTimeout(this.timer);
  }

  private updateResponse(isNewPage: boolean): void {
    if(this.props.response.status ===
        ScheduledReportsModel.ResponseStatus.IN_PROGRESS) {
      if(isNewPage) {
        window.clearTimeout(this.timer);
        this.timer = null;
        this.setState({displayStatus: null});
      }
      if(this.timer === null && (isNewPage ||
          this.state.displayStatus !== DisplayStatus.IN_PROGRESS)) {
        this.timer = window.setTimeout(this.onLoadingTimeout, 500);
      }
      return;
    }
    window.clearTimeout(this.timer);
    this.timer = null;
    this.setState({
      response: this.props.response,
      highlight: this.props.filters.query,
      displayStatus: getDisplayStatus(this.props.response)
    });
  }

  private onQueryChange = (query: string) => {
    this.setState({query});
    this.props.onSubmit?.({filters: {query: query.trim()}, pageIndex: 0});
  };

  private onNavigate = (pageIndex: number) => {
    if(pageIndex !== this.props.pageIndex && this.props.onSubmit) {
      this.layout.current?.scrollToTop();
      window.clearTimeout(this.timer);
      this.timer = null;
      this.setState({displayStatus: null});
    }
    this.props.onSubmit?.({filters: this.props.filters, pageIndex});
  };

  private onRetry = () => {
    this.props.onSubmit?.({
      filters: this.props.filters, pageIndex: this.props.pageIndex
    });
  };

  private onLoadingTimeout = () => {
    this.timer = null;
    this.setState({displayStatus: DisplayStatus.IN_PROGRESS});
  };

  private timer: number;
  private layout: React.RefObject<PageLayout>;
}

enum DisplayStatus {
  IN_PROGRESS,
  READY,
  NO_RESULTS,
  EMPTY,
  ERROR
}

function getDisplayStatus(response: ScheduledReportsModel.Response):
    DisplayStatus {
  if(response.status === ScheduledReportsModel.ResponseStatus.IN_PROGRESS) {
    return null;
  } else if(response.status === ScheduledReportsModel.ResponseStatus.ERROR) {
    return DisplayStatus.ERROR;
  } else if(response.isEmpty) {
    return DisplayStatus.EMPTY;
  } else if(response.filteredCount === 0) {
    return DisplayStatus.NO_RESULTS;
  }
  return DisplayStatus.READY;
}

interface ContentProperties {
  displayStatus: DisplayStatus;
  response: ScheduledReportsModel.Response;
  pageIndex: number;
  highlight: string;
  onRetry: () => void;
  onNavigate: (pageIndex: number) => void;
  onRun: (id: string) => void;
  onDuplicate: (id: string) => void;
  onDelete: (id: string) => void;
}

interface MainProperties extends ContentProperties {
  query: string;
  onQueryChange: (query: string) => void;
  onNewReport: () => void;
}

function Main(props: MainProperties): JSX.Element {
  return <main className={css(STYLES.main)}>
    <Toolbar query={props.query} onQueryChange={props.onQueryChange}
      onNewReport={props.onNewReport}/>
    <ScheduleContent {...props}/>
  </main>;
}

interface ToolbarProperties {
  query: string;
  onQueryChange: (query: string) => void;
  onNewReport: () => void;
}

class Toolbar extends React.Component<ToolbarProperties> {
  public render(): JSX.Element {
    return <form aria-label='Scheduled Report Controls'
        className={css(STYLES.toolbar)} onSubmit={this.onSubmit}>
      <FilterInput placeholder='Filter schedules' aria-label='Filter schedules'
        value={this.props.query} onChange={this.props.onQueryChange}
        className={css(STYLES.query)}/>
      <div className={css(STYLES.toolbarSpace)}/>
      <Button label='New Report' type='button' onClick={this.onNewReport}
        className={css(STYLES.newReport)}/>
    </form>;
  }

  private onSubmit = (event: React.FormEvent) => {
    event.preventDefault();
  };

  private onNewReport = (event: React.MouseEvent) => {
    event.preventDefault();
    this.props.onNewReport?.();
  };
}

function ScheduleContent(props: ContentProperties): JSX.Element {
  const isFallback = props.displayStatus === DisplayStatus.ERROR ||
    props.displayStatus === DisplayStatus.EMPTY ||
    props.displayStatus === DisplayStatus.NO_RESULTS;
  return <section aria-label='Scheduled Reports' aria-live='polite'
      aria-busy={props.displayStatus === DisplayStatus.IN_PROGRESS}>
    {isFallback && <Fallback displayStatus={props.displayStatus}
      onRetry={props.onRetry}/>}
    {!isFallback && <>
      <ScheduledReportList {...props}/>
      <PaginationBlock {...props}/>
    </>}
  </section>;
}

function Fallback(props: {displayStatus: DisplayStatus; onRetry: () => void}):
    JSX.Element {
  return <div className={css(STYLES.fallback)}>
    {props.displayStatus === DisplayStatus.ERROR &&
      <ErrorMessage message='There was an error loading your scheduled reports.'
        onRetry={props.onRetry}/>}
    {props.displayStatus === DisplayStatus.EMPTY &&
      <EmptyMessage
        message='No scheduled reports. Click New Report to generate one.'/>}
    {props.displayStatus === DisplayStatus.NO_RESULTS &&
      <EmptyMessage message='No results found. Try adjusting filters.'/>}
  </div>;
}

function ScheduledReportList(props: ContentProperties): JSX.Element {
  return <ul className={css(STYLES.list)}>
    {props.displayStatus === DisplayStatus.IN_PROGRESS &&
      Array.from({length: 5}, (_, index) =>
        <li key={index}><ScheduledReportItemPlaceholder/></li>)}
    {props.displayStatus === DisplayStatus.READY &&
      props.response.schedules.map(schedule =>
        <Schedule key={schedule.id} schedule={schedule}
          highlight={props.highlight} onRun={props.onRun}
          onDuplicate={props.onDuplicate} onDelete={props.onDelete}/>)}
  </ul>;
}

interface ScheduleProperties {
  schedule: ScheduledReportsModel.Schedule;
  highlight: string;
  onRun: (id: string) => void;
  onDuplicate: (id: string) => void;
  onDelete: (id: string) => void;
}

class Schedule extends React.Component<ScheduleProperties> {
  public render(): JSX.Element {
    return <li className={css(STYLES.schedule)}>
      <ScheduledReportItem {...this.props.schedule}
        highlight={this.props.highlight} onRun={this.onRun}
        onDuplicate={this.onDuplicate} onDelete={this.onDelete}/>
    </li>;
  }

  private onRun = () => {
    this.props.onRun?.(this.props.schedule.id);
  };

  private onDuplicate = () => {
    this.props.onDuplicate?.(this.props.schedule.id);
  };

  private onDelete = () => {
    this.props.onDelete?.(this.props.schedule.id);
  };
}

function PaginationBlock(props: ContentProperties): JSX.Element {
  if(props.displayStatus !== DisplayStatus.READY ||
      props.response.filteredCount <= ScheduledReportsModel.PAGE_SIZE) {
    return null;
  }
  return <div className={css(STYLES.pagination)}>
    <Pagination pageIndex={props.pageIndex}
      totalCount={props.response.filteredCount}
      pageSize={ScheduledReportsModel.PAGE_SIZE} onNavigate={props.onNavigate}/>
  </div>;
}

const STYLES = StyleSheet.create({
  main: {
    boxSizing: 'border-box',
    backgroundColor: '#FFFFFF',
    fontFamily: 'Roboto, system-ui, sans-serif',
    fontWeight: 400,
    color: '#333333',
    '@media (width < 768px)': {padding: '18px 18px 122px'},
    '@media (min-width: 768px)': {padding: '18px 18px 40px'}
  },
  toolbar: {
    boxSizing: 'border-box',
    display: 'flex',
    alignItems: 'center',
    containerType: 'inline-size',
    padding: '0 18px',
    marginBottom: '30px'
  },
  query: {
    minWidth: 0,
    '@media (width < 768px)': {flex: '1 1 0'},
    '@media (min-width: 768px)': {flex: '0 0 406px'}
  },
  toolbarSpace: {
    minWidth: '18px',
    '@media (width < 768px)': {flex: '0 0 18px'},
    '@media (min-width: 768px)': {flex: '1 1 18px'}
  },
  newReport: {
    whiteSpace: 'nowrap',
    '@media (width < 768px)': {flex: '0 0 auto'},
    '@media (min-width: 768px)': {flex: '0 0 140px'}
  },
  list: {
    margin: 0,
    padding: 0,
    listStyle: 'none',
    containerType: 'inline-size'
  },
  schedule: {
    ':last-child > div > div': {borderBottomColor: 'transparent'}
  },
  fallback: {backgroundColor: '#FFFFFF', padding: '0 18px'},
  pagination: {padding: '30px 18px 0'}
});
