import { css, StyleSheet } from 'aphrodite/no-important';
import * as Nexus from 'nexus';
import * as React from 'react';
import { DisplaySize } from '../../display_size';
import { EmptyMessage } from '../../components/empty_message';
import { ErrorMessage } from '../../components/error_message';
import { FilterChip } from '../../components/filter_chip';
import { FilterInput } from '../../components/filter_input';
import { IconLabelButton } from '../../components/icon_label_button';
import { PageLayout } from '../../components/page_layout';
import { Pagination } from '../../components/pagination';
import { SegmentButton } from '../../components/segment_button';
import { SegmentedControl } from '../../components/segmented_control';
import { RequestFilterModal } from './request_filter_modal';
import { RequestItem } from './request_item';
import { RequestItemPlaceholder } from './request_item_placeholder';
import { RequestsModel } from './requests_model';
import { RequestSortSelect } from './request_sort_select';

type Type = Nexus.AccountModificationRequest.Type;
const Type = Nexus.AccountModificationRequest.Type;

interface Properties {

  /** The scope of the displayed requests. */
  scope: RequestsModel.Scope;

  /** The status of the displayed requests list. */
  displayStatus: RequestDirectoryPage.DisplayStatus;

  /** The currently selected request state facet. */
  requestState: RequestsModel.RequestState;

  /** The user's filter criteria. */
  filters: RequestsModel.Filters;

  /** The number of filters currently active. */
  filterCount: number;

  /** The current 0-based page index. */
  pageIndex: number;

  /** The response from the server. */
  response: RequestsModel.Response;

  /** Called when the user requests to retrieve requests. */
  onSubmit?: (submission: RequestsModel.Submission) => void;

  /** Called when a request entry is clicked. */
  onClickRequest?: (id: number) => void;
}

interface State {
  isFilterModalOpen: boolean;
  filterModalSize: DisplaySize;
}

/** Displays the requests directory page. */
export class RequestDirectoryPage extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {
      isFilterModalOpen: false,
      filterModalSize: DisplaySize.SMALL
    };
  }

  public render(): JSX.Element {
    return (
      <PageLayout>
        <main ref={this.mainRef} className={css(STYLES.main)}>
          <Toolbar
            scope={this.props.scope}
            displayStatus={this.props.displayStatus}
            requestState={this.props.requestState}
            filters={this.props.filters}
            filterCount={this.props.filterCount}
            pageIndex={this.props.pageIndex}
            response={this.props.response}
            isFilterModalOpen={this.state.isFilterModalOpen}
            filterModalSize={this.state.filterModalSize}
            onSubmit={this.props.onSubmit}
            onOpenFilterModal={this.onOpenFilterModal}
            onCloseFilterModal={this.onCloseFilterModal}
            onFilterSubmit={this.onFilterSubmit}/>
          <div className={css(STYLES.contentGap)}/>
          <RequestDirectoryContent
            displayStatus={this.props.displayStatus}
            pageIndex={this.props.pageIndex}
            response={this.props.response}
            onClickRequest={this.props.onClickRequest}
            onPageNavigate={this.onPageNavigate}/>
        </main>
      </PageLayout>);
  }

  private onOpenFilterModal = () => {
    const width = this.mainRef.current?.clientWidth ?? 0;
    const filterModalSize = width >= 1036 ? DisplaySize.LARGE :
      width >= 768 ? DisplaySize.MEDIUM : DisplaySize.SMALL;
    this.setState({isFilterModalOpen: true, filterModalSize});
  }

  private onCloseFilterModal = () => {
    this.setState({isFilterModalOpen: false});
  }

  private onFilterSubmit = (criteria: RequestFilterModal.Criteria) => {
    this.setState({isFilterModalOpen: false});
    this.props.onSubmit?.({
      scope: this.props.scope,
      requestState: this.props.requestState,
      filters: {
        query: this.props.filters.query,
        categories: criteria.categories,
        startDate: criteria.startDate,
        endDate: criteria.endDate,
        sortKey: criteria.sortKey
      },
      pageIndex: 0
    });
  }

  private onPageNavigate = (pageIndex: number) => {
    let element = this.mainRef.current?.parentElement;
    while(element && element.scrollTop === 0 &&
        element !== document.documentElement) {
      element = element.parentElement;
    }
    element?.scrollTo(0, 0);
    this.props.onSubmit?.({
      scope: this.props.scope,
      requestState: this.props.requestState,
      filters: this.props.filters,
      pageIndex
    });
  }

  private mainRef = React.createRef<HTMLElement>();
}

