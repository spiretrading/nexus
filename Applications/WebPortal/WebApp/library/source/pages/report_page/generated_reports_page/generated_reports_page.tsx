import { css, StyleSheet } from 'aphrodite/no-important';
import * as Beam from 'beam';
import * as React from 'react';
import { AccountGroupQueryModel, Button, EmptyMessage, ErrorMessage,
  FilterInput, PageLayout, Pagination, SortableTableHeaderCell } from
  '../../../components';
import { DateRange } from '../../../models/date_range';
import { ShareReportModal } from '../share_report_modal';
import { DateFilter } from './date_filter';
import { GeneratedReportsModel } from './generated_reports_model';
import { ReportTable } from './report_table';

interface Properties extends GeneratedReportsModel.Submission {

  /** The response for the current criteria and page. */
  response: GeneratedReportsModel.Response;

  /** The account and group lookup model for sharing. */
  recipientModel: AccountGroupQueryModel;

  /** Requests a page of reports for the supplied criteria. */
  onSubmit?: (submission: GeneratedReportsModel.Submission) => void;

  /** Navigates to the Create Report Page. */
  onNewReport?: () => void;

  /** Deletes the identified reports. */
  onDelete?: (ids: readonly string[]) => void;

  /** Shares reports with the selected accounts and groups. */
  onShare?: (ids: readonly string[],
    recipients: readonly Beam.DirectoryEntry[]) => void;

  /** Downloads the identified reports. */
  onDownload?: (ids: readonly string[]) => void;
}

interface State {
  query: string;
  selected: ReadonlySet<string>;
  sharing: readonly string[];
  response: GeneratedReportsModel.Response;
  highlight: string;
  displayStatus: DisplayStatus;
}

/** Displays generated reports with filtering, sorting, and bulk actions. */
export class GeneratedReportsPage extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {query: props.filters.query, selected: new Set(),
      sharing: null,
      response: props.response, highlight: props.filters.query,
      displayStatus: getDisplayStatus(props.response)};
    this.timer = null;
    this.layout = React.createRef();
    this.recipients = [];
  }

  public render(): JSX.Element {
    const actions = {disabled: this.state.selected.size === 0,
      onDelete: this.onDelete, onShare: this.onOpenShare,
      onDownload: this.onDownload};
    return <>
      <PageLayout ref={this.layout}>
        <Main query={this.state.query} dateRange={this.props.filters.dateRange}
          onQueryChange={this.onQueryChange} onDateChange={this.onDateChange}
          onNewReport={this.props.onNewReport} actions={actions}
          response={this.state.response}
          displayStatus={this.state.displayStatus}
          pageIndex={this.props.pageIndex} selected={this.state.selected}
          sort={this.props.sort} highlight={this.state.highlight}
          onSelectionChange={this.onSelectionChange} onSort={this.onSort}
          onNavigate={this.onNavigate} onRetry={this.onRetry}/>
      </PageLayout>
      {this.state.sharing && <ShareReportModal
        model={this.props.recipientModel} selected={this.recipients}
        onSubmit={this.onShare} onClose={this.onCloseShare}/>}
    </>;
  }

  public componentDidMount(): void {
    this.updateResponse(false);
  }

  public componentDidUpdate(previous: Properties): void {
    if(previous.filters.query !== this.props.filters.query &&
        this.state.query.trim() !== this.props.filters.query) {
      this.setState({query: this.props.filters.query});
    }
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
        GeneratedReportsModel.ResponseStatus.IN_PROGRESS) {
      if(newPage) {
        window.clearTimeout(this.timer);
        this.timer = null;
        this.setState({displayStatus: null});
      }
      if(this.timer === null &&
          this.state.displayStatus !== DisplayStatus.IN_PROGRESS) {
        this.timer = window.setTimeout(this.onLoadingTimeout, 500);
      }
      return;
    }
    window.clearTimeout(this.timer);
    this.timer = null;
    this.setState({response: this.props.response,
      highlight: this.props.filters.query, selected: new Set(),
      displayStatus: getDisplayStatus(this.props.response)});
  }

  private submit(submission: GeneratedReportsModel.Submission): void {
    this.setState({selected: new Set(), sharing: null});
    this.props.onSubmit?.(submission);
  }

  private onQueryChange = (query: string) => {
    this.setState({query});
    this.submit({filters: {...this.props.filters, query: query.trim()},
      sort: this.props.sort, pageIndex: 0});
  };

  private onDateChange = (dateRange: DateRange) => {
    this.submit({filters: {...this.props.filters, dateRange},
      sort: this.props.sort, pageIndex: 0});
  };

  private onSort = (column: ReportTable.Column,
      order: SortableTableHeaderCell.SortOrder) => {
    this.submit({filters: this.props.filters, sort: {column, order},
      pageIndex: 0});
  };

  private onNavigate = (pageIndex: number) => {
    if(pageIndex !== this.props.pageIndex && this.props.onSubmit) {
      this.layout.current?.scrollToTop();
      window.clearTimeout(this.timer);
      this.timer = null;
      this.setState({displayStatus: null});
    }
    this.submit({filters: this.props.filters, sort: this.props.sort,
      pageIndex});
  };

  private onRetry = () => {
    this.submit({filters: this.props.filters, sort: this.props.sort,
      pageIndex: this.props.pageIndex});
  };

  private onSelectionChange = (selected: ReadonlySet<string>) => {
    this.setState({selected});
  };

  private onDelete = () => {
    const ids = [...this.state.selected];
    this.setState({selected: new Set(), sharing: null});
    if(ids.length > 0) {
      this.props.onDelete?.(ids);
    }
  };

  private onOpenShare = () => {
    if(this.state.selected.size > 0) {
      this.setState({sharing: [...this.state.selected]});
    }
  };

  private onCloseShare = () => {
    this.setState({sharing: null});
  };

  private onShare = (recipients: Beam.DirectoryEntry[]) => {
    const ids = this.state.sharing;
    this.setState({sharing: null});
    if(ids) {
      this.props.onShare?.(ids, recipients);
    }
  };

  private onDownload = () => {
    if(this.state.selected.size > 0) {
      this.props.onDownload?.([...this.state.selected]);
    }
  };

  private onLoadingTimeout = () => {
    this.timer = null;
    this.setState({displayStatus: DisplayStatus.IN_PROGRESS,
      selected: new Set()});
  };

  private timer: number;
  private layout: React.RefObject<PageLayout>;
  private recipients: readonly Beam.DirectoryEntry[];
}

