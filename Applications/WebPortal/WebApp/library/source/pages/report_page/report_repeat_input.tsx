import * as Beam from 'beam';
import * as React from 'react';
import { DateRuleInput, IntervalInput, Select } from '../../components';
import { DateRule, DateRuleType, dateRuleToJson, Interval,
  MonthBoundaryDateRule, Weekday, WeekdayDateRule } from '../../models';

interface Properties extends
    Omit<React.HTMLAttributes<HTMLDivElement>, 'onChange'> {

  /** The identifier for the repeat selector. */
  id?: string;

  /** The repeat interval, or null for a one-time report. */
  interval: Interval;

  /** The date rule applied within each interval, or null for its anchor. */
  rule?: DateRule;

  /** The first allowed schedule date. */
  reference: Beam.Date;

  /** Reports changes to the interval and rule together. */
  onChange: (interval: Interval, rule: DateRule) => void;
}

interface State {
  selection: string;
}

/** Edits a report's recurrence using presets and shared date-rule controls. */
export class ReportRepeatInput extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {selection: findPreset(props.interval, props.rule)};
  }

  public render(): JSX.Element {
    const {id, interval, rule, reference, style, onChange, ...attributes} =
      this.props;
    return <div {...attributes} role='group'
        style={{display: 'grid', gap: '10px', width: '100%', ...style}}>
      <Select id={this.props.id} aria-label='Repeat'
          value={this.state.selection} style={{width: '100%'}}
          onChange={this.onSelectionChange}>
        {PRESETS.map(([value, label]) =>
          <option key={value} value={value}>{label}</option>)}
      </Select>
      {this.state.selection === 'custom' && interval && <>
        <div style={{display: 'flex', alignItems: 'center', gap: '8px'}}>
          Every
          <IntervalInput value={interval} required
            style={{flex: 1, minWidth: 0}} onChange={this.onIntervalChange}/>
        </div>
        {interval.unit !== Interval.Unit.DAY &&
          <Select aria-label='Repeat date' value={(() => {
              if(!isDefaultRule(rule)) {
                return 'rule';
              }
              return 'start';
            })()} style={{width: '100%'}} onChange={this.onDateChange}>
            <option value='start'>Same day as start</option>
            <option value='rule'>Date rule</option>
          </Select>}
        {!isDefaultRule(rule) &&
          <DateRuleInput id={`${this.props.id}-rule`} label='Repeat'
            value={rule} reference={reference} types={ruleTypes(interval.unit)}
            showOffset={false} onChange={this.onRuleChange}/>}
      </>}
    </div>;
  }

  public componentDidUpdate(previous: Properties): void {
    if(previous.interval !== this.props.interval ||
        previous.rule !== this.props.rule) {
      if(!this.props.interval || this.state.selection !== 'custom') {
        const selection = findPreset(this.props.interval, this.props.rule);
        if(selection !== this.state.selection) {
          this.setState({selection});
        }
      }
    }
  }

  private onSelectionChange = (selection: string) => {
    this.setState({selection});
    if(selection === 'never') {
      this.props.onChange(null, null);
    } else if(selection === 'daily') {
      this.props.onChange(new Interval(1, Interval.Unit.DAY), null);
    } else if(selection === 'weekly' || selection === 'fortnight') {
      let count = 1;
      if(selection === 'fortnight') {
        count = 2;
      }
      this.props.onChange(new Interval(count, Interval.Unit.WEEK), null);
    } else if(selection === 'month_start' || selection === 'month_end') {
      let boundary = MonthBoundaryDateRule.Boundary.FIRST;
      if(selection === 'month_end') {
        boundary = MonthBoundaryDateRule.Boundary.LAST;
      }
      this.props.onChange(new Interval(1, Interval.Unit.MONTH),
        new MonthBoundaryDateRule(0, boundary, 0));
    } else {
      this.props.onChange(this.props.interval ??
        new Interval(1, Interval.Unit.DAY), this.props.rule ?? null);
    }
  };

  private onIntervalChange = (interval: Interval) => {
    let rule = this.props.rule;
    if(rule && !ruleTypes(interval.unit).includes(rule.type)) {
      rule = null;
    }
    this.props.onChange(interval, rule);
  };

  private onDateChange = (selection: string) => {
    let rule: DateRule = null;
    if(selection === 'rule') {
      if(this.props.interval.unit === Interval.Unit.WEEK) {
        rule = new WeekdayDateRule(0, Weekday.MONDAY);
      } else {
        rule = new MonthBoundaryDateRule(0,
          MonthBoundaryDateRule.Boundary.LAST, 0);
      }
    }
    this.props.onChange(this.props.interval, rule);
  };

  private onRuleChange = (rule: DateRule) => {
    this.props.onChange(this.props.interval, rule);
  };
}

/** Whether the recurrence contains a complete, supported interval and rule. */
export function isValidReportRepeat(interval: Interval, rule: DateRule):
    boolean {
  if(!interval || !Number.isInteger(interval.count) || interval.count < 1 ||
      interval.count > 0xFFFFFFFF || !Number.isInteger(interval.unit) ||
      interval.unit < Interval.Unit.DAY || interval.unit > Interval.Unit.YEAR) {
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
    return [DateRuleType.DAY_OF_MONTH, DateRuleType.MONTH_BOUNDARY];
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
    } else if(interval.count === 1 && interval.unit === Interval.Unit.WEEK) {
      return 'weekly';
    } else if(interval.count === 2 && interval.unit === Interval.Unit.WEEK) {
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
  ['fortnight', 'Every two weeks'],
  ['month_start', 'Beginning of every month'],
  ['month_end', 'End of every month'],
  ['custom', 'Custom']
];
