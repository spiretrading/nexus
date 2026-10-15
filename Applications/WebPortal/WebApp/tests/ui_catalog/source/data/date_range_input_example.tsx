import * as Beam from 'beam';
import * as React from 'react';
import { DateRange, DateRangeInput, DateRangeOption, DateRangeValidation } from
  'web_portal';

interface Properties {

  /** The committed range. */
  value: DateRange;

  /** The label above or beside the selector. */
  label: string;

  /** The arrangement of the controls. */
  orientation: DateRangeInput.Orientation;

  /** The vertical layout's label placement. */
  labelPosition: DateRangeInput.LabelPosition;

  /** Whether both date bounds are required. */
  boundsRequired: boolean;

  /** Whether editing is disabled. */
  disabled: boolean;

  /** Whether values can only be viewed. */
  readOnly: boolean;

  /** Reports a committed range. */
  onChange?: (value: DateRange) => void;

  /** Reports draft validation. */
  onValidationChange?: (validation: DateRangeValidation) => void;
}

/** Demonstrates caller-supplied presets and custom date validation. */
export class DateRangeInputExample extends React.Component<Properties> {
  constructor(props: Properties) {
    super(props);
    const today = Beam.Date.today();
    this.options = [
      new DateRangeOption('month-to-date', 'Month to Date',
        new Beam.Date(today.year, today.month, 1), today),
      new DateRangeOption('previous-month', 'Previous Month',
        Beam.Date.fromDate(new Date(today.year, today.month - 2, 1)),
        Beam.Date.fromDate(new Date(today.year, today.month - 1, 0)))];
  }

  public render(): JSX.Element {
    return <DateRangeInput {...this.props} options={this.options}/>;
  }

  private options: DateRangeOption[];
}
