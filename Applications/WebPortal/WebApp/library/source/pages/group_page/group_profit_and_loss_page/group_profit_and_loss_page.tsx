import { css, StyleSheet } from 'aphrodite/no-important';
import * as Beam from 'beam';
import * as Nexus from 'nexus';
import * as React from 'react';
import { Button, DateInput, EmptyMessage, ErrorMessage, PageLayout,
  Select } from '../../..';
import { ProfitAndLossHeader, ProfitAndLossItemPlaceholder,
  ProfitAndLossTable, ReportStatusIndicator } from
    '../../../pages/account_page/profit_and_loss_page';
import { AccountProfitAndLossItem } from './account_profit_and_loss_item';

interface Properties {

  /** The account's currency symbol. */
  symbol: string;

  /** The account's currency code. */
  code: string;

  /** The status of the report. */
  status: GroupProfitAndLossPage.Status;

  /** The status prior to a transition to IN_PROGRESS. */
  previousStatus: GroupProfitAndLossPage.Status;

  /** The input mode for dates. */
  mode: GroupProfitAndLossPage.Mode;

  /** The selected start date. */
  startDate: Beam.Date;

  /** The selected end date. */
  endDate: Beam.Date;

  /** The total profit and loss. */
  totalPnl: string;

  /** The total fees. */
  totalFees: string;

  /** The total volume. */
  totalVolume: string;

  /** The list of accounts with activity. */
  accounts: GroupProfitAndLossPage.AccountEntry[];

  /** The list of foreign currencies with exchange rates. */
  foreignCurrencies: GroupProfitAndLossPage.ExchangeRate[];

  /** The filepath to the generated report. */
  filepath: string;

  /** Called when the mode changes. */
  onModeChange?: (mode: GroupProfitAndLossPage.Mode) => void;

  /** Called when the start date changes. */
  onStartDateChange?: (date: Beam.Date) => void;

  /** Called when the end date changes. */
  onEndDateChange?: (date: Beam.Date) => void;

  /** Called when the user submits. */
  onSubmit?: (start: Beam.Date, end: Beam.Date) => void;

  /** Called when the user cancels. */
  onCancel?: () => void;
}

/** Displays the group profit and loss page. */
export class GroupProfitAndLossPage extends React.Component<Properties> {
  public render(): JSX.Element {
    const isError = this.props.status === GroupProfitAndLossPage.Status.ERROR;
    const isNoResults =
      this.props.status === GroupProfitAndLossPage.Status.NO_RESULTS;
    const reducedPadding = isError || isNoResults;
    return (
      <PageLayout>
        <main className={
            css(STYLES.main, reducedPadding && STYLES.mainReducedPadding)}>
          <FormContainer
            status={this.props.status}
            mode={this.props.mode}
            startDate={this.props.startDate}
            endDate={this.props.endDate}
            filepath={this.props.filepath}
            accounts={this.props.accounts}
            onModeChange={this.props.onModeChange}
            onStartDateChange={this.props.onStartDateChange}
            onEndDateChange={this.props.onEndDateChange}
            onSubmit={this.props.onSubmit}
            onCancel={this.props.onCancel}/>
          <div className={css(STYLES.contentSpacing)}/>
          <ProfitAndLossContent
            symbol={this.props.symbol}
            code={this.props.code}
            status={this.props.status}
            previousStatus={this.props.previousStatus}
            totalPnl={this.props.totalPnl}
            totalFees={this.props.totalFees}
            totalVolume={this.props.totalVolume}
            accounts={this.props.accounts}
            foreignCurrencies={this.props.foreignCurrencies}
            startDate={this.props.startDate}
            endDate={this.props.endDate}
            onSubmit={this.props.onSubmit}/>
        </main>
      </PageLayout>);
  }
}

export namespace GroupProfitAndLossPage {

  /** Enumerates the possible states of the report. */
  export enum Status {

    /** No report has been generated. */
    EMPTY,

    /** The report is being generated. */
    IN_PROGRESS,

    /** The report is ready. */
    READY,

    /** The report is out of date. */
    STALE,

    /** There is no activity for the selected period. */
    NO_RESULTS,

    /** There was an error generating the report. */
    ERROR
  }

  /** The input mode for the date range. */
  export enum Mode {

    /** The current calendar month. */
    THIS_MONTH,

    /** The previous calendar month. */
    LAST_MONTH,

    /** Start and end date are user-specified. */
    CUSTOM
  }

  /** A currency entry within an account's report. */
  export interface CurrencyEntry {

    /** The currency symbol. */
    symbol: string;

    /** The currency code. */
    code: string;

    /** The total profit and loss. */
    totalProfitAndLoss: Nexus.Money;

