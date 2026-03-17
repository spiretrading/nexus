import * as React from 'react';

interface Properties {

  /** The value of the field. */
  value?: number;

  /** The minimum value. */
  min?: number;

  /** The maximum value. */
  max?: number;

  /** The callback to update the value. */
  update?: (newValue: any) => void;
}

/** A number input with a synchronized slider. */
export class NumberSliderInput extends React.Component<Properties> {
  public render(): JSX.Element {
    return (
      <div style={NumberSliderInput.STYLE.container}>
        <input type='number' style={NumberSliderInput.STYLE.numberInput}
          value={this.props.value}
          min={this.props.min} max={this.props.max}
          onChange={this.onChange} onWheel={this.onWheel}/>
        <input type='range' style={NumberSliderInput.STYLE.slider}
          value={this.props.value}
          min={this.props.min} max={this.props.max}
          onChange={this.onChange}/>
      </div>);
  }

  private onChange = (event: React.ChangeEvent<HTMLInputElement>) => {
    this.props.update(event.target.valueAsNumber);
  }

  private onWheel = (event: React.WheelEvent<HTMLInputElement>) => {
    if(document.activeElement !== event.currentTarget) {
      return;
    }
    event.preventDefault();
    const current = this.props.value ?? 0;
    const delta = event.deltaY < 0 ? 1 : -1;
    let next = current + delta;
    if(this.props.min !== undefined) {
      next = Math.max(next, this.props.min);
    }
    if(this.props.max !== undefined) {
      next = Math.min(next, this.props.max);
    }
    this.props.update(next);
  }

  private static readonly STYLE = {
    container: {
      display: 'flex',
      alignItems: 'center',
      gap: '8px'
    } as React.CSSProperties,
    numberInput: {
      width: '60px'
    } as React.CSSProperties,
    slider: {
      flex: '1 1 auto'
    } as React.CSSProperties
  };
}
