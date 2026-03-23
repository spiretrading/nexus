import { css, StyleSheet } from 'aphrodite/no-important';
import * as Beam from 'beam';
import * as React from 'react';
import { Button, DateInput, Select } from '../../..';
import { CurrencyTooltip } from './currency_tooltip';
import { ProfitAndLossHeader } from './profit_and_loss_header';
import { ProfitAndLossItem } from './profit_and_loss_item';
import { ProfitAndLossItemPlaceholder } from
  './profit_and_loss_item_placeholder';
import { ProfitAndLossTable } from './profit_and_loss_table';
import { ReportStatusIndicator } from './report_status_indicator';

interface Properties {

  /** The account's currency symbol. */
  symbol: string;

  /** The account's currency code. */
  code: string;

  /** The status of the report. */
  status: ProfitAndLossPage.Status;

  /** The status prior to a transition to IN_PROGRESS. */
  previousStatus: ProfitAndLossPage.Status;

  /** The input mode for dates. */
  mode: ProfitAndLossPage.Mode;

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

  /** The list of currencies with activity. */
  currencies: ProfitAndLossPage.CurrencyEntry[];

  /** The list of foreign currencies with exchange rates. */
  foreignCurrencies: CurrencyTooltip.ExchangeRate[];

  /** The filepath to the generated report. */
  filepath: string;

  /** Called when the mode changes. */
  onModeChange?: (mode: ProfitAndLossPage.Mode) => void;

  /** Called when the start date changes. */
  onStartDateChange?: (date: Beam.Date) => void;

  /** Called when the end date changes. */
  onEndDateChange?: (date: Beam.Date) => void;

  /** Called when the user submits. */
  onSubmit?: (start: Beam.Date, end: Beam.Date) => void;

  /** Called when the user cancels. */
  onCancel?: () => void;
}

