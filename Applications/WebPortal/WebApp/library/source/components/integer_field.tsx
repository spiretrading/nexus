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

  /** Determines if the component is readonly. */
  readonly?: boolean;

  /** The event handler for when a change is made. */
  onChange?: (value?: number) => (boolean | void);
}

interface State {
  value: number;
}

/** A editable integer field. */
export class IntegerField extends React.Component<Properties, State> {
  public static readonly defaultProps = {
    min: 0,
    value: 0,
    onChange: () => {}
  };

  constructor(props: Properties) {
    super(props);
    this.state = {
      value: props.value || this.props.min
    };
    this.onKeyDown = this.onKeyDown.bind(this);
    this.onWheel = this.onWheel.bind(this);
    this.onChange = this.onChange.bind(this);
    this.onBlur = this.onBlur.bind(this);
  }

  public render(): JSX.Element {
    const shownValue =  ('0'.repeat(this.props.padding) +
      this.state.value).slice(-1 * (this.props.padding));
    return (
      <input
        onBlur={this.onBlur}
        style={{...IntegerField.STYLE.editBox, ...this.props.style}}
        value={shownValue}
        onChange={this.onChange}
        readOnly={this.props.readonly}
        disabled={this.props.readonly}
        onKeyDown={this.onKeyDown} onWheel={this.onWheel}
        className={css(IntegerField.EXTRA_STYLE.effects)}
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
    const value = (() => {
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
    const value = (() => {
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
      color: '#333333',
      textAlign: 'center' as 'center',
      backgroundColor: '#FFFFFF'
    }
  };
  private static EXTRA_STYLE = StyleSheet.create({
    effects: {
      '-moz-appearance': 'textfield',
      ':focus': {
        outline: 0,
        borderColor: '#684BC7',
        boxShadow: 'none',
        webkitBoxShadow: 'none',
        outlineColor: 'transparent',
        outlineStyle: 'none'
      },
      ':active': {
        borderColor: '#684BC7'
      },
      '::moz-focus-inner': {
        border: 0
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
