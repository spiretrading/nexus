import * as React from 'react';
import { Interval } from 'web_portal';

interface Properties {

  /** The interval shown in the property editor. */
  value: Interval;

  /** Updates the catalog's interval property.
   * @param value - The updated interval.
   */
  update?: (value: Interval) => void;
}

/** Edits the count and unit of an interval property. */
export class IntervalValueInput extends React.Component<Properties> {
  public render(): JSX.Element {
    return <div>
      <input type='number' min={1} step={1} aria-label='Count property'
        value={this.props.value.count ?? ''} onChange={this.onCountChange}/>
      <select aria-label='Unit property' value={this.props.value.unit}
          onChange={this.onUnitChange}>
        {['Day', 'Week', 'Month', 'Year'].map((label, unit) =>
          <option key={unit} value={unit}>{label}</option>)}
      </select>
    </div>;
  }

  private onCountChange = (event: React.ChangeEvent<HTMLInputElement>) => {
    let count = event.target.valueAsNumber;
    if(Number.isNaN(count)) {
      count = null;
    }
    this.props.update?.(new Interval(count, this.props.value.unit));
  };

  private onUnitChange = (event: React.ChangeEvent<HTMLSelectElement>) => {
    this.props.update?.(new Interval(this.props.value.count,
      Number(event.target.value)));
  };
}
