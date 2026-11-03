import * as React from 'react';
import { DayOfMonth, DayOfMonthInput } from 'web_portal';

interface Properties {

  /** The day of the month shown in the property editor. */
  value: DayOfMonth;

  /** Updates the catalog's day of the month property. */
  update?: (value: DayOfMonth) => void;
}

/** Edits a day of the month property. */
export class DayOfMonthValueInput extends React.Component<Properties> {
  public render(): JSX.Element {
    return <DayOfMonthInput aria-label='Day of month property'
      value={this.props.value} onInput={this.props.update}
      onChange={this.props.update}/>;
  }
}