    /** The total volume traded. */
    totalVolume: Nexus.Quantity;

    /** The total fees incurred. */
    totalFees: Nexus.Money;

    /** The list of tickers traded. */
    tickers: ProfitAndLossTable.Ticker[];
  }

  /** An account entry within the group report. */
  export interface AccountEntry {

    /** The username for the account. */
    username: string;

    /** The total profit and loss in account currency. */
    totalPnl: string;

    /** The list of currencies with activity. */
    currencies: CurrencyEntry[];
  }

  /** An exchange rate for a foreign currency. */
  export interface ExchangeRate {

    /** The currency code. */
    code: string;

    /** The currency symbol. */
    symbol: string;

    /** The exchange rate. */
    rate: string;
  }
}

interface FormContainerProperties {
  status: GroupProfitAndLossPage.Status;
  mode: GroupProfitAndLossPage.Mode;
  startDate: Beam.Date;
  endDate: Beam.Date;
  filepath: string;
  accounts: GroupProfitAndLossPage.AccountEntry[];
  onModeChange?: (mode: GroupProfitAndLossPage.Mode) => void;
  onStartDateChange?: (date: Beam.Date) => void;
  onEndDateChange?: (date: Beam.Date) => void;
  onSubmit?: (start: Beam.Date, end: Beam.Date) => void;
  onCancel?: () => void;
}

class FormContainer extends React.Component<FormContainerProperties> {
  constructor(props: FormContainerProperties) {
    super(props);
    this.containerRef = React.createRef<HTMLDivElement>();
    this.formRef = React.createRef<Form>();
    this.lastHeight = 0;
  }

  public render(): JSX.Element {
    return (
      <div ref={this.containerRef} className={css(STYLES.formContainer)}>
        <Form ref={this.formRef}
          status={this.props.status}
          mode={this.props.mode}
          startDate={this.props.startDate}
          endDate={this.props.endDate}
          filepath={this.props.filepath}
          accounts={this.props.accounts}
          onModeChange={this.props.onModeChange}
          onStartDateChange={this.props.onStartDateChange}
          onEndDateChange={this.props.onEndDateChange}
          onSubmit={this.props.onSubmit}
          onCancel={this.props.onCancel}/>
      </div>);
  }

  public componentDidMount(): void {
    const container = this.containerRef.current;
    const form = this.formRef.current?.element;
    if(!container || !form) {
      return;
    }
    container.style.maxHeight = `${form.scrollHeight}px`;
    this.lastHeight = form.scrollHeight;
    this.observer = new ResizeObserver(() => {
      const height = form.scrollHeight;
      if(height > this.lastHeight) {
        container.style.transition = 'none';
        container.style.maxHeight = `${height}px`;
        container.getBoundingClientRect();
        container.style.transition = '';
      } else {
        container.style.maxHeight = `${height}px`;
      }
      this.lastHeight = height;
    });
    this.observer.observe(form);
  }

  public componentWillUnmount(): void {
    this.observer?.disconnect();
  }

  private containerRef: React.RefObject<HTMLDivElement>;
  private formRef: React.RefObject<Form>;
  private observer: ResizeObserver;
  private lastHeight: number;
}

interface FormProperties {
  status: GroupProfitAndLossPage.Status;
  mode: GroupProfitAndLossPage.Mode;
  startDate: Beam.Date;
  endDate: Beam.Date;
  filepath: string;
  accounts: GroupProfitAndLossPage.AccountEntry[];
  onModeChange?: (mode: GroupProfitAndLossPage.Mode) => void;
  onStartDateChange?: (date: Beam.Date) => void;
  onEndDateChange?: (date: Beam.Date) => void;
  onSubmit?: (start: Beam.Date, end: Beam.Date) => void;
  onCancel?: () => void;
}

class Form extends React.Component<FormProperties> {
  constructor(props: FormProperties) {
    super(props);
    this.formRef = React.createRef<HTMLFormElement>();
  }

  public get element(): HTMLFormElement {
    return this.formRef.current;
  }