/** Displays the profit and loss report page. */
export function ProfitAndLossPage(props: Properties) {
  const formRef = React.useRef<HTMLFormElement>(null);
  const isLoading = props.status === ProfitAndLossPage.Status.IN_PROGRESS;
  const isReady = props.status === ProfitAndLossPage.Status.READY;
  const isStale = props.status === ProfitAndLossPage.Status.STALE;
  const isError = props.status === ProfitAndLossPage.Status.ERROR;
  const isNone = props.status === ProfitAndLossPage.Status.NONE;
  const hasData = props.currencies.length > 0;
  const isCustom = props.mode === ProfitAndLossPage.Mode.CUSTOM;
  const showStatus = !(isNone || (isStale && !hasData));
  const reportStatus = (() => {
    if(isError) {
      return ReportStatusIndicator.Status.NONE;
    }
    return props.status as number as ReportStatusIndicator.Status;
  })();
  const onFormSubmit = (event: React.FormEvent) => {
    event.preventDefault();
    props.onSubmit?.(props.startDate, props.endDate);
  };
  const onSelectChange = (value: string) => {
    if(value === 'custom') {
      props.onModeChange?.(ProfitAndLossPage.Mode.CUSTOM);
    } else {
      props.onModeChange?.(ProfitAndLossPage.Mode.PRESET);
    }
  };
  const onRetry = () => {
    props.onSubmit?.(props.startDate, props.endDate);
  };
  const selectValue = (() => {
    if(isCustom) {
      return 'custom';
    }
    return 'this-month';
  })();
  const dateInputs = (
    <div className={css(STYLES.dateInputs)}>
      <div className={css(STYLES.dateGroup)}>
        <label htmlFor='start-date'
          className={css(STYLES.dateLabel)}>Start</label>
        <DateInput id='start-date' value={props.startDate}
          className={css(STYLES.dateInput)}
          onChange={props.onStartDateChange}/>
      </div>
      <div className={css(STYLES.dateGroup)}>
        <label htmlFor='end-date'
          className={css(STYLES.dateLabel)}>End</label>
        <DateInput id='end-date' value={props.endDate}
          className={css(STYLES.dateInput)}
          onChange={props.onEndDateChange}/>
      </div>
    </div>);
  const statusFeedback = (
    <div className={css(STYLES.statusFeedback,
        !showStatus && STYLES.statusFeedbackHidden)}>
      <ReportStatusIndicator id='report-status' status={reportStatus}/>
    </div>);
  const applyButton = (
    <Button label='Apply' type='submit' style={BUTTON_STYLE}
      aria-describedby={isStale ? 'report-status' : undefined}/>);
  const cancelButton = (
    <Button label='Cancel' type='button' style={BUTTON_STYLE}
      onClick={props.onCancel}/>);
  const downloadLink = (
    <a download href={props.filepath}
      className={css(STYLES.downloadLink)}>
      Download
    </a>);
  const actionSheet = (() => {
    const hideSheet = isReady && !hasData;
    if(hideSheet) {
      return null;
    }
    return (
      <section aria-label='Report Actions'
          className={css(STYLES.actionSheet)}>
        {isError &&
          <Button label='Retry' type='button'
            className={css(STYLES.actionSheetButton)} onClick={onRetry}/>}
        {isLoading &&
          <Button label='Cancel' type='button'
            className={css(STYLES.actionSheetButton)}
            onClick={props.onCancel}/>}
        {isReady &&
          <a download href={props.filepath}
            className={css(STYLES.downloadLink,
              STYLES.actionSheetButton)}>
            Download
          </a>}
        {isStale && hasData && <>
          <Button label='Apply' type='submit'
            className={css(STYLES.actionSheetButton)}/>
          <div style={ACTION_SHEET_GAP}/>
          <a download href={props.filepath}
            className={css(STYLES.downloadLink,
              STYLES.actionSheetButton)}>
            Download
          </a>
        </>}
        {isStale && !hasData &&
          <Button label='Apply' type='submit'
            className={css(STYLES.actionSheetButton)}/>}
        {isNone &&
          <Button label='Apply' type='submit'
            className={css(STYLES.actionSheetButton)}/>}
      </section>);
  })();
  const content = (() => {
    if(isNone || (isStale && !hasData)) {
      return (
        <div className={css(STYLES.emptyMessage)}>
          Click Apply to generate a report for the selected period.
        </div>);
    }
    if(isReady && !hasData) {
      return (
        <div className={css(STYLES.emptyMessage)}>
          There is no activity for the selected period.
        </div>);
    }
    if(isError) {
      return (
        <div className={css(STYLES.errorMessage)}>
          <img
            src='resources/account_page/profit_and_loss_page/error.svg'
            style={ERROR_ICON_STYLE}/>
          <span className={css(STYLES.errorText)}>
            There was an error generating the report.
          </span>
          <Button label='Retry' type='button' onClick={onRetry}
            className={css(STYLES.errorRetryButton)}/>
        </div>);
    }
    return (
      <section aria-label='Profit and Loss Report' aria-live='polite'
          aria-busy={isLoading}>
        <ProfitAndLossHeader
          symbol={props.symbol}
          code={props.code}
          totalPnl={props.totalPnl}
          totalFees={props.totalFees}
          totalVolume={props.totalVolume}
          foreignCurrencies={props.foreignCurrencies}
          loading={isLoading}/>
        <div style={LIST_SPACING}/>
        <ul className={css(STYLES.list)}>
          {isLoading ? renderPlaceholders() : renderItems(props)}
        </ul>
      </section>);
  })();
  return (
    <div className={css(STYLES.page)}>
      <main className={css(STYLES.main)}>
        <div className={css(STYLES.formContainer)}
            style={{maxHeight: formRef.current?.scrollHeight}}>
          <form ref={formRef} aria-label='Report Controls'
              onSubmit={onFormSubmit} className={css(STYLES.form)}>
            <div className={css(STYLES.dateFilterArea)}>
              <Select value={selectValue} aria-controls='custom-dates'
                  className={css(STYLES.select)} onChange={onSelectChange}>
                <option value='this-month'>This Month</option>
                <option value='last-month'>Last Month</option>
                <option value='custom'>Custom</option>
              </Select>
              {isCustom &&
                <div id='custom-dates' className={css(STYLES.customDates)}>
                  {dateInputs}
                </div>}
            </div>
            <div className={css(STYLES.mobileStatusFeedback)}>
              {statusFeedback}
            </div>
            <div className={css(STYLES.desktopActionsAndStatus)}>
              <div className={css(STYLES.buttonRow)}>
                {!isLoading && applyButton}
                {isLoading && cancelButton}
                {!isLoading && !(isReady || (isStale && hasData)) ? null :
                  !isLoading && downloadLink}
              </div>
              {statusFeedback}
            </div>
          </form>
        </div>
        <div style={CONTENT_SPACING}/>
        {content}
      </main>
      {actionSheet}
    </div>);
}

function renderPlaceholders(): JSX.Element[] {
  return Array.from({length: 5}, (_, i) =>
    <li key={i} className={css(STYLES.listItem)}>
      <ProfitAndLossItemPlaceholder/>
    </li>);
}

function renderItems(props: Properties): JSX.Element[] {
  return props.currencies.map(currency =>
    <li key={currency.code} className={css(STYLES.listItem)}>
      <ProfitAndLossItem
        symbol={currency.symbol}
        code={currency.code}
        totalPnl={currency.totalPnl}
        totalVolume={currency.totalVolume}
        totalFees={currency.totalFees}
        securities={currency.securities}/>
    </li>);
}

export namespace ProfitAndLossPage {

  /** Enumerates the possible states of the report. */
  export enum Status {

    /** No report has been generated. */
    NONE,

    /** The report is being generated. */
    IN_PROGRESS,

    /** The report is ready. */
    READY,

    /** The report is out of date. */
    STALE,

    /** There was an error generating the report. */
    ERROR
  }

  /** The input mode for the date range. */
  export enum Mode {

    /** Start and end date determined by a selected preset. */
    PRESET,

    /** Start and end date are user-specified. */
    CUSTOM
  }

  /** A currency entry within the report. */
  export interface CurrencyEntry {

    /** The currency symbol. */
    symbol: string;

    /** The currency code. */
    code: string;

