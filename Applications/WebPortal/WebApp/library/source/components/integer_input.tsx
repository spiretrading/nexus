import { css, StyleSheet } from 'aphrodite/no-important';
import * as React from 'react';

interface Properties extends
    Omit<React.InputHTMLAttributes<HTMLInputElement>,
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
    const {min, max, leadingZeros, value, readOnly, onChange,
      style, className, ...rest} = this.props;
    return (
      <input
        {...rest}
        onBlur={this.onBlur}
        style={{...IntegerInput.STYLE.editBox, ...style}}
        value={this.state.text}
        onChange={this.onChange}
        readOnly={readOnly}
        disabled={readOnly}
        onKeyDown={this.onKeyDown} onWheel={this.onWheel}
        className={[css(IntegerInput.EXTRA_STYLE.effects), className].join(' ')}
        type={'text'}/>);
  }

  public componentDidUpdate(prevProps: Properties) {
    if(this.props.value !== prevProps.value) {
      this.setState({
        text: padValue(this.props.value, this.props.leadingZeros)
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
    if(this.state.text === '') {
      this.props.onChange?.(undefined);
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
    this.props.onChange?.(value);
  }

  private increment = () => {
    const parsed = parseInt(this.state.text, 10) || 0;
    const increment = parsed + 1;
    if(this.props.max != null && increment > this.props.max) {
      return;
    }
    this.props.onChange?.(increment);
  }

  private decrement = () => {
    const parsed = parseInt(this.state.text, 10) || 0;
    const decrement = parsed - 1;
    if(this.props.min != null && decrement < this.props.min) {
      return;
    }
    this.props.onChange?.(decrement);
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

function padValue(value: number | undefined, leadingZeros: number): string {
  if(value == null) {
    return '';
  }
  return ('0'.repeat(leadingZeros) + value).slice(-1 * leadingZeros);
}
