import { css, StyleSheet } from 'aphrodite/no-important';
import * as React from 'react';

interface Properties extends
    Omit<React.InputHTMLAttributes<HTMLInputElement>,
      'min' | 'max' | 'value' | 'onChange'> {

  /** The minimum allowed value (inclusive). */
  min?: number;

  /** The maximum allowed value (inclusive). */
  max?: number;

  /** The value to display. */
  value?: number;

  /** The number of digits to display after the decimal point. */
  decimalPlaces?: number;

  /** Called when the value changes.
   * @param value - The updated value.
   */
  onChange?: (value?: number) => void;
}

interface State {
  text: string;
}

/** An editable decimal number input. */
export class DecimalInput extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {
      text: formatValue(props.value, props.decimalPlaces)
    };
  }

  public render(): JSX.Element {
    const {min, max, value, decimalPlaces, readOnly, onChange, style, className,
      ...rest} = this.props;
    return (
      <input
        {...rest}
        style={{...STYLE.input, ...style}}
        className={[css(STYLES.effects), className].join(' ')}
        type='text'
        disabled={readOnly}
        ref={(input) => {this._input = input;}}
        value={this.state.text}
        onKeyDown={this.onKeyDown} onWheel={this.onWheel}
        onChange={this.onChange} onBlur={this.onBlur}/>);
  }

  public componentDidUpdate(prevProps: Properties) {
    if(this.props.value !== prevProps.value) {
      this.setState({
        text: formatValue(this.props.value, this.props.decimalPlaces)
      });
    }
    if(this._start != null) {
      this._input.setSelectionRange(this._start, this._end);
      this._start = null;
      this._end = null;
    }
  }

  private onKeyDown = (event: React.KeyboardEvent<HTMLInputElement>) => {
    if(this.props.readOnly) {
      return;
    }
    if(event.key === 'ArrowUp') {
      this.increment();
    } else if(event.key === 'ArrowDown') {
      this.decrement();
    }
  }

  private onWheel = (event: React.WheelEvent<HTMLInputElement>) => {
    if(this.props.readOnly || document.activeElement !== event.target) {
      return;
    }
    if(event.deltaY > 0) {
      this.decrement();
    } else if(event.deltaY < 0) {
      this.increment();
    }
  }

  private onChange = (event: React.ChangeEvent<HTMLInputElement>) => {
    const text = event.target.value.replace(/[^0-9.\-]/g, '');
    this.setState({text});
  }

  private onBlur = () => {
    if(this.state.text === '') {
      if(this.props.value != null) {
        this.props.onChange?.(undefined);
      }
      return;
    }
    const parsed = parseFloat(this.state.text);
    const value = (() => {
      if(isNaN(parsed)) {
        return this.props.value ?? 0;
      } else if(this.props.min != null && parsed < this.props.min) {
        return this.props.min;
      } else if(this.props.max != null && parsed > this.props.max) {
        return this.props.max;
      } else {
        return parsed;
      }
    })();
    if(value !== this.props.value) {
      this.props.onChange?.(value);
    }
  }

  private increment = () => {
    const current = this.props.value ?? 0;
    const increment = current + 1;
    if(this.props.max != null && increment > this.props.max) {
      return;
    }
    this._start = this._input.selectionStart;
    this._end = this._input.selectionEnd;
    this.props.onChange?.(increment);
  }

  private decrement = () => {
    const current = this.props.value ?? 0;
    const decrement = current - 1;
    if(this.props.min != null && decrement < this.props.min) {
      return;
    }
    this._start = this._input.selectionStart;
    this._end = this._input.selectionEnd;
    this.props.onChange?.(decrement);
  }

  private _input: HTMLInputElement;
  private _start: number;
  private _end: number;
}

type DecimalLabelProperties = Omit<Properties, 'readOnly' | 'onChange'>;

function formatValue(value?: number, decimalPlaces?: number): string {
  if(value == null) {
    return '';
  }
  if(decimalPlaces != null) {
    return value.toFixed(decimalPlaces);
  }
  return value.toString();
}

/** A read-only decimal display. */
export function DecimalLabel(props: DecimalLabelProperties): JSX.Element {
  return <DecimalInput {...props} readOnly/>;
}

const STYLE: Record<string, React.CSSProperties> = {
  input: {
    boxSizing: 'border-box',
    height: '34px',
    border: '1px solid #C8C8C8',
    borderRadius: '1px',
    font: '400 14px Roboto',
    backgroundColor: '#FFFFFF',
    color: '#333333',
    flexGrow: 1,
    maxWidth: '246px',
    width: '100%',
    paddingLeft: '10px'
  }
};

const STYLES = StyleSheet.create({
  effects: {
    ':focus': {
      borderColor: '#684BC7',
      outline: 'none',
      boxShadow: 'none'
    }
  }
});
