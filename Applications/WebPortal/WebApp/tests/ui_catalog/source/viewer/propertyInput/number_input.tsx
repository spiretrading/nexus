import * as React from 'react';

interface Properties {

  /** The value of the field. */
  value?: number;

  /** The callback to update the value. */
  update?: (newValue: number) => void;
}

/** A number input field for positive numbers. */
export class NumberInput extends React.Component<Properties> {
  public render(): JSX.Element {
    return <input type="number" value={this.props.value ?? ''}
      onChange={this.onChange} onWheel={this.onWheel}/>;
  }

  private onChange = (event: React.ChangeEvent<HTMLInputElement>) => {
    const value = event.target.valueAsNumber;
    if(Number.isFinite(value)) {
      this.props.update?.(value);
    } else {
      this.props.update?.(null);
    }
  }

  private onWheel = (event: React.WheelEvent<HTMLInputElement>) => {
    if(document.activeElement !== event.currentTarget) {
      return;
    }
    event.preventDefault();
    const delta = event.deltaY < 0 ? 1 : -1;
    this.props.update(Number(this.props.value) + delta);
  }
}
