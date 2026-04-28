import { css, StyleSheet } from 'aphrodite/no-important';
import * as Beam from 'beam';
import * as React from 'react';
import { PageLayout, Select } from '../../..';

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
          <ProfitAndLossContent/>
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
    totalProfitAndLoss: string;

    /** The total volume traded. */
    totalVolume: string;

    /** The total fees incurred. */
    totalFees: string;

    /** The list of tickers traded. */
    tickers: TickerEntry[];
  }

  /** A ticker entry within a currency. */
  export interface TickerEntry {

    /** The symbol of the ticker. */
    symbol: string;

    /** The volume traded. */
    volume: string;

    /** The fees incurred. */
    fees: string;

    /** The profit and loss. */
    pnl: string;
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
  public render(): JSX.Element {
    return (
      <div ref={this._containerRef} className={css(STYLES.formContainer)}>
        <Form ref={this._formRef}
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
    const container = this._containerRef.current;
    const form = this._formRef.current?.element;
    if(!container || !form) {
      return;
    }
    container.style.maxHeight = `${form.scrollHeight}px`;
    this._observer = new ResizeObserver(() => {
      container.style.maxHeight = `${form.scrollHeight}px`;
    });
    this._observer.observe(form);
  }

  public componentWillUnmount(): void {
    this._observer?.disconnect();
  }

  private _containerRef = React.createRef<HTMLDivElement>();
  private _formRef = React.createRef<Form>();
  private _observer: ResizeObserver;
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
  public get element(): HTMLFormElement {
    return this._formRef.current;
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
      <form ref={this._formRef} aria-label='Report Controls'
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
        <CustomDates isOpen={isCustom}/>
        <DateFilter/>
        <StatusFeedback/>
        <ActionsAndStatus/>
        {!hideActionSheet && <ActionSheet/>}
      </form>);
  }

  private _formRef = React.createRef<HTMLFormElement>();
}

function CustomDates(props: {isOpen: boolean}): JSX.Element {
  return <div className={css(STYLES.customDates)}/>;
}

function DateFilter(): JSX.Element {
  return <div className={css(STYLES.dateFilter)}/>;
}

function StatusFeedback(): JSX.Element {
  return <div className={css(STYLES.statusFeedback)}/>;
}

function ActionsAndStatus(): JSX.Element {
  return <div className={css(STYLES.actionsAndStatus)}/>;
}

function ActionSheet(): JSX.Element {
  return <section className={css(STYLES.actionSheet)}/>;
}

function ProfitAndLossContent(): JSX.Element {
  return (
    <section aria-label='Profit and Loss Report' aria-live='polite'
        aria-busy='false'>
    </section>);
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
  form: {
    '@media (max-width: 767px)': {
      display: 'flex',
      flexDirection: 'column'
    },
    '@media (min-width: 768px) and (max-width: 1035px)': {
      display: 'grid',
      gridTemplateColumns: '246px 1fr auto',
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
  customDates: {
    '@media (min-width: 768px) and (max-width: 1035px)': {
      gridColumn: 1,
      gridRow: 2
    },
    '@media (min-width: 1036px)': {
      display: 'none'
    }
  },
  dateFilter: {
    '@media (max-width: 1035px)': {
      display: 'none'
    },
    '@media (min-width: 1036px)': {
      gridColumn: 1
    }
  },
  statusFeedback: {
    '@media (min-width: 768px)': {
      display: 'none'
    }
  },
  actionsAndStatus: {
    '@media (max-width: 767px)': {
      display: 'none'
    },
    '@media (min-width: 768px) and (max-width: 1035px)': {
      gridColumn: 3,
      gridRow: '1 / -1'
    },
    '@media (min-width: 1036px)': {
      gridColumn: 3
    }
  },
  actionSheet: {
    position: 'fixed',
    inset: 'auto 0 0',
    backgroundColor: '#FFFFFF',
    padding: '18px 18px 30px',
    boxShadow: '0 0 6px rgb(0 0 0 / 25%)',
    '@media (min-width: 768px)': {
      display: 'none'
    }
  }
});