enum DisplayStatus {
  IN_PROGRESS,
  READY,
  NO_RESULTS,
  EMPTY,
  ERROR
}

function getDisplayStatus(response: GeneratedReportsModel.Response):
    DisplayStatus {
  if(response.status === GeneratedReportsModel.ResponseStatus.IN_PROGRESS) {
    return null;
  } else if(response.status === GeneratedReportsModel.ResponseStatus.ERROR) {
    return DisplayStatus.ERROR;
  } else if(response.isEmpty) {
    return DisplayStatus.EMPTY;
  } else if(response.filteredCount === 0) {
    return DisplayStatus.NO_RESULTS;
  }
  return DisplayStatus.READY;
}

interface ActionProperties {
  disabled: boolean;
  onDelete: () => void;
  onShare: () => void;
  onDownload: () => void;
}

interface ContentProperties {
  response: GeneratedReportsModel.Response;
  displayStatus: DisplayStatus;
  pageIndex: number;
  selected: ReadonlySet<string>;
  sort: GeneratedReportsModel.Sort;
  highlight: string;
  actions: ActionProperties;
  onSelectionChange: (selected: ReadonlySet<string>) => void;
  onSort: (column: ReportTable.Column,
    order: SortableTableHeaderCell.SortOrder) => void;
  onNavigate: (pageIndex: number) => void;
  onRetry: () => void;
}

interface ToolbarProperties {
  query: string;
  dateRange: DateRange;
  actions: ActionProperties;
  onQueryChange: (query: string) => void;
  onDateChange: (dateRange: DateRange) => void;
  onNewReport: () => void;
}

function Main(props: ToolbarProperties & ContentProperties): JSX.Element {
  return <main className={css(STYLES.main)}>
    <Toolbar {...props}/>
    <ReportContent {...props}/>
  </main>;
}

class Toolbar extends React.Component<ToolbarProperties> {
  public render(): JSX.Element {
    return <form aria-label='Report Controls' className={css(STYLES.toolbar)}
        onSubmit={this.onSubmit}>
      <div className={css(STYLES.query)}>
        <FilterInput placeholder='Filter reports' aria-label='Filter reports'
          value={this.props.query} onChange={this.props.onQueryChange}
          style={{width: '100%', minWidth: 0}}/>
      </div>
      <div className={css(STYLES.newReport)}>
        <Button label='New Report' type='button'
          style={{width: '100%'}} onClick={this.props.onNewReport}/>
      </div>
      <div className={css(STYLES.dateFilter)}>
        <DateFilter value={this.props.dateRange}
          className={css(STYLES.dateFilterInput)}
          onChange={this.props.onDateChange}/>
      </div>
      <div className={css(STYLES.toolbarActions)}>
        <ButtonGroup {...this.props.actions}/>
      </div>
    </form>;
  }

