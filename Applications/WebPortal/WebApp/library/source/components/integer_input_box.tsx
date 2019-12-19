import { css, StyleSheet } from 'aphrodite';
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
  public static readonly defaultProps = {
    value: 0,
    onChange: () => {}
  };

  constructor(props: Properties) {
    super(props);
    this.state = {
      value: props.value || this.props.min
    }
    this.onKeyDown = this.onKeyDown.bind(this);
    this.onWheel = this.onWheel.bind(this);
    this.onChange = this.onChange.bind(this);
    this.onBlur = this.onBlur.bind(this);
  }

  public render(): JSX.Element {
    const shownValue = (() => {
      return ('0'.repeat(this.props.padding) + 
        this.state.value).slice(-1 * (this.props.padding));
    })();
    return (
      <input
        onBlur={this.onBlur}
        style={{...IntegerInputBox.STYLE.editBox, ...this.props.style}} 
        value={shownValue}
        onChange={this.onChange}
        onKeyDown={this.onKeyDown} onWheel={this.onWheel}
        className={css(IntegerInputBox.EXTRA_STYLE.customHighlighting)}
        type={'text'}/>);
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
    let value = (() => {
      if(event.target.value.length === 0) {
        return 0;
      } else {
        return parseInt(event.target.value.slice(-1 * this.props.padding), 10);
      }
    })();
    if(isNaN(value)) {
      this.forceUpdate();
      return;
    }
    this.setState({value: value});
  }

  private onBlur() {
    let value = (() => {
    if(this.state.value < this.props.min) {
      return this.props.min;
    } else if(this.state.value > this.props.max) {
      return this.props.max;
    } else {
      return this.state.value;
    }
    })();
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
    this.setState({
      value: value
    });
  }

  private static readonly STYLE = {
    editBox: {
      boxSizing: 'border-box' as 'border-box',
      font: '16px Roboto',
      width: '66px',
      height: '34px',
      border: '1px solid #C8C8C8',
      textAlign: 'center' as 'center'
    }
  };
  private static EXTRA_STYLE = StyleSheet.create({
    customHighlighting: {
      '-moz-appearance': 'textfield',
      ':focus': {
        ouline: 0,
        borderColor: '#684BC7',
        boxShadow: 'none',
        webkitBoxShadow: 'none',
        outlineColor: 'transparent',
        outlineStyle: 'none'
      },
      ':active' : {
        borderColor: '#684BC7'
      },
      '::moz-focus-inner': {
        border: 0
      },
      '::placeholder': {
        color: '#8C8C8C'
      },
      '::-webkit-inner-spin-button': {
        '-webkit-appearance': 'none',
        'appearance': 'none',
        margin: 0
      },
      '::-webkit-outer-spin-button': { 
        '-webkit-appearance': 'none',
        'appearance': 'none',
        margin: 0
      }
    }
  });
}
