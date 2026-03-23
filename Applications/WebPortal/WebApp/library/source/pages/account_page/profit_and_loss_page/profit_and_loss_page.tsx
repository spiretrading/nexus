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
  const onFormSubmit = (event: React.FormEvent) => {
    event.preventDefault();
    props.onSubmit?.(props.startDate, props.endDate);
  };
  return (
    <div className={css(STYLES.page)}>
      <main className={css(STYLES.main)}>
        <FormContainer scrollHeight={formRef.current?.scrollHeight}>
          <form ref={formRef} aria-label='Report Controls'
              onSubmit={onFormSubmit} className={css(STYLES.form)}>
            <DateFilterArea
              mode={props.mode}
              startDate={props.startDate}
              endDate={props.endDate}
              onModeChange={props.onModeChange}
              onStartDateChange={props.onStartDateChange}
              onEndDateChange={props.onEndDateChange}/>
            <MobileStatusFeedback
              status={props.status}
              currencies={props.currencies}/>
            <ActionsAndStatus
              status={props.status}
              currencies={props.currencies}
              filepath={props.filepath}
              isLoading={isLoading}
              onCancel={props.onCancel}/>
          </form>
        </FormContainer>
        <div style={CONTENT_SPACING}/>
        <ProfitAndLossContent
          symbol={props.symbol}
          code={props.code}
          status={props.status}
          totalPnl={props.totalPnl}
          totalFees={props.totalFees}
          totalVolume={props.totalVolume}
          currencies={props.currencies}
          foreignCurrencies={props.foreignCurrencies}
          startDate={props.startDate}
          endDate={props.endDate}
          onSubmit={props.onSubmit}/>
      </main>
      <ActionSheet
        status={props.status}
        currencies={props.currencies}
        filepath={props.filepath}
        startDate={props.startDate}
        endDate={props.endDate}
        onSubmit={props.onSubmit}
        onCancel={props.onCancel}/>
    </div>);
}

/** Div:FormContainer — animates form height changes. */
function FormContainer(props: {
    scrollHeight: number;
    children: React.ReactNode;
  }) {
  return (
    <div className={css(STYLES.formContainer)}
      style={{maxHeight: props.scrollHeight}}>
      {props.children}
    </div>);
}

/** Div:DateFilterArea — contains Select and CustomDates.
 *  Maps to DateFilter at >= 1036px, or direct Select+CustomDates at smaller
 *  breakpoints. */
function DateFilterArea(props: {
    mode: ProfitAndLossPage.Mode;
    startDate: Beam.Date;
    endDate: Beam.Date;
    onModeChange?: (mode: ProfitAndLossPage.Mode) => void;
    onStartDateChange?: (date: Beam.Date) => void;
    onEndDateChange?: (date: Beam.Date) => void;
  }) {
  const isCustom = props.mode === ProfitAndLossPage.Mode.CUSTOM;
  const onSelectChange = (value: string) => {
    if(value === 'custom') {
      props.onModeChange?.(ProfitAndLossPage.Mode.CUSTOM);
    } else {
      props.onModeChange?.(ProfitAndLossPage.Mode.PRESET);
    }
  };
  const selectValue = isCustom ? 'custom' : 'this-month';
  return (
    <div className={css(STYLES.dateFilterArea)}>
      <Select value={selectValue} aria-controls='custom-dates'
          className={css(STYLES.select)} onChange={onSelectChange}>
        <option value='this-month'>This Month</option>
        <option value='last-month'>Last Month</option>
        <option value='custom'>Custom</option>
      </Select>
      {isCustom &&
        <CustomDates
          startDate={props.startDate}
          endDate={props.endDate}
          onStartDateChange={props.onStartDateChange}
          onEndDateChange={props.onEndDateChange}/>}
    </div>);
}

/** Div:CustomDates — wraps DateInputs with responsive padding. */
function CustomDates(props: {
    startDate: Beam.Date;
    endDate: Beam.Date;
    onStartDateChange?: (date: Beam.Date) => void;
    onEndDateChange?: (date: Beam.Date) => void;
  }) {
  return (
    <div id='custom-dates' className={css(STYLES.customDates)}>
      <DateInputs
        startDate={props.startDate}
        endDate={props.endDate}
        onStartDateChange={props.onStartDateChange}
        onEndDateChange={props.onEndDateChange}/>
    </div>);
}

