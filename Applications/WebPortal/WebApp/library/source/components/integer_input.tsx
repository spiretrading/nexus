import * as React from 'react';
import { Input } from './input';

interface Properties extends Omit<React.InputHTMLAttributes<HTMLInputElement>,
    'min' | 'max' | 'value' | 'onChange'> {

  /** The minimum allowed value. */
  min?: number;

  /** The maximum allowed value. */
  max?: number;

  /** The number of leading zeros to display. */
  leadingZeros?: number;

  /** The value to display. */
  value?: number;

  /** The event handler for when a change is made. */
  onChange?: (value?: number) => void;
}

interface State {
  text: string;
}

/** An editable integer field. */
export class IntegerInput extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {
      text: padValue(props.value, props.leadingZeros)
    };
  }

  public render(): JSX.Element {
    const {min, max, leadingZeros, value, onChange, ...rest} = this.props;
    return (
      <Input
        {...rest}
        ref={this._inputRef}
        onBlur={this.onBlur}
        value={this.state.text}
        onChange={this.onChange}
        onKeyDown={this.onKeyDown} onWheel={this.onWheel}/>);
  }

  public componentDidUpdate(prevProps: Properties) {
    if(this.props.value !== prevProps.value) {
      this.setState({
        text: padValue(this.props.value, this.props.leadingZeros)
      });
      return;
    }
    if(this._start != null) {
      this._inputRef.current?.setSelectionRange(this._start, this._end);
      this._start = null;
      this._end = null;
    }
  }

  private onKeyDown = (event: React.KeyboardEvent<HTMLInputElement>) => {
    if(this.props.readOnly || this.props.disabled) {
      return;
    }
    if(event.key === 'ArrowUp') {
      event.preventDefault();
      this.increment();
    } else if(event.key === 'ArrowDown') {
      event.preventDefault();
      this.decrement();
    }
  }

  private onWheel = (event: React.WheelEvent<HTMLInputElement>) => {
    if(this.props.readOnly || this.props.disabled ||
        document.activeElement !== event.target) {
      return;
    }
    if(event.deltaY > 0) {
      this.decrement();
    } else if(event.deltaY < 0) {
      this.increment();
    }
  }

  private onChange = (event: React.ChangeEvent<HTMLInputElement>) => {
    const text = event.target.value.replace(/[^0-9]/g, '');
    this.setState({text});
  }

  private onBlur = () => {
    if(this.state.text === '') {
      if(this.props.value != null) {
        this.props.onChange?.(undefined);
      }
      return;
    }
    const parsed = parseInt(this.state.text, 10);
    const value = (() => {
      if(isNaN(parsed)) {
        return this.props.min ?? 0;
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
    const parsed = parseInt(this.state.text, 10) || 0;
    const increment = parsed + 1;
    if(this.props.max != null && increment > this.props.max) {
      return;
    }
    this._start = this._inputRef.current?.selectionStart;
    this._end = this._inputRef.current?.selectionEnd;
    this.props.onChange?.(increment);
  }

  private decrement = () => {
    const parsed = parseInt(this.state.text, 10) || 0;
    const decrement = parsed - 1;
    if(this.props.min != null && decrement < this.props.min) {
      return;
    }
    this._start = this._inputRef.current?.selectionStart;
    this._end = this._inputRef.current?.selectionEnd;
    this.props.onChange?.(decrement);
  }

  private _inputRef = React.createRef<HTMLInputElement>();
  private _start: number;
  private _end: number;
}

function padValue(value: number | undefined, leadingZeros: number): string {
  if(value == null) {
    return '';
  }
  return ('0'.repeat(leadingZeros) + value).slice(-1 * leadingZeros);
}
