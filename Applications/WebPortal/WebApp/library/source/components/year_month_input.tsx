import { css, StyleSheet } from 'aphrodite/no-important';
import * as Beam from 'beam';
import * as React from 'react';
import { IntegerInput } from './integer_input';

interface Properties extends
    Omit<React.HTMLAttributes<HTMLDivElement>, 'onChange'> {

  /** The year and month to display. The day is ignored. */
  value?: Beam.Date;

  /** Whether the value can only be read. */
  readOnly?: boolean;

  /** Whether interaction is disabled. */
  disabled?: boolean;

  /** Whether to display an error border. */
  error?: boolean;

  /** Reports a date with day 1, or null when both fields are empty. */
  onChange?: (value: Beam.Date) => void;
}

interface State {
  year: number;
  month: number;
  revision: number;
}

/** Edits a calendar year and month without requiring a day. */
export class YearMonthInput extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {
      year: props.value?.year,
      month: props.value?.month,
      revision: 0
    };
    this.published = undefined;
  }

  public render(): JSX.Element {
    const {id, className, value, readOnly, disabled, error, onChange,
      ...attributes} = this.props;
    const invalid = error || attributes['aria-invalid'] === true ||
      attributes['aria-invalid'] === 'true';
    return <div {...attributes}
        className={[css(STYLES.container,
          disabled && STYLES.disabled, readOnly && STYLES.readOnly,
          invalid && STYLES.error), className].filter(Boolean).join(' ')}>
      <IntegerInput key={`year-${this.state.revision}`} id={id}
        aria-label='Year' placeholder='YYYY'
        aria-invalid={attributes['aria-invalid']}
        aria-describedby={attributes['aria-describedby']}
        min={MINIMUM_YEAR} max={MAXIMUM_YEAR} leadingZeros={4}
        value={this.state.year} readOnly={readOnly} disabled={disabled}
        onChange={this.onYearChange} style={STYLE.year}/>
      <span className={css(STYLES.separator)} aria-hidden>-</span>
      <IntegerInput key={`month-${this.state.revision}`}
        aria-label='Month' placeholder='MM'
        aria-invalid={attributes['aria-invalid']}
        aria-describedby={attributes['aria-describedby']}
        min={1} max={12} leadingZeros={2}
        value={this.state.month} readOnly={readOnly} disabled={disabled}
        onChange={this.onMonthChange} style={STYLE.month}/>
    </div>;
  }

  public componentDidUpdate(previous: Properties): void {
    if(previous.value !== this.props.value &&
        this.props.value !== this.published) {
      this.setState(state => ({year: this.props.value?.year,
        month: this.props.value?.month, revision: state.revision + 1}));
    }
    this.published = undefined;
  }

  private publish(): void {
    const {year, month} = this.state;
    if(Number.isInteger(year) && year >= MINIMUM_YEAR && year <= MAXIMUM_YEAR &&
        Number.isInteger(month) && month >= 1 && month <= 12) {
      this.published = new Beam.Date(year, month, 1);
    } else if(year == null && month == null) {
      this.published = null;
    } else {
      return;
    }
    this.props.onChange?.(this.published);
  }

  private onYearChange = (year: number) => {
    this.setState({year}, () => this.publish());
  };

  private onMonthChange = (month: number) => {
    this.setState({month}, () => this.publish());
  };

  private published: Beam.Date;
}

const MINIMUM_YEAR = 1400;
const MAXIMUM_YEAR = 9999;

const STYLES = StyleSheet.create({
  container: {
    backgroundColor: '#FFFFFF',
    border: '1px solid #C8C8C8',
    borderRadius: '1px',
    color: '#000000',
    fontSize: '0.875rem',
    padding: '9px 10px',
    display: 'flex',
    alignItems: 'center',
    boxSizing: 'border-box',
    height: '34px',
    ':hover': {
      borderColor: '#684BC7'
    },
    ':focus-within': {
      borderColor: '#684BC7'
    }
  },
  disabled: {
    opacity: 0.4,
    cursor: 'not-allowed',
    pointerEvents: 'none'
  },
  readOnly: {
    borderColor: 'transparent',
    ':hover': {
      borderColor: 'transparent'
    },
    ':focus-within': {
      borderColor: 'transparent'
    }
  },
  error: {
    borderColor: '#E63F44',
    ':hover': {
      borderColor: '#E63F44'
    },
    ':focus-within': {
      borderColor: '#E63F44'
    }
  },
  separator: {
    width: '0.75em',
    textAlign: 'center',
    userSelect: 'none'
  }
});

const FIELD_STYLE: React.CSSProperties = {
  backgroundColor: 'transparent',
  border: 'none',
  padding: 0,
  textAlign: 'center',
  font: 'inherit',
  flexShrink: 0
};

const STYLE: Record<string, React.CSSProperties> = {
  year: {
    ...FIELD_STYLE,
    width: '3em'
  },
  month: {
    ...FIELD_STYLE,
    width: '2em'
  }
};