/** Div:DateInputs — responsive date input groups. */
function DateInputs(props: {
    startDate: Beam.Date;
    endDate: Beam.Date;
    onStartDateChange?: (date: Beam.Date) => void;
    onEndDateChange?: (date: Beam.Date) => void;
  }) {
  return (
    <div className={css(STYLES.dateInputs)}>
      <DateGroup label='Start' id='start-date'
        value={props.startDate} onChange={props.onStartDateChange}/>
      <DateGroup label='End' id='end-date'
        value={props.endDate} onChange={props.onEndDateChange}/>
    </div>);
}

/** Div:StartDateGroup / Div:EndDateGroup — label + input pair. */
function DateGroup(props: {
    label: string;
    id: string;
    value: Beam.Date;
    onChange?: (date: Beam.Date) => void;
  }) {
  return (
    <div className={css(STYLES.dateGroup)}>
      <label htmlFor={props.id}
        className={css(STYLES.dateLabel)}>{props.label}</label>
      <DateInput id={props.id} value={props.value}
        className={css(STYLES.dateInput)} onChange={props.onChange}/>
    </div>);
}

/** Div:StatusFeedback — wraps ReportStatusIndicator with padding. */
function StatusFeedback(props: {
    status: ProfitAndLossPage.Status;
    currencies: ProfitAndLossPage.CurrencyEntry[];
  }) {
  const isNone = props.status === ProfitAndLossPage.Status.NONE;
  const isStale = props.status === ProfitAndLossPage.Status.STALE;
  const isError = props.status === ProfitAndLossPage.Status.ERROR;
  const hasData = props.currencies.length > 0;
  const hidden = isNone || (isStale && !hasData);
  const reportStatus = (() => {
    if(isError) {
      return ReportStatusIndicator.Status.NONE;
    }
    return props.status as number as ReportStatusIndicator.Status;
  })();
  return (
    <div className={css(STYLES.statusFeedback,
        hidden && STYLES.statusFeedbackHidden)}>
      <ReportStatusIndicator id='report-status' status={reportStatus}/>
    </div>);
}

/** Mobile-only StatusFeedback wrapper — hidden at >= 768px. */
function MobileStatusFeedback(props: {
    status: ProfitAndLossPage.Status;
    currencies: ProfitAndLossPage.CurrencyEntry[];
  }) {
  return (
    <div className={css(STYLES.mobileStatusFeedback)}>
      <StatusFeedback status={props.status} currencies={props.currencies}/>
    </div>);
}

/** Div:ActionsAndStatus — desktop buttons + StatusFeedback.
 *  Hidden on mobile, visible at >= 768px. */
function ActionsAndStatus(props: {
    status: ProfitAndLossPage.Status;
    currencies: ProfitAndLossPage.CurrencyEntry[];
    filepath: string;
    isLoading: boolean;
    onCancel?: () => void;
  }) {
  const isReady = props.status === ProfitAndLossPage.Status.READY;
  const isStale = props.status === ProfitAndLossPage.Status.STALE;
  const hasData = props.currencies.length > 0;
  const showDownload = !props.isLoading && (isReady || (isStale && hasData));
  return (
    <div className={css(STYLES.desktopActionsAndStatus)}>
      <div className={css(STYLES.buttonRow)}>
        {!props.isLoading &&
          <Button label='Apply' type='submit' style={BUTTON_STYLE}
            aria-describedby={isStale ? 'report-status' : undefined}/>}
        {props.isLoading &&
          <Button label='Cancel' type='button' style={BUTTON_STYLE}
            onClick={props.onCancel}/>}
        {showDownload &&
          <a download href={props.filepath}
            className={css(STYLES.downloadLink)}>
            Download
          </a>}
      </div>
      <StatusFeedback status={props.status} currencies={props.currencies}/>
    </div>);
}

/** Section:ActionSheet — mobile fixed-bottom action overlay.
 *  Hidden at >= 768px. */
