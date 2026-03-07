import { css, StyleSheet } from 'aphrodite/no-important';
import * as Beam from 'beam';
import * as Nexus from 'nexus';
import * as React from 'react';
import { EmptyMessage } from '../../components/empty_message';
import { ErrorMessage } from '../../components/error_message';
import { FilterChip } from '../../components/filter_chip';
import { FilterInput } from '../../components/filter_input';
import { IconLabelButton } from '../../components/icon_label_button';
import { DisplaySize } from '../../display_size';
import { PageLayout } from '../../components/page_layout';
import { Pagination } from '../../components/pagination';
import { SegmentButton } from '../../components/segment_button';
import { SegmentedControl } from '../../components/segmented_control';
import { RequestFilterModal } from './request_filter_modal';
import { RequestItem } from './request_item';
import { RequestItemPlaceholder } from './request_item_placeholder';
import { RequestSortSelect } from './request_sort_select';

type Type = Nexus.AccountModificationRequest.Type;
const Type = Nexus.AccountModificationRequest.Type;

interface Properties {

  /** The scope of the displayed requests. */
  scope: RequestDirectoryPage.Scope;

  /** The status of the displayed requests list. */
  displayStatus: RequestDirectoryPage.DisplayStatus;

  /** The currently selected request state facet. */
  requestState: RequestDirectoryPage.RequestState;

  /** The user's filter criteria. */
  filters: RequestDirectoryPage.Filters;

  /** The number of filters currently active. */
  filterCount: number;

  /** The current 0-based page index. */
  pageIndex: number;

  /** The response from the server. */
  response: RequestDirectoryPage.Response;

  /** Called when the user requests to retrieve requests. */
  onSubmit?: (submission: RequestDirectoryPage.Submission) => void;
}

interface State {
  query: string;
  categories: Set<Type>;
  requestState: RequestDirectoryPage.RequestState;
  sortKey: RequestSortSelect.Field;
  pageIndex: number;
  isFilterModalOpen: boolean;
}