    /** The total profit and loss. */
    totalPnl: string;

    /** The total volume traded. */
    totalVolume: string;

    /** The total fees incurred. */
    totalFees: string;

    /** The list of securities traded. */
    securities: ProfitAndLossTable.Security[];
  }
}

const ERROR_ICON_STYLE: React.CSSProperties = {
  width: '44px',
  height: '44px'
};

const BUTTON_STYLE: React.CSSProperties = {
  width: '140px'
};

const ACTION_SHEET_GAP: React.CSSProperties = {
  height: '10px'
};

const LIST_SPACING: React.CSSProperties = {
  height: '30px'
};

const CONTENT_SPACING: React.CSSProperties = {
  height: '30px'
};

const STYLES = StyleSheet.create({
  page: {
    display: 'flex',
    justifyContent: 'center'
  },
  main: {
    width: 'min(100%, 460px)',
    padding: '18px 18px 40px',
    fontFamily: "'Roboto', system-ui, sans-serif",
    fontWeight: 400,
    color: '#333333',
    '@media (min-width: 768px) and (max-width: 1035px)': {
      width: '768px'
    },
    '@media (min-width: 1036px)': {
      width: '1036px'
    }
  },
  formContainer: {
    overflow: 'hidden',
    transition: 'max-height 200ms ease-in-out'
  },
  form: {
    display: 'flex',
    flexDirection: 'column',
    '@media (min-width: 768px) and (max-width: 1035px)': {
      flexDirection: 'row',
      alignItems: 'flex-start'
    },
    '@media (min-width: 1036px)': {
      flexDirection: 'row',
      alignItems: 'flex-start'
    }
  },
  dateFilterArea: {
    display: 'flex',
    flexDirection: 'column',
    '@media (min-width: 768px) and (max-width: 1035px)': {
      width: '246px',
      flexShrink: 0
    },
    '@media (min-width: 1036px)': {
      flexDirection: 'row',
      alignItems: 'center'
    }
  },
  select: {
    '@media (min-width: 1036px)': {
      width: '200px'
    }
  } as any,
  customDates: {
    paddingTop: '18px',
    '@media (min-width: 768px) and (max-width: 1035px)': {
      paddingTop: '12px'
    },
    '@media (min-width: 1036px)': {
      paddingTop: 0,
      paddingInlineStart: '18px'
    }
  },
  dateInputs: {
    display: 'flex',
    flexDirection: 'column',
    gap: '20px',
    '@media (min-width: 768px) and (max-width: 1035px)': {
      gap: '8px'
    },
    '@media (min-width: 1036px)': {
      flexDirection: 'row',
      alignItems: 'center',
      gap: '18px'
    }
  },
  dateGroup: {
    display: 'flex',
    flexDirection: 'column',
    gap: '12px',
    '@media (min-width: 768px) and (max-width: 1035px)': {
      flexDirection: 'row',
      alignItems: 'center',
      gap: '8px'
    },
    '@media (min-width: 1036px)': {
      flexDirection: 'row',
      alignItems: 'center',
      gap: '8px'
    }
  },
  dateLabel: {
    fontSize: '0.875rem',
    paddingInlineStart: '10px',
    '@media (min-width: 768px)': {
      paddingInlineStart: 0
    }
  },
  dateInput: {
    '@media (min-width: 1036px)': {
      width: '150px'
    }
  } as any,
  mobileStatusFeedback: {
    '@media (min-width: 768px)': {
      display: 'none'
    }
  },
  desktopActionsAndStatus: {
    display: 'none',
    '@media (min-width: 768px)': {
      display: 'flex',
      flexDirection: 'column',
      marginInlineStart: 'auto',
      paddingInlineStart: '18px'
    }
  },
  buttonRow: {
    display: 'flex',
    alignItems: 'center',
    gap: '18px'
  },
  statusFeedback: {
    paddingTop: '18px',
    '@media (min-width: 768px)': {
      paddingTop: '12px',
      display: 'flex',
      justifyContent: 'flex-end'
    }
  },
  statusFeedbackHidden: {
    display: 'none'
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
  } as any,
  actionSheetButton: {
    width: '100%'
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
    fontFamily: "'Roboto', system-ui, sans-serif",
    fontSize: '0.875rem',
    fontWeight: 500,
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
    ':focus': {
      backgroundColor: '#4B23A0'
    },
    ':active': {
      backgroundColor: '#4B23A0'
    }
  },
  list: {
    padding: 0,
    margin: 0,
    listStyle: 'none',
    containerType: 'inline-size'
  } as any,
  listItem: {
    padding: 0
  },
  emptyMessage: {
    textAlign: 'center',
    padding: '80px 0'
  },
  errorMessage: {
    display: 'flex',
    flexDirection: 'column',
    alignItems: 'center',
    gap: '18px',
    padding: '40px 0',
    '@media (min-width: 768px)': {
      padding: '47px 0'
    }
  },
  errorText: {
    textAlign: 'center'
  },
  errorRetryButton: {
    display: 'none',
    '@media (min-width: 768px)': {
      display: 'inline-block',
      width: '246px'
    }
  }
});