  public render(): JSX.Element {
    const isCustom = this.props.mode === GroupProfitAndLossPage.Mode.CUSTOM;
    const isError = this.props.status === GroupProfitAndLossPage.Status.ERROR;
    const isNoResults =
      this.props.status === GroupProfitAndLossPage.Status.NO_RESULTS;
    const hideActionSheet = isError || isNoResults;
    const isDateRangeValid =
      this.props.endDate.compare(this.props.startDate) >= 0;
    const MODE_MAP: Record<string, GroupProfitAndLossPage.Mode> = {
      'this-month': GroupProfitAndLossPage.Mode.THIS_MONTH,
      'last-month': GroupProfitAndLossPage.Mode.LAST_MONTH,
      'custom': GroupProfitAndLossPage.Mode.CUSTOM
    };
    const VALUE_MAP: Record<number, string> = {
      [GroupProfitAndLossPage.Mode.THIS_MONTH]: 'this-month',
      [GroupProfitAndLossPage.Mode.LAST_MONTH]: 'last-month',
      [GroupProfitAndLossPage.Mode.CUSTOM]: 'custom'
    };
    const onSelectChange = (value: string) => {
      this.props.onModeChange?.(MODE_MAP[value]);
    };
    const onFormSubmit = (event: React.FormEvent) => {
      event.preventDefault();
      if(isDateRangeValid) {
        this.props.onSubmit?.(this.props.startDate, this.props.endDate);
      }
    };
    return (
      <form ref={this.formRef} aria-label='Report Controls'
          onSubmit={onFormSubmit} className={css(STYLES.form)}>
        <div className={css(STYLES.selectWrapper)}>
          <Select value={VALUE_MAP[this.props.mode]}
              aria-controls='custom-dates' style={{width: '100%'}}
              onChange={onSelectChange}>
            <option value='this-month'>This Month</option>
            <option value='last-month'>Last Month</option>
            <option value='custom'>Custom</option>
          </Select>
        </div>
        <CustomDates className={css(STYLES.customDates)}
          isOpen={isCustom}
          startDate={this.props.startDate}
          endDate={this.props.endDate}
          onStartDateChange={this.props.onStartDateChange}
          onEndDateChange={this.props.onEndDateChange}/>
        <DateFilter
          mode={this.props.mode}
          isCustom={isCustom}
          startDate={this.props.startDate}
          endDate={this.props.endDate}
          onModeChange={this.props.onModeChange}
          onStartDateChange={this.props.onStartDateChange}
          onEndDateChange={this.props.onEndDateChange}/>
        <StatusFeedback
          status={this.props.status}
          accounts={this.props.accounts}/>
        <ActionsAndStatus
          status={this.props.status}
          accounts={this.props.accounts}
          filepath={this.props.filepath}
          isDateRangeValid={isDateRangeValid}
          startDate={this.props.startDate}
          endDate={this.props.endDate}
          onSubmit={this.props.onSubmit}
          onCancel={this.props.onCancel}/>
        {!hideActionSheet &&
          <ActionSheet
            status={this.props.status}
            accounts={this.props.accounts}
            filepath={this.props.filepath}
            isDateRangeValid={isDateRangeValid}
            startDate={this.props.startDate}
            endDate={this.props.endDate}
            onSubmit={this.props.onSubmit}
            onCancel={this.props.onCancel}/>}
      </form>);
  }

  private formRef: React.RefObject<HTMLFormElement>;
}

interface CustomDatesProperties {
  className?: string;
  isOpen: boolean;
  startDate: Beam.Date;
  endDate: Beam.Date;
  onStartDateChange?: (date: Beam.Date) => void;
  onEndDateChange?: (date: Beam.Date) => void;
}

class CustomDates extends React.Component<CustomDatesProperties> {
  constructor(props: CustomDatesProperties) {
    super(props);
    this.wrapperRef = React.createRef<HTMLDivElement>();
    this.contentRef = React.createRef<HTMLDivElement>();
  }

  public render(): JSX.Element {
    const className = [css(STYLES.customDatesWrapper),
      this.props.className].join(' ');
    return (
      <div ref={this.wrapperRef} id='custom-dates'
          className={className}
          style={{maxHeight: this.props.isOpen ? undefined : '0px'}}>
        <div ref={this.contentRef} className={css(STYLES.customDatesContent)}>
          <DateInputs
            startDate={this.props.startDate}
            endDate={this.props.endDate}
            onStartDateChange={this.props.onStartDateChange}
            onEndDateChange={this.props.onEndDateChange}/>
        </div>
      </div>);
  }

  public componentDidMount(): void {
    this.observer = new ResizeObserver(() => {
      const wrapper = this.wrapperRef.current;
      const content = this.contentRef.current;
      if(wrapper && content && this.props.isOpen) {
        wrapper.style.maxHeight = `${content.scrollHeight}px`;
      }
    });
    if(this.contentRef.current) {
      this.observer.observe(this.contentRef.current);
    }
  }

  public componentDidUpdate(prevProps: {isOpen: boolean}): void {
    const wrapper = this.wrapperRef.current;
    const content = this.contentRef.current;
    if(!wrapper || !content) {
      return;
    }
    if(this.props.isOpen && !prevProps.isOpen) {
      wrapper.style.maxHeight = `${content.scrollHeight}px`;
    } else if(!this.props.isOpen && prevProps.isOpen) {
      wrapper.style.maxHeight = `${wrapper.scrollHeight}px`;
      wrapper.getBoundingClientRect();
      wrapper.style.maxHeight = '0px';
    }
  }