  private onSubmit = (event: React.FormEvent) => {
    event.preventDefault();
  };
}

function ButtonGroup(props: ActionProperties): JSX.Element {
  return <div className={css(STYLES.buttons)}>
    <Button type='button' label='Delete' variant={Button.Variant.SECONDARY}
      disabled={props.disabled} onClick={props.onDelete}
      style={{flex: '1 1 0', minWidth: 0}}/>
    <Button type='button' label='Share' variant={Button.Variant.SECONDARY}
      disabled={props.disabled} onClick={props.onShare}
      style={{flex: '1 1 0', minWidth: 0}}/>
    <Button type='button' label='Download' variant={Button.Variant.SECONDARY}
      disabled={props.disabled} onClick={props.onDownload}
      style={{flex: '1 1 0', minWidth: 0}}/>
  </div>;
}

function ActionSheet(props: ActionProperties): JSX.Element {
  if(props.disabled) {
    return null;
  }
  return <section aria-label='Selected Report Actions'
      className={css(STYLES.actionSheet)}>
    <ButtonGroup {...props}/>
  </section>;
}

function ReportContent(props: ContentProperties): JSX.Element {
  return <section aria-label='Generated Reports' aria-live='polite'
      aria-busy={props.displayStatus === DisplayStatus.IN_PROGRESS}>
    {props.displayStatus === DisplayStatus.ERROR &&
      <ErrorMessage message='There was an error loading your reports.'
        onRetry={props.onRetry}/>}
    {props.displayStatus === DisplayStatus.EMPTY &&
      <EmptyMessage message='No reports. Click New Report to generate one.'/>}
    {props.displayStatus === DisplayStatus.NO_RESULTS &&
      <EmptyMessage message='No results found. Try adjusting filters.'/>}
    {(props.displayStatus === DisplayStatus.READY ||
        props.displayStatus === DisplayStatus.IN_PROGRESS) &&
      <Reports {...props}/>}
  </section>;
}

function Reports(props: ContentProperties): JSX.Element {
  return <div className={css(STYLES.reports)}>
    <ReportTable reports={props.response.reports} selected={props.selected}
      className={css(STYLES.reportTable)}
      highlight={props.highlight} sortColumn={props.sort.column}
      sortOrder={props.sort.order}
      loading={props.displayStatus === DisplayStatus.IN_PROGRESS}
      onSelectionChange={props.onSelectionChange} onSort={props.onSort}/>
    <PaginationBlock {...props}/>
    <ActionSheet {...props.actions}/>
  </div>;
}

function PaginationBlock(props: ContentProperties): JSX.Element {
  if(props.displayStatus !== DisplayStatus.READY ||
      props.response.filteredCount <= GeneratedReportsModel.PAGE_SIZE) {
    return null;
  }
  return <div style={{paddingTop: '30px'}}>
    <Pagination pageIndex={props.pageIndex}
      pageSize={GeneratedReportsModel.PAGE_SIZE}
      totalCount={props.response.filteredCount} onNavigate={props.onNavigate}/>
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
    display: 'grid',
    containerType: 'inline-size',
    '@media (width < 768px)': {
      gridTemplateColumns: 'minmax(0, 1fr) auto',
      columnGap: '18px',
      marginBottom: '30px'
    },
    '@media (min-width: 768px)': {
      gridTemplateColumns: 'minmax(0, 1fr) 140px',
      marginBottom: '18px'
    }
  },
  query: {
    minWidth: 0,
    gridColumn: 1,
    gridRow: 1,
    '@media (768px <= width < 1036px)': {width: '246px'},
    '@media (min-width: 1036px)': {width: '406px'}
  },
  newReport: {
    gridColumn: 2,
    gridRow: 1
  },
  dateFilter: {
    gridRow: 2,
    minHeight: '34px',
    marginTop: '12px',
    minWidth: 0,
    '@media (width < 768px)': {gridColumn: '1 / -1'},
    '@media (768px <= width < 1036px)': {gridColumn: 1, width: '246px'},
    '@media (min-width: 1036px)': {
      gridColumn: 1,
      width: 'fit-content'
    }
  },
  dateFilterInput: {
    '@media (min-width: 1036px)': {
      ':is(div)': {width: 'max-content', containerType: 'normal'}
    }
  },
  toolbarActions: {
    gridColumn: '1 / -1',
    gridRow: 3,
    width: '316px',
    marginTop: '18px',
    '@media (width < 768px)': {display: 'none'}
  },
  buttons: {display: 'flex', gap: '8px', maxWidth: '424px', margin: '0 auto'},
  reports: {position: 'relative'},
  reportTable: {'@media (min-width: 768px)': {maxWidth: '696px'}},
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
