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
export class PaddedIntegerField extends React.Component<Properties, State> {
  public static readonly defaultProps = {
    value: 0,
    onChange: () => {}
  };

  constructor(props: Properties) {
    super(props);
    this.state = {
      value: props.value || 0,
    }
    this.onKeyDown = this.onKeyDown.bind(this);
    this.onWheel = this.onWheel.bind(this);
    this.onChange = this.onChange.bind(this);
    this.onBlur = this.onBlur.bind(this);
  }

  public render(): JSX.Element {
    const shownValue = (() => {
      return ('0'.repeat(this.props.padding) + this.state.value).slice(-1 * (this.props.padding));
    })();
    return (
        <input
          onBlur={this.onBlur}
          style={{...PaddedIntegerField.STYLE.editBox, ...this.props.style}} 
          value={shownValue}
          min={0}
          onChange={this.onChange}
          className={css(PaddedIntegerField.EXTRA_STYLE.customHighlighting)}
          type={'number'}/>);
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
    console.log('BOOP BOPP');
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
    console.log(value);
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
      width: '18px',
      height: '17px',
      padding: 0,
      margin: 0,
      font: '400 14px Roboto',
      border: '0px solid #ffffff',
      color: '#000000',
      textAlign: 'right'as 'right',
      WebkitAppearance: 'none' as 'none',
      MozAppearance: 'textfield' as 'textfield'
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
