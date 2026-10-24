import { StyleSheetTestUtils } from 'aphrodite/no-important';
import * as assert from 'node:assert/strict';
import { afterEach, beforeEach, describe, it } from 'node:test';
import * as Beam from 'beam';
import { DayOfMonth, Interval, ValidationError, Weekday } from
  '../../source/models';
import { DayOfYear, Recurrence } from
  '../../source/pages/report_page/recurrence';
import { RecurrenceInput } from
  '../../source/pages/report_page/recurrence_input';

function makeInput() {
  const commits: Recurrence[] = [];
  const errors: ValidationError[] = [];
  const input: any = new RecurrenceInput({value:
    new Recurrence(new Interval(1, Interval.Unit.DAY), null),
    onChange: value => commits.push(value),
    onValidationChange: validation => errors.push(validation.error)});
  input.setState = (update: any, callback?: () => void) => {
    input.state = {...input.state, ...update};
    callback?.();
  };
  return {input, commits, errors};
}

describe('RecurrenceInput', () => {
  beforeEach(() => StyleSheetTestUtils.suppressStyleInjection());
  afterEach(() => StyleSheetTestUtils.clearBufferAndResumeStyleInjection());

  it('retains_occurrences_when_switching_units', () => {
    const {input, commits} = makeInput();
    input.onIntervalChange(new Interval(2, Interval.Unit.WEEK));
    input.onWeekdayChange(Weekday.FRIDAY);
    input.onIntervalChange(new Interval(2, Interval.Unit.MONTH));
    const day = new DayOfMonth(DayOfMonth.Position.LAST,
      DayOfMonth.DayType.WEEKDAY);
    input.onDayChange(day);
    input.onIntervalChange(new Interval(2, Interval.Unit.YEAR));
    input.onMonthChange(Beam.Date.Month.MARCH);
    const other = new DayOfMonth(DayOfMonth.Position.DAY, 15);
    input.onDayChange(other);
    assert.deepEqual(commits.at(-1).occurrence,
      new DayOfYear(other, Beam.Date.Month.MARCH));
    input.onIntervalChange(new Interval(2, Interval.Unit.DAY));
    assert.equal(commits.at(-1).occurrence, null);
    input.onIntervalChange(new Interval(2, Interval.Unit.WEEK));
    assert.equal(commits.at(-1).occurrence, Weekday.FRIDAY);
    input.onIntervalChange(new Interval(2, Interval.Unit.YEAR));
    assert.deepEqual(commits.at(-1).occurrence,
      new DayOfYear(other, Beam.Date.Month.MARCH));
  });

  it('validates_drafts_and_commits_blurred_counts', () => {
    const {input, commits, errors} = makeInput();
    input.onIntervalInput({target: {tagName: 'INPUT', value: ''}});
    assert.equal(commits.length, 0);
    assert.equal(errors.at(-1), ValidationError.REQUIRED);
    input.onIntervalBlur({target: {tagName: 'INPUT'}});
    assert.equal(commits.at(-1).interval.count, null);
    input.onIntervalInput({target: {tagName: 'INPUT', value: '3'}});
    assert.equal(commits.length, 1);
    input.onIntervalBlur({target: {tagName: 'INPUT'}});
    assert.equal(commits.at(-1).interval.count, 3);
    input.onIntervalBlur({target: {tagName: 'INPUT'}});
    assert.equal(commits.length, 2);
    input.onIntervalChange(new Interval(3, Interval.Unit.YEAR));
    input.onDayInput(new DayOfMonth(DayOfMonth.Position.DAY, null));
    assert.equal(commits.length, 3);
    assert.equal(errors.at(-1), ValidationError.REQUIRED);
    input.onDayChange(new DayOfMonth(DayOfMonth.Position.DAY, 31));
    assert.equal(errors.at(-1), ValidationError.NONE);
    input.onDayInput(new DayOfMonth(DayOfMonth.Position.DAY, null));
    input.onIntervalChange(new Interval(3, Interval.Unit.DAY));
    assert.equal(errors.at(-1), ValidationError.NONE);
  });

  it('applies_external_values_and_blocks_disabled_edits', () => {
    const {input, commits} = makeInput();
    const previous = input.props;
    const value = new Recurrence(new Interval(4, Interval.Unit.YEAR),
      new DayOfYear(new DayOfMonth(DayOfMonth.Position.DAY, 12),
        Beam.Date.Month.DECEMBER));
    input.props = {...previous, value};
    input.componentDidUpdate(previous);
    assert.equal(input.state.value, value);
    input.onDayChange(new DayOfMonth(DayOfMonth.Position.DAY, 14));
    const occurrence = commits.at(-1).occurrence;
    assert.ok(occurrence instanceof DayOfYear);
    assert.equal(occurrence.month, Beam.Date.Month.DECEMBER);
    for(const property of ['readOnly', 'disabled']) {
      input.props = {...input.props, [property]: true};
      input.onIntervalChange(new Interval(1, Interval.Unit.DAY));
      input.onDayChange(new DayOfMonth(DayOfMonth.Position.DAY, 1));
      input.onMonthChange(Beam.Date.Month.JANUARY);
      assert.equal(commits.length, 1);
      input.props = {...input.props, [property]: false};
    }
  });
});
