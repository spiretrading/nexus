import * as React from 'react';
import { Weekday } from '../models/date_rule';
import { Select } from './select';

interface Properties extends Omit<React.ComponentProps<typeof Select>,
    'children' | 'defaultValue' | 'multiple' | 'onChange' | 'value'> {

  /** The selected day of the week. */
  value: Weekday;

  /** Called when the selected day changes. */
  onChange?: (value: Weekday) => void;
}

/** Selects a day of the week, listed from Monday through Sunday. */
export class DayOfWeekSelect extends React.Component<Properties> {
  public render(): JSX.Element {
    const {onChange, ...properties} = this.props;
    return <Select {...properties} onChange={this.onChange}>
      {Object.values(Weekday).map(day =>
        <option key={day} value={day}>
          {day[0] + day.slice(1).toLowerCase()}
        </option>)}
    </Select>;
  }

  private onChange = (value: string) => {
    this.props.onChange?.(value as Weekday);
  };
}