  public componentWillUnmount(): void {
    this.observer?.disconnect();
  }

  private wrapperRef: React.RefObject<HTMLDivElement>;
  private contentRef: React.RefObject<HTMLDivElement>;
  private observer: ResizeObserver;
}

function DateInputs(props: {
      startDate: Beam.Date;
      endDate: Beam.Date;
      onStartDateChange?: (date: Beam.Date) => void;
      onEndDateChange?: (date: Beam.Date) => void;
    }): JSX.Element {
  return (
    <div className={css(STYLES.dateInputs)}>
      <StartLabel/>
      <StartDate value={props.startDate} onChange={props.onStartDateChange}/>
      <EndLabel/>
      <EndDate value={props.endDate} onChange={props.onEndDateChange}/>
      <ValidityCheckSection
        startDate={props.startDate} endDate={props.endDate}/>
    </div>);
}

function StartLabel(): JSX.Element {
  return (
    <label htmlFor='start-date'
      className={css(STYLES.dateLabel, STYLES.startLabel)}>Start</label>);
}

function EndLabel(): JSX.Element {
  return (
    <label htmlFor='end-date'
      className={css(STYLES.dateLabel, STYLES.endLabel)}>End</label>);
}

function StartDate(props: {
      value: Beam.Date;
      onChange?: (date: Beam.Date) => void;
    }): JSX.Element {
  return (
    <div className={css(STYLES.startDate)}>
      <DateInput id='start-date' value={props.value}
        style={{width: '100%'}} onChange={props.onChange}/>
    </div>);
}

function EndDate(props: {
      value: Beam.Date;
      onChange?: (date: Beam.Date) => void;
    }): JSX.Element {
  return (
    <div className={css(STYLES.endDate)}>
      <DateInput id='end-date' value={props.value}
        style={{width: '100%'}} onChange={props.onChange}/>
    </div>);
}

interface ValidityCheckSectionProperties {
  startDate: Beam.Date;
  endDate: Beam.Date;
}

class ValidityCheckSection extends
    React.Component<ValidityCheckSectionProperties> {
  constructor(props: ValidityCheckSectionProperties) {
    super(props);
    this.wrapperRef = React.createRef<HTMLDivElement>();
    this.messageRef = React.createRef<HTMLSpanElement>();
    this.maxHeight = '0px';
  }

  public render(): JSX.Element {
    const isInvalid = this.props.endDate.compare(this.props.startDate) < 0;
    return (
      <div ref={this.wrapperRef} className={css(STYLES.validityCheckSection)}
          style={{maxHeight: isInvalid ? this.maxHeight : '0px'}}>
        <span ref={this.messageRef} className={css(STYLES.invalidMessage)}>
          End date must be greater than start date
        </span>
      </div>);
  }

  public componentDidMount(): void {
    this.observer = new ResizeObserver(() => this.updateMaxHeight());
    if(this.messageRef.current) {
      this.observer.observe(this.messageRef.current);
    }
    this.updateMaxHeight();
  }

  public componentWillUnmount(): void {
    this.observer?.disconnect();
  }

  private updateMaxHeight(): void {
    const message = this.messageRef.current;
    if(message) {
      this.maxHeight = `${message.offsetHeight + 12}px`;
      const wrapper = this.wrapperRef.current;
      const isInvalid =
        this.props.endDate.compare(this.props.startDate) < 0;
      if(wrapper && isInvalid) {
        wrapper.style.maxHeight = this.maxHeight;
      }
    }
  }

  private wrapperRef: React.RefObject<HTMLDivElement>;
  private messageRef: React.RefObject<HTMLSpanElement>;
  private observer: ResizeObserver;
  private maxHeight: string;
}