function ActionSheet(props: {
    status: ProfitAndLossPage.Status;
    currencies: ProfitAndLossPage.CurrencyEntry[];
    filepath: string;
    startDate: Beam.Date;
    endDate: Beam.Date;
    onSubmit?: (start: Beam.Date, end: Beam.Date) => void;
    onCancel?: () => void;
  }) {
  const isLoading = props.status === ProfitAndLossPage.Status.IN_PROGRESS;
  const isReady = props.status === ProfitAndLossPage.Status.READY;
  const isStale = props.status === ProfitAndLossPage.Status.STALE;
  const isError = props.status === ProfitAndLossPage.Status.ERROR;
  const isNone = props.status === ProfitAndLossPage.Status.NONE;
  const hasData = props.currencies.length > 0;
  if(isReady && !hasData) {
    return null;
  }
  const onRetry = () => {
    props.onSubmit?.(props.startDate, props.endDate);
  };
  const fullWidthStyle = css(STYLES.actionSheetButton);
  const fullWidthDownload = css(STYLES.downloadLink, STYLES.actionSheetButton);
  return (
    <section aria-label='Report Actions'
        className={css(STYLES.actionSheet)}>
      {isError &&
        <Button label='Retry' type='button'
          className={fullWidthStyle} onClick={onRetry}/>}
      {isLoading &&
        <Button label='Cancel' type='button'
          className={fullWidthStyle} onClick={props.onCancel}/>}
      {isReady &&
        <a download href={props.filepath} className={fullWidthDownload}>
          Download
        </a>}
      {isStale && hasData && <>
        <Button label='Apply' type='submit'
          className={fullWidthStyle}/>
        <div style={ACTION_SHEET_GAP}/>
        <a download href={props.filepath} className={fullWidthDownload}>
          Download
        </a>
      </>}
      {isStale && !hasData &&
        <Button label='Apply' type='submit'
          className={fullWidthStyle}/>}
      {isNone &&
        <Button label='Apply' type='submit'
          className={fullWidthStyle}/>}
    </section>);
}

/** Section:ProfitAndLossContent — report content area. */
function ProfitAndLossContent(props: {
    symbol: string;
    code: string;
    status: ProfitAndLossPage.Status;
    totalPnl: string;
    totalFees: string;
    totalVolume: string;
    currencies: ProfitAndLossPage.CurrencyEntry[];
    foreignCurrencies: CurrencyTooltip.ExchangeRate[];
    startDate: Beam.Date;
    endDate: Beam.Date;
    onSubmit?: (start: Beam.Date, end: Beam.Date) => void;
  }) {
  const isLoading = props.status === ProfitAndLossPage.Status.IN_PROGRESS;
  const isReady = props.status === ProfitAndLossPage.Status.READY;
  const isStale = props.status === ProfitAndLossPage.Status.STALE;
  const isError = props.status === ProfitAndLossPage.Status.ERROR;
  const isNone = props.status === ProfitAndLossPage.Status.NONE;
  const hasData = props.currencies.length > 0;
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
      <ErrorMessage
        startDate={props.startDate}
        endDate={props.endDate}
        onSubmit={props.onSubmit}/>);
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
      <ProfitAndLossList
        currencies={props.currencies}
        isLoading={isLoading}/>
    </section>);
}

/** Div:ErrorMessage — error display with icon, text, and retry. */
function ErrorMessage(props: {
    startDate: Beam.Date;
    endDate: Beam.Date;
    onSubmit?: (start: Beam.Date, end: Beam.Date) => void;
  }) {
  const onRetry = () => {
    props.onSubmit?.(props.startDate, props.endDate);
  };
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

/** Ul:ProfitAndLossList — list of currency items or placeholders. */
function ProfitAndLossList(props: {
    currencies: ProfitAndLossPage.CurrencyEntry[];
    isLoading: boolean;
  }) {
  return (
    <ul className={css(STYLES.list)}>
      {props.isLoading ?
        Array.from({length: 5}, (_, i) =>
          <li key={i} className={css(STYLES.listItem)}>
            <ProfitAndLossItemPlaceholder/>
          </li>) :
        props.currencies.map(currency =>
          <li key={currency.code} className={css(STYLES.listItem)}>
            <ProfitAndLossItem
              symbol={currency.symbol}
              code={currency.code}
              totalPnl={currency.totalPnl}
              totalVolume={currency.totalVolume}
              totalFees={currency.totalFees}
              securities={currency.securities}/>
          </li>)}
    </ul>);
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
    '@media (min-width: 768px) and (max-width: 1035px)': {
      paddingInlineStart: 0,
      width: '50px',
      flexShrink: 0
    },
    '@media (min-width: 1036px)': {
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
