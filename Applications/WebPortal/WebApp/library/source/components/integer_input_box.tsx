import * as React from 'react';

interface Properties {

  /** The minimum allowed value. */
  min?: number;

  /** The maximum allowed value. */
  max?: number;

  /** The amount of padding to display (0s are used for padding). */
  padding?: number;

  /** The value to display. */
  value?: number;

  /** Additional CSS styles. */
  style?: any;

  /** The class name of the input box. */
  className?: string;

  /** The event handler for when a change is made. */
  onChange?: (value?: number) => (boolean | void);
}

interface State {
  value: number;
}

/** Displays an input box for modifying integer values. */
export class IntegerInputBox extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {
      value: props.value || 0
    };
    this.onKeyDown = this.onKeyDown.bind(this);
    this.onWheel = this.onWheel.bind(this);
    this.onChange = this.onChange.bind(this);
  }

  public render(): JSX.Element {
    const value = this.state.value.toString().padStart(
      this.props.padding || 0, '0');
    return (
      <div>
        <input style={{...IntegerInputBox.STYLE.input, ...this.props.style}} 
          type='text'
          className={this.props.className}
          ref={(input) => { this._input = input; }} value={value}
          onKeyDown={this.onKeyDown} onWheel={this.onWheel}
          onChange={this.onChange}/>
      </div>);
  }

  public componentDidUpdate() {
    if(this._start != null) {
      this._input.setSelectionRange(this._start, this._end);
      this._start = null;
      this._end = null;
    }
    if(this.state.value < 10) {
      console.log('shove selected!', this.state.value );
      this._input.setSelectionRange(2, 2);
    }
  }

  private onKeyDown(event: React.KeyboardEvent<HTMLInputElement>) {
    if(event.keyCode === 38) {
      this.increment();
    } else if(event.keyCode === 40) {
      this.decrement();
    }
  }

  private onWheel(event: React.WheelEvent<HTMLInputElement>) {
    if(document.activeElement !== event.target) {
      return;
    }
    if(event.deltaY > 0) {
      this.decrement();
    } else if(event.deltaY < 0) {
      this.increment();
    }
  }

  private onChange(event: React.ChangeEvent<HTMLInputElement>) {
    console.log(event.target.value);
    let value = (() => {
      if(event.target.value.length === 0) {
        return 0;
      } else {
        return parseInt(event.target.value, 10);
      }
    })();
    if(isNaN(value)) {
      this._start = this._input.selectionStart - 1;
      this._end = this._input.selectionEnd - 1;
      this.forceUpdate();
      return;
    }
    if(this.props.max != null && value > this.props.max) {
      value = Math.trunc(value / 10);
    }
    if(this.props.min != null && value < this.props.min ||
        this.props.max != null && value > this.props.max) {
      return;
    }
    this.update(value);
  }

  private increment() {
    const increment = this.state.value + 1;
    if(this.props.max != null && increment > this.props.max) {
      return;
    }
    this.update(increment);
  }

  private decrement() {
    const decrement = this.state.value - 1;
    if(this.props.min != null && decrement < this.props.min) {
      return;
    }
    this.update(decrement);
  }

  private update(value: number) {
    if(this.props.onChange) {
      const commit = this.props.onChange(value);
      if(commit != null && commit === false) {
        return;
      }
    }
    this._start = this._input.selectionStart;
    this._end = this._input.selectionEnd;
    this.setState({
      value: value
    });
  }

  private static readonly STYLE = {
    input: {
      boxSizing: 'border-box' as 'border-box',
      font: '16px Roboto',
      width: '66px',
      height: '34px',
      border: '1px solid #C8C8C8',
      textAlign: 'center' as 'center'
    }
  };
  private _input: HTMLInputElement;
  private _start: number;
  private _end: number;
}