function DateFilter(props: {
      mode: GroupProfitAndLossPage.Mode;
      isCustom: boolean;
      startDate: Beam.Date;
      endDate: Beam.Date;
      onModeChange?: (mode: GroupProfitAndLossPage.Mode) => void;
      onStartDateChange?: (date: Beam.Date) => void;
      onEndDateChange?: (date: Beam.Date) => void;
    }): JSX.Element {
  const MODE_MAP: Record<string, GroupProfitAndLossPage.Mode> = {
    'this-month': GroupProfitAndLossPage.Mode.THIS_MONTH,
    'last-month': GroupProfitAndLossPage.Mode.LAST_MONTH,
    'custom': GroupProfitAndLossPage.Mode.CUSTOM
  };
  const VALUE_MAP: Record<number, string> = {
    [GroupProfitAndLossPage.Mode.THIS_MONTH]: 'this-month',
    [GroupProfitAndLossPage.Mode.LAST_MONTH]: 'last-month',
    [GroupProfitAndLossPage.Mode.CUSTOM]: 'custom'
  };
  const onSelectChange = (value: string) => {
    props.onModeChange?.(MODE_MAP[value]);
  };
  return (
    <div className={css(STYLES.dateFilter)}>
      <div className={css(STYLES.dateFilterSelect)}>
        <Select value={VALUE_MAP[props.mode]}
            aria-controls='custom-dates' style={{width: '100%'}}
            onChange={onSelectChange}>
          <option value='this-month'>This Month</option>
          <option value='last-month'>Last Month</option>
          <option value='custom'>Custom</option>
        </Select>
      </div>
      <CustomDates className={css(STYLES.dateFilterDates)}
        isOpen={props.isCustom}
        startDate={props.startDate}
        endDate={props.endDate}
        onStartDateChange={props.onStartDateChange}
        onEndDateChange={props.onEndDateChange}/>
    </div>);
}

function StatusFeedback(props: {
      status: GroupProfitAndLossPage.Status;
      accounts: GroupProfitAndLossPage.AccountEntry[];
    }): JSX.Element {
  const isEmpty = props.status === GroupProfitAndLossPage.Status.EMPTY;
  const isNoResults = props.status === GroupProfitAndLossPage.Status.NO_RESULTS;
  const isStale = props.status === GroupProfitAndLossPage.Status.STALE;
  const hasData = props.accounts.length > 0;
  const hidden = isEmpty || isNoResults || (isStale && !hasData);
  return (
    <div className={
        css(STYLES.statusFeedback, hidden && STYLES.statusFeedbackHidden)}>
      <ReportStatusIndicator id='report-status'
        status={toReportStatus(props.status)}/>
    </div>);
}

function ActionsAndStatus(props: {
      status: GroupProfitAndLossPage.Status;
      accounts: GroupProfitAndLossPage.AccountEntry[];
      filepath: string;
      isDateRangeValid: boolean;
      startDate: Beam.Date;
      endDate: Beam.Date;
      onSubmit?: (start: Beam.Date, end: Beam.Date) => void;
      onCancel?: () => void;
    }): JSX.Element {
  const isLoading = props.status === GroupProfitAndLossPage.Status.IN_PROGRESS;
  const isReady = props.status === GroupProfitAndLossPage.Status.READY;
  const isStale = props.status === GroupProfitAndLossPage.Status.STALE;
  const isError = props.status === GroupProfitAndLossPage.Status.ERROR;
  const isEmpty = props.status === GroupProfitAndLossPage.Status.EMPTY;
  const isNoResults = props.status === GroupProfitAndLossPage.Status.NO_RESULTS;
  const hasData = props.accounts.length > 0;
  const applyDisabled =
    isLoading || isReady || isNoResults || !props.isDateRangeValid;
  const downloadDisabled = isEmpty || isNoResults || isError || isLoading;
  const filename =
    `group-pl-${props.startDate.toJson()}-${props.endDate.toJson()}.csv`;
  const onApply = () => {
    if(props.isDateRangeValid) {
      props.onSubmit?.(props.startDate, props.endDate);
    }
  };
  const showStatus = !isEmpty && !isNoResults && !(isStale && !hasData);
  return (
    <div className={css(STYLES.actionsAndStatus)}>
      <div className={css(STYLES.buttonRow)}>
        {isLoading ?
          <>
            <Button label='Apply' type='submit' disabled={applyDisabled}
              style={BUTTON_STYLE}/>
            <Button label='Cancel' type='button' style={BUTTON_STYLE}
              onClick={props.onCancel}/>
          </> :
          <>
            <Button label='Apply' type='submit' disabled={applyDisabled}
              style={BUTTON_STYLE}
              aria-describedby={isStale ? 'report-status' : undefined}/>
            <a download={downloadDisabled ? undefined : filename}
              href={downloadDisabled ? undefined : props.filepath}
              className={css(STYLES.downloadLink,
                downloadDisabled && STYLES.downloadLinkDisabled)}
              aria-disabled={downloadDisabled || undefined}>
              Download
            </a>
          </>}
      </div>
      {showStatus &&
        <div className={css(STYLES.desktopStatusFeedback)}>
          <ReportStatusIndicator id='report-status'
            status={toReportStatus(props.status)}/>
        </div>}
    </div>);
}

