import { css, StyleSheet } from 'aphrodite/no-important';
import * as Beam from 'beam';
import * as React from 'react';
import { InputGroup, Select } from '../../components';
import { DateRule, DateRuleType, dateRuleToJson, Interval,
  MonthBoundaryDateRule, ValidationError } from '../../models';
import { Recurrence } from './recurrence';
import { RecurrenceInput } from './recurrence_input';
import { fromReportRecurrence, toReportRecurrence, upcomingReportRuns } from
  './report_recurrence';
import { RunsTable } from './runs_table';

interface Properties {

  /** The repeat interval, or null for a one-time report. */
  interval: Interval;

  /** The date rule applied within each interval. */
  rule?: DateRule;

  /** The selected month for yearly recurrences. */
  month?: number;

  /** The first allowed schedule timestamp. */
  start: Beam.DateTime;

  /** The current time used by the upcoming-runs preview. */
  now: Date;

  /** Reports a committed recurrence. */
  onChange: (interval: Interval, rule: DateRule, month: number) => void;

  /** Reports the validity of the current custom draft. */
  onValidationChange: (validation: InputGroup.ValidationState) => void;
}

interface State {
  selection: string;
  valid: boolean;
  recurrence: Recurrence;
}

/** Edits a report recurrence and previews its next three runs. */
export class ReportRepeatInput extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {selection: findPreset(props.interval, props.rule),
      valid: true,
      recurrence: this.makeRecurrence(props)};
  }

  public render(): JSX.Element {
    const props = this.props;
    const recurrence = {interval: props.interval, rule: props.rule,
      month: props.month};
    const runs = (() => {
      if(!this.state.valid || !isValidReportRepeat(props.interval, props.rule,
          props.month)) {
        return [];
      }
      return upcomingReportRuns(props.start, recurrence, props.now, 3);
    })();
    return <div>
      <InputGroup label='Repeat'>
        <Select value={this.state.selection} onChange={this.onSelectionChange}>
          {PRESETS.map(([value, label]) =>
            <option key={value} value={value}>{label}</option>)}
        </Select>
      </InputGroup>
      {this.state.selection === 'custom' && props.interval &&
        <div className={css(STYLES.custom)}>
          <RecurrenceInput value={this.state.recurrence}
            onChange={this.onRecurrenceChange}
            onValidationChange={this.onValidationChange}/>
        </div>}
      {!!props.interval && runs.length > 0 &&
        <div className={css(STYLES.runs)}><RunsTable runs={runs}/></div>}
    </div>;
  }

  public componentDidUpdate(previous: Properties): void {
    if(previous.interval !== this.props.interval ||
        previous.rule !== this.props.rule ||
        previous.month !== this.props.month) {
      this.setState({recurrence: this.makeRecurrence(this.props)});
      if(!this.props.interval || this.state.selection !== 'custom') {
        const selection = findPreset(this.props.interval, this.props.rule);
        if(selection !== this.state.selection) {
          this.setState({selection});
        }
      }
    }
  }

  private makeRecurrence(props: Properties): Recurrence {
    return fromReportRecurrence({interval: props.interval ??
      new Interval(1, Interval.Unit.DAY), rule: props.rule, month: props.month},
      props.start?.date ?? Beam.Date.today());
  }

  private onSelectionChange = (selection: string) => {
    this.setState({selection, valid: true});
    this.props.onValidationChange({valid: true, error: ValidationError.NONE});
    if(selection === 'never') {
      this.props.onChange(null, null, null);
    } else if(selection === 'daily') {
      this.props.onChange(new Interval(1, Interval.Unit.DAY), null, null);
    } else if(selection === 'weekly' || selection === 'fortnight') {
      let count = 7;
      if(selection === 'fortnight') {
        count = 14;
      }
      this.props.onChange(new Interval(count, Interval.Unit.DAY), null, null);
    } else if(selection === 'month_start' || selection === 'month_end') {
      let boundary = MonthBoundaryDateRule.Boundary.FIRST;
      if(selection === 'month_end') {
        boundary = MonthBoundaryDateRule.Boundary.LAST;
      }
      this.props.onChange(new Interval(1, Interval.Unit.MONTH),
        new MonthBoundaryDateRule(0, boundary, 0), null);
    } else {
      const recurrence = fromReportRecurrence({
        interval: this.props.interval ?? new Interval(1, Interval.Unit.DAY),
        rule: this.props.rule, month: this.props.month},
        this.props.start?.date ?? Beam.Date.today());
      this.onRecurrenceChange(recurrence);
    }
  };

  private onRecurrenceChange = (value: Recurrence) => {
    const {interval, rule, month} = toReportRecurrence(value);
    this.props.onChange(interval, rule, month);
  };

  private onValidationChange = (validation: InputGroup.ValidationState) => {
    if(validation.valid !== this.state.valid) {
      this.setState({valid: validation.valid});
    }
    this.props.onValidationChange(validation);
  };
}