/** Displays the requests directory page. */
export class RequestDirectoryPage extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {
      query: props.filters.query,
      categories: new Set(props.filters.categories),
      requestState: props.requestState,
      sortKey: props.filters.sortKey,
      pageIndex: props.pageIndex,
      isFilterModalOpen: false
    };
  }

  public render(): JSX.Element {
    return (
      <PageLayout>
        <div className={css(STYLES.main)}>
          {this.renderToolbar()}
          {this.renderRequestContent()}
          {this.renderPaginationSection()}
        </div>
      </PageLayout>);
  }

  private renderToolbar(): JSX.Element {
    return (
      <form aria-label='Request Filters'
          className={css(STYLES.toolbar)}
          onSubmit={this.onFormSubmit}>
        <div className={css(STYLES.querySection)}>
          <FilterInput value={this.state.query}
            onChange={this.onQueryChange}/>
          <div className={css(STYLES.chipGap)}/>
          <FilterChip label='Risk Controls'
            isChecked={this.state.categories.has(Type.RISK)}
            onChange={this.onToggleRisk}/>
          <div className={css(STYLES.chipSpacing)}/>
          <FilterChip label='Entitlements'
            isChecked={this.state.categories.has(Type.ENTITLEMENTS)}
            onChange={this.onToggleEntitlements}/>
          <div className={css(STYLES.chipSpacing)}/>
          <FilterChip label='Compliance'
            isChecked={this.state.categories.has(Type.COMPLIANCE)}
            onChange={this.onToggleCompliance}/>
        </div>
        <div className={css(STYLES.toolbarGap)}/>
        <div className={css(STYLES.controlsSection)}>
          <SegmentedControl name='request-state'>
            <SegmentButton label='Pending'
              badge={this.formatBadge(
                this.props.response.facetCounts.pending)}
              isChecked={this.state.requestState ===
                RequestDirectoryPage.RequestState.PENDING}
              onChange={this.onSelectPending}/>
            <SegmentButton label='Approved'
              badge={this.formatBadge(
                this.props.response.facetCounts.approved)}
              isChecked={this.state.requestState ===
                RequestDirectoryPage.RequestState.APPROVED}
              onChange={this.onSelectApproved}/>
            <SegmentButton label='Rejected'
              badge={this.formatBadge(
                this.props.response.facetCounts.rejected)}
              isChecked={this.state.requestState ===
                RequestDirectoryPage.RequestState.REJECTED}
              onChange={this.onSelectRejected}/>
          </SegmentedControl>
          <div className={css(STYLES.controlsGap)}/>
          <label className={css(STYLES.sortLabel)}
            htmlFor='sort-by-2'>Sort by</label>
          <RequestSortSelect value={this.state.sortKey}
            onChange={this.onSortChange}/>
          <div className={css(STYLES.filtersButtonGap)}/>
          <IconLabelButton
            icon='resources/requests_page/filters.svg'
            label={this.props.filterCount > 0 ?
              `Filters (${this.props.filterCount})` : 'Filters'}
            onClick={this.onOpenFilterModal}/>
        </div>
        {this.state.isFilterModalOpen &&
          <RequestFilterModal
            displaySize={DisplaySize.LARGE}
            categories={this.state.categories}
            startDate={this.props.filters.startDate}
            endDate={this.props.filters.endDate}
            sortKey={this.state.sortKey}
            onSubmit={this.onFilterSubmit}
            onClose={this.onCloseFilterModal}/>}
      </form>);
  }

  private renderRequestContent(): JSX.Element {
    const isLoading = this.props.displayStatus ===
      RequestDirectoryPage.DisplayStatus.IN_PROGRESS;
    if(this.props.displayStatus ===
        RequestDirectoryPage.DisplayStatus.ERROR) {
      return (
        <section aria-label='Requests' aria-live='polite'>
          <div className={css(STYLES.fallback)}>
            <ErrorMessage
              message='There was an error loading the requests.'/>
          </div>
        </section>);
    }
    if(this.props.displayStatus ===
        RequestDirectoryPage.DisplayStatus.EMPTY) {
      return (
        <section aria-label='Requests' aria-live='polite'>
          <div className={css(STYLES.fallback)}>
            <EmptyMessage
              message='No results found. Try adjusting filters.'/>
          </div>
        </section>);
    }
    if(isLoading) {
      return (
        <section aria-label='Requests' aria-live='polite' aria-busy='true'>
          <ul className={css(STYLES.requestList)}>
            {Array.from({length: 5}, (_, i) =>
              <li key={i} className={css(STYLES.listItem,
                  i === 4 && STYLES.listItemLast)}>
                <RequestItemPlaceholder/>
              </li>)}
          </ul>
        </section>);
    }
    const items = this.props.response.requestList;
    return (
      <section aria-label='Requests' aria-live='polite' aria-busy='false'>
        <ul className={css(STYLES.requestList)}>
          {items.map((item, i) =>
            <li key={item.id} className={css(STYLES.listItem,
                i === items.length - 1 && STYLES.listItemLast)}>
              <RequestItem
                id={item.id}
                category={item.category}
                state={item.state}
                updateTime={item.updateTime}
                accountName={item.accountName}
                effectiveDate={item.effectiveDate}
                firstChange={item.firstChange}
                additionalChangesCount={item.additionalChangesCount}
                commentCount={item.commentCount}
                managerApproval={item.managerApproval}/>
            </li>)}
        </ul>
      </section>);
  }

  private renderPaginationSection(): JSX.Element {
    if(this.props.displayStatus ===
        RequestDirectoryPage.DisplayStatus.IN_PROGRESS &&
        this.props.response.requestList.length <= 50) {
      return null;
    }
    return (
      <div className={css(STYLES.paginationSection)}>
        <Pagination
          pageIndex={this.state.pageIndex}
          totalCount={this.props.response.requestList.length}
          onNavigate={this.onPageNavigate}/>
      </div>);
  }

  private formatBadge(count: number): string | undefined {
    if(this.props.response.status ===
        RequestDirectoryPage.ResponseStatus.READY) {
      return String(count);
    }
    return undefined;
  }

  private onFormSubmit = (event: React.FormEvent) => {
    event.preventDefault();
  }

  private onQueryChange = (value: string) => {
    this.setState({query: value});
  }

  private toggleCategory(type: Type) {
    const next = new Set(this.state.categories);
    if(next.has(type)) {
      next.delete(type);
    } else {
      next.add(type);
    }
    this.setState({categories: next});
  }

  private onToggleRisk = () => {
    this.toggleCategory(Type.RISK);
  }

  private onToggleEntitlements = () => {
    this.toggleCategory(Type.ENTITLEMENTS);
  }

  private onToggleCompliance = () => {
    this.toggleCategory(Type.COMPLIANCE);
  }

  private onSelectPending = () => {
    this.setState({
      requestState: RequestDirectoryPage.RequestState.PENDING
    });
    this.submit(RequestDirectoryPage.RequestState.PENDING);
  }

  private onSelectApproved = () => {
    this.setState({
      requestState: RequestDirectoryPage.RequestState.APPROVED
    });
    this.submit(RequestDirectoryPage.RequestState.APPROVED);
  }

  private onSelectRejected = () => {
    this.setState({
      requestState: RequestDirectoryPage.RequestState.REJECTED
    });
    this.submit(RequestDirectoryPage.RequestState.REJECTED);
  }

  private onSortChange = (value: RequestSortSelect.Field) => {
    this.setState({sortKey: value});
  }

  private onOpenFilterModal = () => {
    this.setState({isFilterModalOpen: true});
  }

  private onCloseFilterModal = () => {
    this.setState({isFilterModalOpen: false});
  }

  private onFilterSubmit = (criteria: RequestFilterModal.Criteria) => {
    this.setState({
      categories: criteria.categories,
      sortKey: criteria.sortKey,
      isFilterModalOpen: false
    });
    this.props.onSubmit?.({
      scope: this.props.scope,
      requestState: this.state.requestState,
      filters: {
        query: this.state.query,
        categories: criteria.categories,
        startDate: criteria.startDate,
        endDate: criteria.endDate,
        sortKey: criteria.sortKey
      },
      pageIndex: 0
    });
  }

  private onPageNavigate = (pageIndex: number) => {
    this.setState({pageIndex});
    this.submit(this.state.requestState, pageIndex);
  }

  private submit(requestState: RequestDirectoryPage.RequestState,
      pageIndex?: number) {
    this.props.onSubmit?.({
      scope: this.props.scope,
      requestState,
      filters: {
        query: this.state.query,
        categories: this.state.categories,
        startDate: this.props.filters.startDate,
        endDate: this.props.filters.endDate,
        sortKey: this.state.sortKey
      },
      pageIndex: pageIndex ?? this.state.pageIndex
    });
  }
}