function ActionSheet(props: {
      status: GroupProfitAndLossPage.Status;
      accounts: GroupProfitAndLossPage.AccountEntry[];
      filepath: string;
      isDateRangeValid: boolean;
      startDate: Beam.Date;
      endDate: Beam.Date;
      onSubmit?: (start: Beam.Date, end: Beam.Date) => void;
      onCancel?: () => void;
    }): JSX.Element {
  const isLoading = props.status === GroupProfitAndLossPage.Status.IN_PROGRESS;
  const isReady = props.status === GroupProfitAndLossPage.Status.READY;
  const isStale = props.status === GroupProfitAndLossPage.Status.STALE;
  const hasData = props.accounts.length > 0;
  const applyDisabled = isLoading || !props.isDateRangeValid;
  const filename =
    `group-pl-${props.startDate.toJson()}-${props.endDate.toJson()}.csv`;
  const onApply = () => {
    if(props.isDateRangeValid) {
      props.onSubmit?.(props.startDate, props.endDate);
    }
  };
  return (
    <section aria-label='Report Actions' className={css(STYLES.actionSheet)}>
      {isReady &&
        <a download={filename} href={props.filepath}
            className={css(STYLES.downloadLink, STYLES.actionSheetButton)}>
          Download
        </a>}
      {isStale && hasData && <>
        <Button label='Apply' type='button' disabled={applyDisabled}
          aria-describedby='report-status'
          className={css(STYLES.actionSheetButton)} onClick={onApply}/>
        <div style={ACTION_SHEET_GAP}/>
        <a download={filename} href={props.filepath}
            className={css(STYLES.downloadLink, STYLES.actionSheetButton)}>
          Download
        </a>
      </>}
      {isStale && !hasData &&
        <Button label='Apply' type='button' disabled={applyDisabled}
          aria-describedby='report-status'
          className={css(STYLES.actionSheetButton)} onClick={onApply}/>}
      {isLoading && <>
        <Button label='Apply' type='button' disabled={applyDisabled}
          className={css(STYLES.actionSheetButton)} onClick={onApply}/>
        <div style={ACTION_SHEET_GAP}/>
        <Button label='Cancel' type='button'
          className={css(STYLES.actionSheetButton)}
          onClick={props.onCancel}/>
      </>}
      {!isReady && !isStale && !isLoading &&
        <Button label='Apply' type='button' disabled={applyDisabled}
          className={css(STYLES.actionSheetButton)} onClick={onApply}/>}
    </section>);
}

function toReportStatus(
    status: GroupProfitAndLossPage.Status): ReportStatusIndicator.Status {
  if(status === GroupProfitAndLossPage.Status.ERROR) {
    return ReportStatusIndicator.Status.NONE;
  }
  return status as number as ReportStatusIndicator.Status;
}

const BUTTON_STYLE: React.CSSProperties = {width: '140px'};
const ACTION_SHEET_GAP: React.CSSProperties = {height: '10px'};

function ProfitAndLossContent(props: {
      symbol: string;
      code: string;
      status: GroupProfitAndLossPage.Status;
      previousStatus: GroupProfitAndLossPage.Status;
      totalPnl: string;
      totalFees: string;
      totalVolume: string;
      accounts: GroupProfitAndLossPage.AccountEntry[];
      foreignCurrencies: GroupProfitAndLossPage.ExchangeRate[];
      startDate: Beam.Date;
      endDate: Beam.Date;
      onSubmit?: (start: Beam.Date, end: Beam.Date) => void;
    }): JSX.Element {
  const isLoading = props.status === GroupProfitAndLossPage.Status.IN_PROGRESS;
  const isEmpty = props.status === GroupProfitAndLossPage.Status.EMPTY;
  const isNoResults = props.status === GroupProfitAndLossPage.Status.NO_RESULTS;
  const isError = props.status === GroupProfitAndLossPage.Status.ERROR;
  if(isEmpty) {
    return (
      <section aria-label='Profit and Loss Report' aria-live='polite'>
        <EmptyMessage
          message='Click Apply to generate a report for the selected period.'/>
      </section>);
  }
  if(isNoResults) {
    return (
      <section aria-label='Profit and Loss Report' aria-live='polite'>
        <EmptyMessage message='There is no activity for the selected period.'/>
      </section>);
  }
  if(isError) {
    const onRetry = () => {
      props.onSubmit?.(props.startDate, props.endDate);
    };
    return (
      <section aria-label='Profit and Loss Report' aria-live='polite'>
        <ErrorMessage message='There was an error generating the report.'
          onRetry={onRetry}/>
      </section>);
  }
  return (
    <section aria-label='Profit and Loss Report' aria-live='polite'
        aria-busy={isLoading ? 'true' : 'false'}>
      <ProfitAndLossHeader
        symbol={props.symbol}
        code={props.code}
        totalPnl={props.totalPnl}
        totalFees={props.totalFees}
        totalVolume={props.totalVolume}
        foreignCurrencies={props.foreignCurrencies}
        loading={isLoading}/>
      <div className={css(STYLES.listSpacing)}/>
      <ProfitAndLossList
        symbol={props.symbol}
        code={props.code}
        status={props.status}
        previousStatus={props.previousStatus}
        accounts={props.accounts}/>
    </section>);
}

