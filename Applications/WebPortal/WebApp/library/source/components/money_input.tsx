import { css, StyleSheet } from 'aphrodite/no-important';
import * as Nexus from 'nexus';
import * as React from 'react';

interface Properties extends
    Omit<React.InputHTMLAttributes<HTMLInputElement>,
      'min' | 'max' | 'value' | 'onChange'> {

  /** The minimum allowed value (inclusive). */
  min?: Nexus.Money;

  /** The maximum allowed value (inclusive). */
  max?: Nexus.Money;

  /** The value to display. */
  value?: Nexus.Money;

  /** The event handler for when a change is made. */
  onChange?: (value?: Nexus.Money) => void;
}

interface State {
  text: string;
}

/** An editable money input. */
export class MoneyInput extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {
      text: props.value?.toString() ?? ''
    };
  }

  public render(): JSX.Element {
    const {min, max, value, readOnly, onChange, style, className, ...rest} =
      this.props;
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
        text: this.props.value?.toString() ?? ''
      });
    }
    if(this._start != null) {
      this._input.setSelectionRange(this._start, this._end);
      this._start = null;
      this._end = null;
    }
  }

  private onKeyDown = (event: React.KeyboardEvent<HTMLInputElement>) => {
    if(event.key === 'ArrowUp') {
      this.increment();
    } else if(event.key === 'ArrowDown') {
      this.decrement();
    }
  }

  private onWheel = (event: React.WheelEvent<HTMLInputElement>) => {
    if(document.activeElement !== event.target) {
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
    const parsed = Nexus.Money.parse(this.state.text);
    const value = (() => {
      if(parsed === null) {
        return this.props.value ?? Nexus.Money.ZERO;
      } else if(this.props.min && parsed.compare(this.props.min) < 0) {
        return this.props.min;
      } else if(this.props.max && parsed.compare(this.props.max) > 0) {
        return this.props.max;
      } else {
        return parsed;
      }
    })();
    if(!value.equals(this.props.value)) {
      this.props.onChange?.(value);
    }
  }

  private increment = () => {
    const current = this.props.value ?? Nexus.Money.ZERO;
    const increment = current.add(Nexus.Money.ONE);
    if(this.props.max && increment.compare(this.props.max) > 0) {
      return;
    }
    this.props.onChange?.(increment);
  }

  private decrement = () => {
    const current = this.props.value ?? Nexus.Money.ZERO;
    const decrement = current.subtract(Nexus.Money.ONE);
    if(this.props.min && decrement.compare(this.props.min) < 0) {
      return;
    }
    this.props.onChange?.(decrement);
  }

  private _input: HTMLInputElement;
  private _start: number;
  private _end: number;
}

type MoneyLabelProperties = Omit<Properties, 'readOnly' | 'onChange'>;

/** A read-only money display. */
export function MoneyLabel(props: MoneyLabelProperties): JSX.Element {
  return <MoneyInput {...props} readOnly/>;
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