export namespace RequestDirectoryPage {

  /** The scope of the displayed requests. */
  export enum Scope {
    YOU,
    GROUP
  }

  /** The status of the displayed requests list. */
  export enum DisplayStatus {
    IN_PROGRESS,
    ERROR,
    EMPTY,
    READY
  }

  /** The currently selected request state facet. */
  export enum RequestState {
    PENDING,
    APPROVED,
    REJECTED
  }

  /** The status of the server response. */
  export enum ResponseStatus {
    IN_PROGRESS,
    READY,
    ERROR
  }

  /** The user's filter criteria. */
  export interface Filters {
    query: string;
    categories: Set<Type>;
    startDate?: Beam.Date;
    endDate?: Beam.Date;
    sortKey: RequestSortSelect.Field;
  }

  /** The facet counts across request states. */
  export interface FacetCounts {
    pending: number;
    approved: number;
    rejected: number;
  }

  /** A single request list entry. */
  export interface RequestEntry {
    id: number;
    category: Nexus.AccountModificationRequest.Type;
    state: Nexus.AccountModificationRequest.Status;
    updateTime: Date;
    accountName: string;
    effectiveDate: Date;
    firstChange: RequestItem.Change;
    additionalChangesCount: number;
    commentCount: number;
    managerApproval?: RequestItem.ManagerApproval;
  }

  /** The response from the server. */
  export interface Response {
    status: ResponseStatus;
    facetCounts: FacetCounts;
    requestList: RequestEntry[];
  }

  /** The submission signal payload. */
  export interface Submission {
    scope: Scope;
    requestState: RequestState;
    filters: Filters;
    pageIndex: number;
  }
}

const STYLES = StyleSheet.create({
  main: {
    padding: '18px 0 40px',
    backgroundColor: '#FFFFFF',
    fontFamily: "'Roboto', system-ui, sans-serif",
    fontWeight: 400,
    color: '#333333'
  },
  toolbar: {
    padding: '0 18px',
    containerType: 'inline-size'
  },
  querySection: {
    display: 'flex',
    flexWrap: 'wrap',
    alignItems: 'center',
    gap: '0'
  },
  chipGap: {
    width: '12px',
    flexShrink: 0
  },
  chipSpacing: {
    width: '10px',
    flexShrink: 0
  },
  controlsSection: {
    display: 'flex',
    flexWrap: 'wrap',
    alignItems: 'center',
    gap: '0'
  },
  toolbarGap: {
    height: '10px'
  },
  controlsGap: {
    width: '12px',
    flexShrink: 0
  },
  sortLabel: {
    fontFamily: 'inherit',
    fontSize: 'inherit',
    fontWeight: 'inherit',
    marginRight: '4px',
    whiteSpace: 'nowrap'
  },
  filtersButtonGap: {
    flexGrow: 1
  },
  fallback: {
    backgroundColor: '#FFFFFF',
    padding: '0 18px'
  },
  requestList: {
    padding: 0,
    margin: 0,
    listStyle: 'none',
    containerType: 'inline-size'
  },
  listItem: {
    borderBottom: '1px solid #E6E6E6'
  },
  listItemLast: {
    borderBottomColor: 'transparent'
  },
  paginationSection: {
    padding: '0 18px'
  }
});
