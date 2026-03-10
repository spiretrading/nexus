import { css, StyleSheet } from 'aphrodite/no-important';
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
  style?: React.CSSProperties;

  /** The class name of the input box. */
  className?: string;

  /** Determines if the component is readonly. */
  readonly?: boolean;

  /** The event handler for when a change is made. */
  onChange?: (value?: number) => (boolean | void);
}

interface State {
  text: string;
}

/** An editable integer field. */
export class IntegerField extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {
      text: padValue(props.value ?? props.min ?? 0, props.padding)
    };
  }

  public render(): JSX.Element {
    return (
      <input
        onBlur={this.onBlur}
        style={{...IntegerField.STYLE.editBox, ...this.props.style}}
        value={this.state.text}
        onChange={this.onChange}
        readOnly={this.props.readonly}
        disabled={this.props.readonly}
        onKeyDown={this.onKeyDown} onWheel={this.onWheel}
        className={css(IntegerField.EXTRA_STYLE.effects)}
        type={'text'}/>);
  }

  public componentDidUpdate(prevProps: Properties) {
    if(this.props.value !== prevProps.value) {
      this.setState({
        text: padValue(this.props.value, this.props.padding)
      });
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
    const text = event.target.value.replace(/[^0-9]/g, '');
    this.setState({text});
  }

  private onBlur = () => {
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
    this.update(value);
  }

  private increment = () => {
    const parsed = parseInt(this.state.text, 10) || 0;
    const increment = parsed + 1;
    if(this.props.max != null && increment > this.props.max) {
      return;
    }
    this.update(increment);
  }

  private decrement = () => {
    const parsed = parseInt(this.state.text, 10) || 0;
    const decrement = parsed - 1;
    if(this.props.min != null && decrement < this.props.min) {
      return;
    }
    this.update(decrement);
  }

  private update = (value: number) => {
    if(this.props.onChange) {
      const commit = this.props.onChange(value);
      if(commit != null && commit === false) {
        return;
      }
    }
    this.setState({
      text: padValue(value, this.props.padding)
    });
  }

  private static readonly STYLE = {
    editBox: {
      boxSizing: 'border-box',
      font: '16px Roboto',
      width: '66px',
      height: '34px',
      border: '1px solid #C8C8C8',
      color: '#333333',
      textAlign: 'center',
      backgroundColor: '#FFFFFF'
    } as React.CSSProperties
  };
  private static readonly EXTRA_STYLE = StyleSheet.create({
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

function padValue(value: number, padding: number): string {
  return ('0'.repeat(padding) + value).slice(-1 * padding);
}
