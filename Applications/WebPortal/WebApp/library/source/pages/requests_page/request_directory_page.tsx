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
          {this.renderToolbar()}
          <div className={css(STYLES.contentGap)}/>
          {this.renderRequestContent()}
          {this.renderPaginationSection()}
        </main>
      </PageLayout>);
  }

  private renderToolbar(): JSX.Element {
    return (
      <form aria-label='Request Filters' className={css(STYLES.toolbar)}
        onSubmit={this.onFormSubmit}>
        {this.renderNarrowToolbar()}
        {this.renderWideToolbar()}
        {this.state.isFilterModalOpen &&
          <RequestFilterModal
            displaySize={this.state.filterModalSize}
            categories={this.props.filters.categories}
            startDate={this.props.filters.startDate}
            endDate={this.props.filters.endDate}
            sortKey={this.props.filters.sortKey}
            onSubmit={this.onFilterSubmit}
            onClose={this.onCloseFilterModal}/>}
      </form>);
  }

  private renderNarrowToolbar(): JSX.Element {
    const filtersLabel = this.props.filterCount > 0 ?
      `Filters (${this.props.filterCount})` : 'Filters';
    return (
      <div className={css(STYLES.narrowToolbar)}>
        <div className={css(STYLES.narrowQueryRow)}>
          <div className={css(STYLES.narrowQueryCell)}>
            <FilterInput value={this.props.filters.query}
              placeholder='Filter requests'
              onChange={this.onQueryChange}/>
          </div>
          <div className={css(STYLES.narrowFiltersGap)}/>
          <IconLabelButton
            icon='resources/requests_page/filters.svg'
            label={filtersLabel}
            onClick={this.onOpenFilterModal}/>
        </div>
        <div className={css(STYLES.narrowSegmentGap)}/>
        {this.renderSegmentedControl('request-state-narrow', false)}
      </div>);
  }

  private renderWideToolbar(): JSX.Element {
    return (
      <div className={css(STYLES.wideToolbar)}>
        <div className={css(STYLES.wideQueryColumn)}>
          {this.renderQuerySection()}
        </div>
        <div className={css(STYLES.wideFlexColumn)}/>
        <div className={css(STYLES.wideGapColumn)}/>
        <div className={css(STYLES.wideControlsColumn)}>
          {this.renderControlsSection()}
        </div>
      </div>);
  }

  private renderQuerySection(): JSX.Element {
    return (
      <div className={css(STYLES.querySection)}>
        <FilterInput value={this.props.filters.query}
          placeholder='Filter requests'
          onChange={this.onQueryChange}/>
        <div className={css(STYLES.querySectionGap)}/>
        <div className={css(STYLES.chipRow)}>
          <FilterChip label='Risk Controls'
            isChecked={this.props.filters.categories.has(Type.RISK)}
            onChange={this.onToggleRisk}/>
          <div className={css(STYLES.chipSpacing)}/>
          <FilterChip label='Entitlements'
            isChecked={this.props.filters.categories.has(Type.ENTITLEMENTS)}
            onChange={this.onToggleEntitlements}/>
          <div className={css(STYLES.chipSpacing)}/>
          <FilterChip label='Compliance'
            isChecked={this.props.filters.categories.has(Type.COMPLIANCE)}
            onChange={this.onToggleCompliance}/>
        </div>
      </div>);
  }

  private renderControlsSection(): JSX.Element {
    const filtersLabel = this.props.filterCount > 0 ?
      `Filters (${this.props.filterCount})` : 'Filters';
    return (
      <div className={css(STYLES.controlsSection)}>
        <div className={css(STYLES.mediumSegment)}>
          {this.renderSegmentedControl('request-state-medium', false)}
        </div>
        <div className={css(STYLES.wideSegment)}>
          {this.renderSegmentedControl('request-state', true)}
        </div>
        <div className={css(STYLES.controlsSectionGap)}/>
        <div className={css(STYLES.sortRow)}>
          <label className={css(STYLES.sortLabel)}
            htmlFor='request-sort'>Sort by</label>
          <div className={css(STYLES.sortLabelGap)}/>
          <div className={css(STYLES.sortByCell)}>
            <RequestSortSelect id='request-sort'
              value={this.props.filters.sortKey}
              onChange={this.onSortChange}/>
          </div>
          <div className={css(STYLES.sortFiltersGap)}/>
          <div className={css(STYLES.wideFiltersButton)}>
            <IconLabelButton
              aria-label='Filters'
              variant={IconLabelButton.Variant.ICON_LABEL}
              style={{width: '120px'}}
              icon='resources/requests_page/filters.svg'
              label={filtersLabel}
              onClick={this.onOpenFilterModal}/>
          </div>
          <div className={css(STYLES.mediumFiltersButton)}>
            <IconLabelButton
              aria-label='Filters'
              icon='resources/requests_page/filters.svg'
              label='Filters'
              onClick={this.onOpenFilterModal}/>
          </div>
        </div>
      </div>);
  }

  private renderSegmentedControl(
      name: string, showBadge: boolean): JSX.Element {
    const badge = (count: number) =>
      showBadge ? this.formatBadge(count) : undefined;
    return (
      <SegmentedControl name={name}>
        <SegmentButton label='Pending'
          badge={badge(this.props.response.facetCounts.pending)}
          isChecked={this.props.requestState ===
            RequestsModel.RequestState.PENDING}
          onChange={this.onSelectPending}/>
        <SegmentButton label='Approved'
          badge={badge(this.props.response.facetCounts.approved)}
          isChecked={this.props.requestState ===
            RequestsModel.RequestState.APPROVED}
          onChange={this.onSelectApproved}/>
        <SegmentButton label='Rejected'
          badge={badge(this.props.response.facetCounts.rejected)}
          isChecked={this.props.requestState ===
            RequestsModel.RequestState.REJECTED}
          onChange={this.onSelectRejected}/>
      </SegmentedControl>);
  }

  private renderRequestContent(): JSX.Element {
    const isLoading = this.props.displayStatus ===
      RequestDirectoryPage.DisplayStatus.IN_PROGRESS;
    if(this.props.displayStatus ===
        RequestDirectoryPage.DisplayStatus.ERROR) {
      return (
        <section aria-label='Requests' aria-live='polite'
            className={css(STYLES.fallbackSection)}>
          <div className={css(STYLES.fallback)}>
            <ErrorMessage message='There was an error loading the requests.'/>
          </div>
        </section>);
    }
    if(this.props.displayStatus ===
        RequestDirectoryPage.DisplayStatus.EMPTY) {
      return (
        <section aria-label='Requests' aria-live='polite'
            className={css(STYLES.fallbackSection)}>
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
            {[0, 1, 2, 3, 4].map(i =>
              <li key={i}>
                <RequestItemPlaceholder/>
              </li>)}
          </ul>
        </section>);
    }
    const all = this.props.response.requestList;
    const start = this.props.pageIndex * PAGE_SIZE;
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
                onClick={this.props.onClickRequest}/>
            </li>)}
        </ul>
      </section>);
  }

  private renderPaginationSection(): JSX.Element {
    if(this.props.displayStatus ===
        RequestDirectoryPage.DisplayStatus.ERROR ||
        this.props.displayStatus ===
        RequestDirectoryPage.DisplayStatus.EMPTY) {
      return null;
    }
    if(this.props.displayStatus ===
        RequestDirectoryPage.DisplayStatus.IN_PROGRESS &&
        this.props.response.requestList.length <= PAGE_SIZE) {
      return null;
    }
    return (
      <div className={css(STYLES.paginationSection)}>
        <Pagination
          pageSize={PAGE_SIZE}
          pageIndex={this.props.pageIndex}
          totalCount={this.props.response.requestList.length}
          onNavigate={this.onPageNavigate}/>
      </div>);
  }

  private formatBadge(count: number): string | undefined {
    if(this.props.response.status ===
        RequestsModel.ResponseStatus.READY) {
      return String(count);
    }
    return undefined;
  }

  private onFormSubmit = (event: React.FormEvent) => {
    event.preventDefault();
  }

  private onQueryChange = (value: string) => {
    this.props.onSubmit?.({
      scope: this.props.scope,
      requestState: this.props.requestState,
      filters: {
        ...this.props.filters,
        query: value
      },
      pageIndex: this.props.pageIndex
    });
  }

  private toggleCategory(type: Type) {
    const next = new Set(this.props.filters.categories);
    if(next.has(type)) {
      next.delete(type);
    } else {
      next.add(type);
    }
    this.props.onSubmit?.({
      scope: this.props.scope,
      requestState: this.props.requestState,
      filters: {
        ...this.props.filters,
        categories: next
      },
      pageIndex: this.props.pageIndex
    });
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
    this.submit(RequestsModel.RequestState.PENDING);
  }

  private onSelectApproved = () => {
    this.submit(RequestsModel.RequestState.APPROVED);
  }

  private onSelectRejected = () => {
    this.submit(RequestsModel.RequestState.REJECTED);
  }

  private onSortChange = (value: RequestsModel.SortField) => {
    this.props.onSubmit?.({
      scope: this.props.scope,
      requestState: this.props.requestState,
      filters: {
        ...this.props.filters,
        sortKey: value
      },
      pageIndex: this.props.pageIndex
    });
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
    this.submit(this.props.requestState, pageIndex);
  }

  private mainRef = React.createRef<HTMLElement>();

  private submit(requestState: RequestsModel.RequestState,
      pageIndex?: number) {
    this.props.onSubmit?.({
      scope: this.props.scope,
      requestState,
      filters: this.props.filters,
      pageIndex: pageIndex ?? this.props.pageIndex
    });
  }
}

const PAGE_SIZE = 25;

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
    containerType: 'inline-size',
    display: 'flex',
    flexDirection: 'column',
    flex: '1 1 auto'
  },
  toolbar: {
    padding: '0 18px'
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
    '@container (min-width: 1036px)': {
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
    '@container (min-width: 1036px)': {
      display: 'block'
    }
  },
  mediumSegment: {
    '@container (min-width: 1036px)': {
      display: 'none'
    }
  },
  wideSegment: {
    display: 'none',
    '@container (min-width: 1036px)': {
      display: 'block'
    }
  },
  mediumFiltersButton: {
    '@container (min-width: 1036px)': {
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