/** Whether the recurrence contains a complete, supported interval and rule. */
export function isValidReportRepeat(interval: Interval, rule: DateRule,
    month?: number):
    boolean {
  if(!interval || !Number.isInteger(interval.count) || interval.count < 1 ||
      interval.count > 0xFFFFFFFF || !Number.isInteger(interval.unit) ||
      interval.unit < Interval.Unit.DAY || interval.unit > Interval.Unit.YEAR) {
    return false;
  }
  if(month != null && (interval.unit !== Interval.Unit.YEAR ||
      !Number.isInteger(month) || month < 1 || month > 12)) {
    return false;
  }
  if(isDefaultRule(rule)) {
    return true;
  }
  if(!ruleTypes(interval.unit).includes(rule.type) ||
      rule.type === DateRuleType.SPECIFIC_DATE || rule.offset !== 0) {
    return false;
  }
  try {
    dateRuleToJson(rule);
    return true;
  } catch {
    return false;
  }
}

function ruleTypes(unit: Interval.Unit): readonly DateRuleType[] {
  if(unit === Interval.Unit.WEEK) {
    return [DateRuleType.WEEKDAY];
  } else if(unit === Interval.Unit.MONTH || unit === Interval.Unit.YEAR) {
    return [DateRuleType.DAY_OF_MONTH, DateRuleType.MONTH_BOUNDARY,
      DateRuleType.ORDINAL_DAY_OF_MONTH];
  }
  return [];
}

function isDefaultRule(rule: DateRule): boolean {
  return !rule || rule.type === DateRuleType.DAY_OFFSET && rule.offset === 0;
}

function findPreset(interval: Interval, rule: DateRule): string {
  if(!interval) {
    return 'never';
  }
  if(isDefaultRule(rule)) {
    if(interval.count === 1 && interval.unit === Interval.Unit.DAY) {
      return 'daily';
    } else if(interval.count === 7 && interval.unit === Interval.Unit.DAY) {
      return 'weekly';
    } else if(interval.count === 14 && interval.unit === Interval.Unit.DAY) {
      return 'fortnight';
    }
  } else if(interval.count === 1 && interval.unit === Interval.Unit.MONTH &&
      rule.type === DateRuleType.MONTH_BOUNDARY && rule.offset === 0 &&
      rule.dayOffset === 0) {
    if(rule.boundary === MonthBoundaryDateRule.Boundary.FIRST) {
      return 'month_start';
    }
    return 'month_end';
  }
  return 'custom';
}

const PRESETS = [
  ['never', 'Never'],
  ['daily', 'Every day'],
  ['weekly', 'Every week'],
  ['fortnight', 'Every 2 weeks'],
  ['month_start', 'Beginning of every month'],
  ['month_end', 'End of every month'],
  ['custom', 'Custom']
];

const STYLES = StyleSheet.create({
  custom: {paddingTop: '20px',
    '@container (min-width: 384px)': {paddingTop: '10px'}},
  runs: {boxSizing: 'border-box', paddingTop: '30px',
    '@container (min-width: 384px)': {width: '384px', paddingLeft: '138px'}}
});