/** Form:Toolbar — contains search, filter chips, segmented control, sort,
 *  and filter button. Has 3 layouts: narrow (default), medium (768-1035px),
 *  and wide (>= 1036px). */
function Toolbar(props: {
    scope: RequestsModel.Scope;
    displayStatus: RequestDirectoryPage.DisplayStatus;
    requestState: RequestsModel.RequestState;
    filters: RequestsModel.Filters;
    filterCount: number;
    pageIndex: number;
    response: RequestsModel.Response;
    isFilterModalOpen: boolean;
    filterModalSize: DisplaySize;
    onSubmit?: (submission: RequestsModel.Submission) => void;
    onOpenFilterModal?: () => void;
    onCloseFilterModal?: () => void;
    onFilterSubmit?: (criteria: RequestFilterModal.Criteria) => void;
  }) {
  const onFormSubmit = (event: React.FormEvent) => {
    event.preventDefault();
  };
  return (
    <form aria-label='Request Filters' className={css(STYLES.toolbar)}
        onSubmit={onFormSubmit}>
      <NarrowToolbar
        scope={props.scope}
        requestState={props.requestState}
        filters={props.filters}
        filterCount={props.filterCount}
        pageIndex={props.pageIndex}
        response={props.response}
        onSubmit={props.onSubmit}
        onOpenFilterModal={props.onOpenFilterModal}/>
      <WideToolbar
        scope={props.scope}
        requestState={props.requestState}
        filters={props.filters}
        filterCount={props.filterCount}
        pageIndex={props.pageIndex}
        response={props.response}
        onSubmit={props.onSubmit}
        onOpenFilterModal={props.onOpenFilterModal}/>
      {props.isFilterModalOpen &&
        <RequestFilterModal
          displaySize={props.filterModalSize}
          categories={props.filters.categories}
          startDate={props.filters.startDate}
          endDate={props.filters.endDate}
          sortKey={props.filters.sortKey}
          onSubmit={props.onFilterSubmit}
          onClose={props.onCloseFilterModal}/>}
    </form>);
}

/** Div:NarrowToolbar — mobile layout with Query + Filters on one row,
 *  RequestStateSegment below. Hidden at >= 768px. */
function NarrowToolbar(props: {
    scope: RequestsModel.Scope;
    requestState: RequestsModel.RequestState;
    filters: RequestsModel.Filters;
    filterCount: number;
    pageIndex: number;
    response: RequestsModel.Response;
    onSubmit?: (submission: RequestsModel.Submission) => void;
    onOpenFilterModal?: () => void;
  }) {
  const filtersLabel = props.filterCount > 0 ?
    `Filters (${props.filterCount})` : 'Filters';
  const onQueryChange = (value: string) => {
    props.onSubmit?.({
      scope: props.scope,
      requestState: props.requestState,
      filters: {...props.filters, query: value},
      pageIndex: props.pageIndex
    });
  };
  const onSelectState = (state: RequestsModel.RequestState) => {
    props.onSubmit?.({
      scope: props.scope,
      requestState: state,
      filters: props.filters,
      pageIndex: props.pageIndex
    });
  };
  return (
    <div className={css(STYLES.narrowToolbar)}>
      <div className={css(STYLES.narrowQueryRow)}>
        <div className={css(STYLES.narrowQueryCell)}>
          <FilterInput value={props.filters.query}
            placeholder='Filter requests'
            onChange={onQueryChange}/>
        </div>
        <div className={css(STYLES.narrowFiltersGap)}/>
        <IconLabelButton
          icon='resources/requests_page/filters.svg'
          label={filtersLabel}
          onClick={props.onOpenFilterModal}/>
      </div>
      <div className={css(STYLES.narrowSegmentGap)}/>
      <RequestStateSegment name='request-state-narrow' showBadge={false}
        requestState={props.requestState}
        response={props.response}
        onSelectState={onSelectState}/>
    </div>);
}

/** Div:WideToolbar — medium/wide layout with QuerySection and ControlsSection
 *  side by side. Hidden below 768px. */
function WideToolbar(props: {
    scope: RequestsModel.Scope;
    requestState: RequestsModel.RequestState;
    filters: RequestsModel.Filters;
    filterCount: number;
    pageIndex: number;
    response: RequestsModel.Response;
    onSubmit?: (submission: RequestsModel.Submission) => void;
    onOpenFilterModal?: () => void;
  }) {
  return (
    <div className={css(STYLES.wideToolbar)}>
      <div className={css(STYLES.wideQueryColumn)}>
        <QuerySection
          scope={props.scope}
          requestState={props.requestState}
          filters={props.filters}
          pageIndex={props.pageIndex}
          onSubmit={props.onSubmit}/>
      </div>
      <div className={css(STYLES.wideFlexColumn)}/>
      <div className={css(STYLES.wideGapColumn)}/>
      <div className={css(STYLES.wideControlsColumn)}>
        <ControlsSection
          scope={props.scope}
          requestState={props.requestState}
          filters={props.filters}
          filterCount={props.filterCount}
          pageIndex={props.pageIndex}
          response={props.response}
          onSubmit={props.onSubmit}
          onOpenFilterModal={props.onOpenFilterModal}/>
      </div>
    </div>);
}

