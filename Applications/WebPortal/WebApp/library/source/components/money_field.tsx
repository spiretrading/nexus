import { css, StyleSheet } from 'aphrodite/no-important';
import * as Nexus from 'nexus';
import * as React from 'react';

interface Properties {

  /** The minimum allowed value (inclusive). */
  min?: Nexus.Money;

  /** The maximum allowed value (inclusive). */
  max?: Nexus.Money;

  /** The initial value to display. */
  value?: Nexus.Money;

  /** Indicates if the component is readonly. */
  readonly?: boolean;

  /** The event handler for when a change is made. */
  onChange?: (value?: Nexus.Money) => (boolean | void);

  /** Additional CSS styles. */
  style?: React.CSSProperties;

  /** The class name of the money input box. */
  className?: string;
}

interface State {
  text: string;
  value: Nexus.Money;
}

/** An editable money field. */
export class MoneyField extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    const value = props.value ?? Nexus.Money.ZERO;
    this.state = {
      text: value.toString(),
      value
    };
  }

  public render(): JSX.Element {
    return (
        <input
          style={{...MoneyField.STYLE.input, ...this.props.style}}
          className={css(MoneyField.EXTRA_STYLE.effects) + ' ' +
            this.props.className}
          type='text'
          disabled={this.props.readonly}
          ref={(input) => {this._input = input;}}
          value={this.state.text}
          onKeyDown={this.onKeyDown} onWheel={this.onWheel}
          onChange={this.onChange} onBlur={this.onBlur}/>);
  }

  public componentDidUpdate() {
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
    const parsed = Nexus.Money.parse(this.state.text);
    const value = (() => {
      if(parsed === null) {
        return this.state.value;
      } else if(this.props.min && parsed.compare(this.props.min) < 0) {
        return this.props.min;
      } else if(this.props.max && parsed.compare(this.props.max) > 0) {
        return this.props.max;
      } else {
        return parsed;
      }
    })();
    this.commit(value);
  }

  private increment = () => {
    const increment = this.state.value.add(Nexus.Money.ONE);
    if(this.props.max && increment.compare(this.props.max) > 0) {
      return;
    }
    this.commit(increment);
  }

  private decrement = () => {
    const decrement = this.state.value.subtract(Nexus.Money.ONE);
    if(this.props.min && decrement.compare(this.props.min) < 0) {
      return;
    }
    this.commit(decrement);
  }

  private commit = (value: Nexus.Money) => {
    if(this.props.onChange) {
      const commit = this.props.onChange(value);
      if(commit != null && commit === false) {
        return;
      }
    }
    this._start = this._input.selectionStart;
    this._end = this._input.selectionEnd;
    this.setState({
      text: value.toString(),
      value
    });
  }

  private static readonly STYLE = {
    input: {
      boxSizing: 'border-box',
      height: '34px',
      display: 'flex',
      flexDirection: 'row',
      flexWrap: 'nowrap',
      alignItems: 'center',
      justifyContent: 'space-between',
      border: '1px solid #C8C8C8',
      borderRadius: '1px',
      font: '400 14px Roboto',
      backgroundColor: '#FFFFFF',
      color: '#333333',
      flexGrow: 1,
      maxWidth: '246px',
      width: '100%',
      paddingLeft: '10px'
    } as React.CSSProperties
  };
  private static readonly EXTRA_STYLE = StyleSheet.create({
    effects: {
      ':focus': {
        borderColor: '#684BC7',
        boxShadow: 'none',
        webkitBoxShadow: 'none',
        outlineColor: 'transparent',
        outlineStyle: 'none'
      },
      '::moz-focus-inner': {
        border: 0
      }
    }
  });

  private _input: HTMLInputElement;
  private _start: number;
  private _end: number;
}