function ProfitAndLossList(props: {
    symbol: string;
    code: string;
    status: GroupProfitAndLossPage.Status;
    previousStatus: GroupProfitAndLossPage.Status;
    accounts: GroupProfitAndLossPage.AccountEntry[];
  }): JSX.Element {
  const isLoading =
    props.status === GroupProfitAndLossPage.Status.IN_PROGRESS;
  if(isLoading) {
    return (
      <ul className={css(STYLES.profitAndLossList)}>
        {Array.from({length: 5}, (_, i) =>
          <li key={i}><ProfitAndLossItemPlaceholder/></li>)}
      </ul>);
  }
  return (
    <ul className={css(STYLES.profitAndLossList)}>
      {props.accounts.map((account, i) =>
        <li key={i}>
          <AccountProfitAndLossItem
            username={account.username}
            symbol={props.symbol}
            code={props.code}
            totalPnl={account.totalPnl}
            currencies={account.currencies}/>
        </li>)}
    </ul>);
}

const STYLES = StyleSheet.create({
  main: {
    padding: '18px',
    fontFamily: '"Roboto", system-ui, sans-serif',
    fontWeight: 400,
    color: '#333333',
    '@media (max-width: 767px)': {
      paddingBottom: '166px'
    },
    '@media (min-width: 768px)': {
      paddingBottom: '40px'
    }
  },
  mainReducedPadding: {
    '@media (max-width: 767px)': {
      paddingBottom: '40px'
    }
  },
  formContainer: {
    overflow: 'hidden',
    transition: 'max-height 200ms ease-in-out'
  },
  contentSpacing: {
    height: '30px',
    flexShrink: 0
  },
  listSpacing: {
    height: '30px'
  },
  profitAndLossList: {
    padding: 0,
    listStyle: 'none',
    containerType: 'inline-size'
  },
  form: {
    '@media (max-width: 767px)': {
      display: 'flex',
      flexDirection: 'column'
    },
    '@media (min-width: 768px) and (max-width: 1035px)': {
      display: 'grid',
      gridTemplateColumns: '246px 1fr auto',
      gridTemplateRows: 'auto auto',
      columnGap: '18px',
      alignItems: 'start'
    },
    '@media (min-width: 1036px)': {
      display: 'grid',
      gridTemplateColumns: 'auto 1fr auto',
      columnGap: '18px',
      alignItems: 'start'
    }
  },
  selectWrapper: {
    '@media (min-width: 768px) and (max-width: 1035px)': {
      gridColumn: 1,
      gridRow: 1
    },
    '@media (min-width: 1036px)': {
      display: 'none'
    }
  },
  customDatesWrapper: {
    overflow: 'hidden',
    transition: 'max-height 200ms ease-in-out'
  },
  customDates: {
    '@media (min-width: 768px) and (max-width: 1035px)': {
      gridColumn: 1,
      gridRow: 2
    },
    '@media (min-width: 1036px)': {
      display: 'none'
    }
  },
  customDatesContent: {
    '@media (max-width: 767px)': {
      paddingTop: '18px'
    },
    '@media (min-width: 768px) and (max-width: 1035px)': {
      paddingTop: '12px'
    }
  },
  dateInputs: {
    '@media (max-width: 767px)': {
      display: 'grid',
      gridTemplateColumns: '1fr',
      gridTemplateRows: 'auto 12px auto 20px auto 12px auto auto',
    },
    '@media (min-width: 768px) and (max-width: 1035px)': {
      display: 'grid',
      gridTemplateColumns: '50px 8px 1fr',
      gridTemplateRows: 'auto 8px auto auto',
      alignItems: 'center'
    },
    '@media (min-width: 1036px)': {
      display: 'grid',
      gridTemplateColumns: 'auto 8px 150px 18px auto 8px 150px',
      gridTemplateRows: 'auto auto',
      alignItems: 'center'
    }
  },
  dateLabel: {
    fontSize: '0.875rem',
    paddingInlineStart: '10px',
    '@media (min-width: 768px)': {
      paddingInlineStart: 0
    }
  },
  startLabel: {
    '@media (max-width: 767px)': {
      gridRow: 1
    },
    '@media (min-width: 768px) and (max-width: 1035px)': {
      gridColumn: 1,
      gridRow: 1
    },
    '@media (min-width: 1036px)': {
      gridColumn: 1,
      gridRow: 1
    }
  },
  endLabel: {
    '@media (max-width: 767px)': {
      gridRow: 5
    },
    '@media (min-width: 768px) and (max-width: 1035px)': {
      gridColumn: 1,
      gridRow: 3
    },
    '@media (min-width: 1036px)': {
      gridColumn: 5,
      gridRow: 1
    }
  },
  startDate: {
    '@media (max-width: 767px)': {
      gridRow: 3
    },
    '@media (min-width: 768px) and (max-width: 1035px)': {
      gridColumn: 3,
      gridRow: 1
    },
    '@media (min-width: 1036px)': {
      gridColumn: 3,
      gridRow: 1
    }
  },
  endDate: {
    '@media (max-width: 767px)': {
      gridRow: 7
    },
    '@media (min-width: 768px) and (max-width: 1035px)': {
      gridColumn: 3,
      gridRow: 3
    },
    '@media (min-width: 1036px)': {
      gridColumn: 7,
      gridRow: 1
    }
  },
  validityCheckSection: {
    overflow: 'hidden',
    transition: 'max-height 200ms ease-in-out',
    '@media (max-width: 767px)': {
      gridRow: 8
    },
    '@media (min-width: 768px) and (max-width: 1035px)': {
      gridColumn: '1 / -1',
      gridRow: 4
    },
    '@media (min-width: 1036px)': {
      gridColumn: '1 / -1',
      gridRow: 2
    }
  },
  invalidMessage: {
    display: 'block',
    paddingTop: '12px',
    fontSize: '0.875rem',
    color: '#E63F44'
  },
  dateFilter: {
    '@media (max-width: 1035px)': {
      display: 'none'
    },
    '@media (min-width: 1036px)': {
      gridColumn: 1,
      display: 'flex',
      flexDirection: 'row',
      alignItems: 'flex-start'
    }
  },
  dateFilterSelect: {
    width: '200px',
    flexShrink: 0
  },
  dateFilterDates: {
    paddingInlineStart: '18px'
  },
  statusFeedback: {
    paddingTop: '18px',
    '@media (min-width: 768px)': {
      display: 'none'
    }
  },
  statusFeedbackHidden: {
    display: 'none'
  },
  actionsAndStatus: {
    '@media (max-width: 767px)': {
      display: 'none'
    },
    '@media (min-width: 768px) and (max-width: 1035px)': {
      display: 'flex',
      flexDirection: 'column',
      gridColumn: 3,
      gridRow: '1 / -1',
      alignSelf: 'stretch'
    },
    '@media (min-width: 1036px)': {
      display: 'flex',
      flexDirection: 'column',
      gridColumn: 3
    }
  },
  actionSheet: {
    position: 'fixed',
    inset: 'auto 0 0',
    backgroundColor: '#FFFFFF',
    padding: '18px 18px 30px',
    boxShadow: '0 0 6px rgb(0 0 0 / 25%)',
    display: 'flex',
    flexDirection: 'column',
    '@media (min-width: 768px)': {
      display: 'none'
    }
  },
  actionSheetButton: {
    width: '100%'
  },
  buttonRow: {
    display: 'flex',
    alignItems: 'center',
    gap: '18px'
  },
  desktopStatusFeedback: {
    paddingTop: '12px',
    display: 'flex',
    justifyContent: 'flex-end',
    '@media (min-width: 768px) and (max-width: 1035px)': {
      marginTop: 'auto'
    }
  },
  downloadLink: {
    display: 'flex',
    alignItems: 'center',
    justifyContent: 'center',
    backgroundColor: '#684BC7',
    color: '#FFFFFF',
    borderWidth: '1px',
    borderStyle: 'solid',
    borderColor: 'transparent',
    borderRadius: '1px',
    fontFamily: '"Roboto", system-ui, sans-serif',
    fontSize: '0.875rem',
    fontWeight: 400,
    padding: '3px 9px',
    width: '140px',
    height: '34px',
    boxSizing: 'border-box',
    cursor: 'pointer',
    outline: 'none',
    textDecoration: 'none',
    ':hover': {
      backgroundColor: '#4B23A0'
    },
    ':focus-visible': {
      backgroundColor: '#4B23A0'
    },
    ':active': {
      backgroundColor: '#4B23A0'
    }
  },
  downloadLinkDisabled: {
    opacity: 0.4,
    cursor: 'not-allowed',
    pointerEvents: 'none' as any
  }
});