/** Div:QuerySection — Query input + 3 FilterChips.
 *  Used in the wide toolbar at >= 768px. */
function QuerySection(props: {
    scope: RequestsModel.Scope;
    requestState: RequestsModel.RequestState;
    filters: RequestsModel.Filters;
    pageIndex: number;
    onSubmit?: (submission: RequestsModel.Submission) => void;
  }) {
  const onQueryChange = (value: string) => {
    props.onSubmit?.({
      scope: props.scope,
      requestState: props.requestState,
      filters: {...props.filters, query: value},
      pageIndex: props.pageIndex
    });
  };
  const toggleCategory = (type: Type) => {
    const next = new Set(props.filters.categories);
    if(next.has(type)) {
      next.delete(type);
    } else {
      next.add(type);
    }
    props.onSubmit?.({
      scope: props.scope,
      requestState: props.requestState,
      filters: {...props.filters, categories: next},
      pageIndex: props.pageIndex
    });
  };
  return (
    <div className={css(STYLES.querySection)}>
      <FilterInput value={props.filters.query}
        placeholder='Filter requests'
        onChange={onQueryChange}/>
      <div className={css(STYLES.querySectionGap)}/>
      <div className={css(STYLES.chipRow)}>
        <FilterChip label='Risk Controls'
          isChecked={props.filters.categories.has(Type.RISK)}
          onChange={() => toggleCategory(Type.RISK)}/>
        <div className={css(STYLES.chipSpacing)}/>
        <FilterChip label='Entitlements'
          isChecked={props.filters.categories.has(Type.ENTITLEMENTS)}
          onChange={() => toggleCategory(Type.ENTITLEMENTS)}/>
        <div className={css(STYLES.chipSpacing)}/>
        <FilterChip label='Compliance'
          isChecked={props.filters.categories.has(Type.COMPLIANCE)}
          onChange={() => toggleCategory(Type.COMPLIANCE)}/>
      </div>
    </div>);
}

/** Div:ControlsSection — RequestStateSegment + Sort controls + Filters button.
 *  Two layouts: default (300px) and wide (384px at >= 1036px). */
function ControlsSection(props: {
    scope: RequestsModel.Scope;
    requestState: RequestsModel.RequestState;
    filters: RequestsModel.Filters;
    filterCount: number;
    pageIndex: number;
    response: RequestsModel.Response;
    onSubmit?: (submission: RequestsModel.Submission) => void;
    onOpenFilterModal?: () => void;
  }) {
  const filtersLabel = props.filterCount > 0 ?
    `Filters (${props.filterCount})` : 'Filters';
  const onSelectState = (state: RequestsModel.RequestState) => {
    props.onSubmit?.({
      scope: props.scope,
      requestState: state,
      filters: props.filters,
      pageIndex: props.pageIndex
    });
  };
  const onSortChange = (value: RequestsModel.SortField) => {
    props.onSubmit?.({
      scope: props.scope,
      requestState: props.requestState,
      filters: {...props.filters, sortKey: value},
      pageIndex: props.pageIndex
    });
  };
  return (
    <div className={css(STYLES.controlsSection)}>
      <div className={css(STYLES.mediumSegment)}>
        <RequestStateSegment name='request-state-medium' showBadge={false}
          requestState={props.requestState}
          response={props.response}
          onSelectState={onSelectState}/>
      </div>
      <div className={css(STYLES.wideSegment)}>
        <RequestStateSegment name='request-state' showBadge={true}
          requestState={props.requestState}
          response={props.response}
          onSelectState={onSelectState}/>
      </div>
      <div className={css(STYLES.controlsSectionGap)}/>
      <div className={css(STYLES.sortRow)}>
        <label className={css(STYLES.sortLabel)}
          htmlFor='request-sort'>Sort by</label>
        <div className={css(STYLES.sortLabelGap)}/>
        <div className={css(STYLES.sortByCell)}>
          <RequestSortSelect id='request-sort'
            value={props.filters.sortKey}
            onChange={onSortChange}/>
        </div>
        <div className={css(STYLES.sortFiltersGap)}/>
        <div className={css(STYLES.wideFiltersButton)}>
          <IconLabelButton
            aria-label='Filters'
            variant={IconLabelButton.Variant.ICON_LABEL}
            style={{width: '120px'}}
            icon='resources/requests_page/filters.svg'
            label={filtersLabel}
            onClick={props.onOpenFilterModal}/>
        </div>
        <div className={css(STYLES.mediumFiltersButton)}>
          <IconLabelButton
            aria-label='Filters'
            icon='resources/requests_page/filters.svg'
            label='Filters'
            onClick={props.onOpenFilterModal}/>
        </div>
      </div>
    </div>);
}

