import * as Beam from 'beam';
import * as React from 'react';
import { DateRange, DateRangeInput, DateRangeRules, DateRangeValidation,
  makeParametersDateRangeOptions } from 'web_portal';

interface Properties {

  /** The committed range. */
  value: DateRange;

  /** The reference date used by the presets and custom rules. */
  referenceDate: Beam.Date;

  /** Reports the custom rule configuration. */
  onRulesChange?: (rules: DateRangeRules) => void;

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
  public render(): JSX.Element {
    const today = this.props.referenceDate ?? Beam.Date.today();
    const options = makeParametersDateRangeOptions(today);
    return <DateRangeInput {...this.props} options={options}/>;
  }
}
