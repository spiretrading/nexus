import * as Beam from 'beam';
import * as React from 'react';
import { Select } from './select';

interface Properties extends Omit<React.ComponentProps<typeof Select>,
    'children' | 'defaultValue' | 'multiple' | 'onChange' | 'value'> {

  /** The selected month of the year. */
  value: Beam.Date.Month;

  /** Called when the selected month changes. */
  onChange?: (value: Beam.Date.Month) => void;
}

/** Selects a month of the year, listed from January through December. */
export class MonthSelect extends React.Component<Properties> {
  public render(): JSX.Element {
    const {onChange, ...properties} = this.props;
    return <Select {...properties} onChange={this.onChange}>
      {Object.values(Beam.Date.Month).
        filter(value => typeof value === 'number').map(month => {
          const name = Beam.Date.Month[month];
          return <option key={month} value={month}>
            {name[0] + name.slice(1).toLowerCase()}
          </option>;
        })}
    </Select>;
  }

  private onChange = (value: string) => {
    this.props.onChange?.(Number(value) as Beam.Date.Month);
  };
}