/** SegmentedControl for Pending/Approved/Rejected state selection. */
function RequestStateSegment(props: {
    name: string;
    showBadge: boolean;
    requestState: RequestsModel.RequestState;
    response: RequestsModel.Response;
    onSelectState?: (state: RequestsModel.RequestState) => void;
  }) {
  const badge = (count: number) => {
    if(props.showBadge &&
        props.response.status === RequestsModel.ResponseStatus.READY) {
      return String(count);
    }
    return undefined;
  };
  return (
    <SegmentedControl name={props.name}>
      <SegmentButton label='Pending'
        badge={badge(props.response.facetCounts.pending)}
        isChecked={props.requestState ===
          RequestsModel.RequestState.PENDING}
        onChange={() =>
          props.onSelectState?.(RequestsModel.RequestState.PENDING)}/>
      <SegmentButton label='Approved'
        badge={badge(props.response.facetCounts.approved)}
        isChecked={props.requestState ===
          RequestsModel.RequestState.APPROVED}
        onChange={() =>
          props.onSelectState?.(RequestsModel.RequestState.APPROVED)}/>
      <SegmentButton label='Rejected'
        badge={badge(props.response.facetCounts.rejected)}
        isChecked={props.requestState ===
          RequestsModel.RequestState.REJECTED}
        onChange={() =>
          props.onSelectState?.(RequestsModel.RequestState.REJECTED)}/>
    </SegmentedControl>);
}

/** Section:RequestDirectoryContent — shows RequestList + PaginationSection,
 *  or Fallback when ERROR/EMPTY. */
function RequestDirectoryContent(props: {
    displayStatus: RequestDirectoryPage.DisplayStatus;
    pageIndex: number;
    response: RequestsModel.Response;
    onClickRequest?: (id: number) => void;
    onPageNavigate?: (pageIndex: number) => void;
  }) {
  const isError = props.displayStatus ===
    RequestDirectoryPage.DisplayStatus.ERROR;
  const isEmpty = props.displayStatus ===
    RequestDirectoryPage.DisplayStatus.EMPTY;
  if(isError || isEmpty) {
    return (
      <Fallback displayStatus={props.displayStatus}/>);
  }
  return (<>
    <RequestList
      displayStatus={props.displayStatus}
      pageIndex={props.pageIndex}
      response={props.response}
      onClickRequest={props.onClickRequest}/>
    <PaginationSection
      displayStatus={props.displayStatus}
      pageIndex={props.pageIndex}
      response={props.response}
      onPageNavigate={props.onPageNavigate}/>
  </>);
}

/** Div:Fallback — shows EmptyMessage or ErrorMessage. */
function Fallback(props: {
    displayStatus: RequestDirectoryPage.DisplayStatus;
  }) {
  return (
    <section aria-label='Requests' aria-live='polite'
        className={css(STYLES.fallbackSection)}>
      <div className={css(STYLES.fallback)}>
        {props.displayStatus ===
          RequestDirectoryPage.DisplayStatus.ERROR &&
          <ErrorMessage message='There was an error loading the requests.'/>}
        {props.displayStatus ===
          RequestDirectoryPage.DisplayStatus.EMPTY &&
          <EmptyMessage
            message='No results found. Try adjusting filters.'/>}
      </div>
    </section>);
}

/** Ul:RequestList — list of RequestItem or RequestItemPlaceholder.
 *  Shows 5 placeholders when IN_PROGRESS, otherwise paginated items. */
