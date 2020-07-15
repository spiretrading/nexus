import { css, StyleSheet } from 'aphrodite';
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
  style?: any;

  /** The class name of the money input box. */
  className?: string;
}

interface State {
  value: Nexus.Money;
}

/** A editable money field. */
export class MoneyField extends React.Component<Properties, State> {
  public static readonly defaultProps = {
    onChange: () => {}
  };

  constructor(props: Properties) {
    super(props);
    this.state = {
      value: props.value || Nexus.Money.ZERO
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
          value={this.state.value.toString()}
          onKeyDown={this.onKeyDown} onWheel={this.onWheel}
          onChange={this.onChange}/>);
  }

  public componentDidUpdate() {
    if(this._start != null) {
      this._input.setSelectionRange(this._start, this._end);
      this._start = null;
      this._end = null;
    }
  }

  private onKeyDown = (event: React.KeyboardEvent<HTMLInputElement>) => {
    if(event.keyCode === 38) {
      this.increment();
    } else if(event.keyCode === 40) {
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
    const pointIndex = event.target.value.search('\\.');
    const sanitizedValue = (() => {
      if(pointIndex === -1) {
        const leftPortion = event.target.value.substr(0,
          this._input.selectionStart - 1);
        const insertPortion = event.target.value.substr(
          this._input.selectionEnd - 1, 1);
        let rightPortion = event.target.value.substr(
          this._input.selectionEnd);
        rightPortion = rightPortion.padEnd(2, '0');
        return leftPortion + insertPortion + '.' + rightPortion;
      }
      const secondPointIndex = event.target.value.substr(pointIndex + 1).search(
        '\\.');
      if(secondPointIndex === -1) {
        return event.target.value;
      } else if(this._input.selectionStart > pointIndex + 1) {
        return '';
      } else if(secondPointIndex === 0) {
        return event.target.value.slice(0, pointIndex + 1) +
          event.target.value.slice(pointIndex + 2);
      } else {
        return event.target.value.slice(0, pointIndex + 1) + '00';
      }
    })();
    let value = Nexus.Money.parse(sanitizedValue);
    if(value === null) {
      this._start = this._input.selectionStart - 1;
      this._end = this._input.selectionEnd - 1;
      this.forceUpdate();
      return;
    }
    if(this.props.min && value.compare(this.props.min) < 0 ||
        this.props.max && value.compare(this.props.max) > 0) {
      return;
    }
    this.update(value);
  }

  private increment = () => {
    const increment = this.state.value.add(Nexus.Money.ONE);
    if(this.props.max && increment.compare(this.props.max) > 0) {
      return;
    }
    this.update(increment);
  }

  private decrement = () => {
    const decrement = this.state.value.subtract(Nexus.Money.ONE);
    if(this.props.min && decrement.compare(this.props.min) < 0) {
      return;
    }
    this.update(decrement);
  }

  private update = (value: Nexus.Money) => {
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
      height: '34px',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      flexWrap: 'nowrap' as 'nowrap',
      alignItems: 'center' as 'center',
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
    }
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