function RequestList(props: {
    displayStatus: RequestDirectoryPage.DisplayStatus;
    pageIndex: number;
    response: RequestsModel.Response;
    onClickRequest?: (id: number) => void;
  }) {
  const isLoading = props.displayStatus ===
    RequestDirectoryPage.DisplayStatus.IN_PROGRESS;
  if(isLoading) {
    return (
      <section aria-label='Requests' aria-live='polite' aria-busy='true'>
        <ul className={css(STYLES.requestList)}>
          {[0, 1, 2, 3, 4].map(i =>
            <li key={i}>
              <RequestItemPlaceholder/>
            </li>)}
        </ul>
      </section>);
  }
  const all = props.response.requestList;
  const start = props.pageIndex * PAGE_SIZE;
  const items = all.slice(start, start + PAGE_SIZE);
  return (
    <section aria-label='Requests' aria-live='polite' aria-busy='false'>
      <ul className={css(STYLES.requestList)}>
        {items.map(item =>
          <li key={item.id}>
            <RequestItem
              id={item.id}
              category={item.category}
              state={item.state}
              updateTime={item.updateTime}
              accountName={item.account.name}
              effectiveDate={item.effectiveDate}
              firstChange={item.firstChange}
              additionalChangesCount={item.additionalChangesCount}
              commentCount={item.commentCount}
              managerApproval={item.managerApproval}
              onClick={props.onClickRequest}/>
          </li>)}
      </ul>
    </section>);
}

/** Div:PaginationSection — shows Pagination or nothing.
 *  Hidden when IN_PROGRESS and list size <= 50, or when ERROR/EMPTY. */
function PaginationSection(props: {
    displayStatus: RequestDirectoryPage.DisplayStatus;
    pageIndex: number;
    response: RequestsModel.Response;
    onPageNavigate?: (pageIndex: number) => void;
  }) {
  if(props.displayStatus ===
      RequestDirectoryPage.DisplayStatus.IN_PROGRESS &&
      props.response.requestList.length <= PAGINATION_THRESHOLD) {
    return null;
  }
  return (
    <div className={css(STYLES.paginationSection)}>
      <Pagination
        pageSize={PAGE_SIZE}
        pageIndex={props.pageIndex}
        totalCount={props.response.requestList.length}
        onNavigate={props.onPageNavigate}/>
    </div>);
}

const PAGE_SIZE = 25;
const PAGINATION_THRESHOLD = 50;

export namespace RequestDirectoryPage {

  /** The status of the displayed requests list. */
  export enum DisplayStatus {
    IN_PROGRESS,
    ERROR,
    EMPTY,
    READY
  }
}

const STYLES = StyleSheet.create({
  main: {
    paddingTop: '18px',
    paddingBottom: '40px',
    backgroundColor: '#FFFFFF',
    fontFamily: "'Roboto', system-ui, sans-serif",
    fontWeight: 400,
    color: '#333333',
    display: 'flex',
    flexDirection: 'column',
    flex: '1 1 auto'
  },
  toolbar: {
    padding: '0 18px'
  },
  narrowToolbar: {
    '@media (min-width: 768px)': {
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
    '@media (min-width: 768px)': {
      display: 'flex'
    }
  },
  wideQueryColumn: {
    alignSelf: 'flex-start'
  },
  wideFlexColumn: {
    flex: '1 1 0'
  },
  wideGapColumn: {
    width: '18px',
    flexShrink: 0
  },
  wideControlsColumn: {
    width: '300px',
    flexShrink: 0,
    '@media (min-width: 1036px)': {
      width: '384px'
    }
  },
  querySection: {
    display: 'flex',
    flexDirection: 'column'
  },
  querySectionGap: {
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
  controlsSection: {
    display: 'flex',
    flexDirection: 'column'
  },
  controlsSectionGap: {
    height: '12px'
  },
  sortRow: {
    display: 'flex',
    alignItems: 'center'
  },
  sortLabel: {
    fontFamily: 'inherit',
    fontSize: '0.875rem',
    fontWeight: 'inherit',
    whiteSpace: 'nowrap'
  },
  sortLabelGap: {
    width: '8px',
    flexShrink: 0
  },
  sortByCell: {
    flex: '1 1 0'
  },
  sortFiltersGap: {
    width: '18px',
    flexShrink: 0
  },
  wideFiltersButton: {
    display: 'none',
    '@media (min-width: 1036px)': {
      display: 'block'
    }
  },
  mediumSegment: {
    '@media (min-width: 1036px)': {
      display: 'none'
    }
  },
  wideSegment: {
    display: 'none',
    '@media (min-width: 1036px)': {
      display: 'block'
    }
  },
  mediumFiltersButton: {
    '@media (min-width: 1036px)': {
      display: 'none'
    }
  },
  contentGap: {
    height: '30px'
  },
  fallbackSection: {
    display: 'flex',
    flexDirection: 'column',
    flex: '1 1 auto'
  },
  fallback: {
    backgroundColor: '#FFFFFF',
    padding: '0 18px',
    flex: '1 1 auto'
  },
  requestList: {
    padding: 0,
    margin: 0,
    listStyle: 'none'
  },
  paginationSection: {
    padding: '30px 18px 0'
  }
});
