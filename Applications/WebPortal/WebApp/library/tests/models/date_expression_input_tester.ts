import * as assert from 'node:assert/strict';
import { describe, it } from 'node:test';
import * as Beam from 'beam';
import * as React from 'react';
import { DateExpressionInput } from
  '../../source/components/date_expression_input';
import { DateExpression, DateOffset } from
  '../../source/models/date_expression';
import { DayOfMonth } from '../../source/models/day_of_month';
import { Weekday } from '../../source/models/date_rule';

function makeInput(echo = true) {
  const drafts: DateExpression[] = [];
  const commits: DateExpression[] = [];
  const input = new DateExpressionInput({
    value: {mode: DateExpression.Mode.FIXED, date: new Beam.Date(2026, 10, 20)},
    onInput: value => {
      drafts.push(value);
      if(echo) {
        replace(value);
      }
    }, onChange: value => {
      commits.push(value);
      replace(value);
    }});
  input.setState = update => {
    Object.assign(input, {state: {...input.state, ...update}});
  };
  function replace(value: DateExpression) {
    const previous = input.props;
    Object.assign(input, {props: {...previous, value}});
    input.componentDidUpdate(previous);
  }
  function fields(element = input.render()): React.ReactElement[] {
    return React.Children.toArray(element.props.children).
      filter(React.isValidElement) as React.ReactElement[];
  }
  return {input, drafts, commits, replace, fields};
}

describe('DateExpressionInput', () => {
  it('retains_modes_and_zero_offset_direction', () => {
    const fixture = makeInput();
    fixture.fields()[0].props.onChange(DateExpression.Mode.WEEKDAY);
    fixture.fields()[1].props.onChange(Weekday.FRIDAY);
    let offset = fixture.fields(fixture.fields()[2]);
    offset[1].props.onChange('future');
    offset = fixture.fields(fixture.fields()[2]);
    assert.equal(offset[1].props.value, 'future');
    offset[0].props.onChange(2);
    assert.deepEqual(fixture.commits.at(-1), {
      mode: DateExpression.Mode.WEEKDAY, date: {day: Weekday.FRIDAY,
        offset: {value: 2, unit: DateOffset.Unit.WEEK}}});
    fixture.fields()[0].props.onChange(DateExpression.Mode.DAY_OF_MONTH);
    fixture.fields()[1].props.onChange(
      new DayOfMonth(DayOfMonth.Position.LAST, DayOfMonth.DayType.WEEKDAY));
    fixture.fields()[0].props.onChange(DateExpression.Mode.WEEKDAY);
    assert.equal(fixture.fields()[1].props.value, Weekday.FRIDAY);
    assert.equal(fixture.fields(fixture.fields()[2])[0].props.value, 2);
    fixture.fields()[0].props.onChange(DateExpression.Mode.DAY_OF_MONTH);
    assert.equal(fixture.fields()[1].props.value.position,
      DayOfMonth.Position.LAST);
    fixture.fields()[0].props.onChange(DateExpression.Mode.FIXED);
    assert.equal(fixture.fields(fixture.fields()[1])[0].props.value.day, 20);
  });

  it('commits_offset_drafts_once_with_or_without_parent_echo', () => {
    for(const echo of [false, true]) {
      const fixture = makeInput(echo);
      fixture.fields()[0].props.onChange(DateExpression.Mode.DAY_OFFSET);
      for(const [text, expected] of [['12', -12], ['', null]]) {
        const count = fixture.commits.length;
        fixture.fields(fixture.fields()[1])[0].props.onInput({
          currentTarget: {value: text}});
        assert.equal(fixture.commits.length, count);
        assert.deepEqual(fixture.drafts.at(-1), {
          mode: DateExpression.Mode.DAY_OFFSET,
          date: {value: expected, unit: DateOffset.Unit.DAY}});
        fixture.fields()[1].props.onBlur({target: {tagName: 'INPUT'}});
        fixture.fields()[1].props.onBlur({target: {tagName: 'INPUT'}});
        assert.equal(fixture.commits.length, count + 1);
        assert.deepEqual(fixture.commits.at(-1), fixture.drafts.at(-1));
      }
    }
  });

  it('accepts_external_updates_without_committing_them', () => {
    const fixture = makeInput();
    fixture.fields()[0].props.onChange(DateExpression.Mode.DAY_OFFSET);
    fixture.fields(fixture.fields()[1])[0].props.onInput({
      currentTarget: {value: '12'}});
    fixture.replace({mode: DateExpression.Mode.DAY_OFFSET,
      date: {value: 4, unit: DateOffset.Unit.DAY}});
    const count = fixture.commits.length;
    fixture.fields()[1].props.onBlur({target: {tagName: 'INPUT'}});
    assert.equal(fixture.commits.length, count);
    assert.equal(fixture.fields(fixture.fields()[1])[0].props.value, 4);
    assert.equal(fixture.fields(fixture.fields()[1])[1].props.value, 'future');
  });

  it('suppresses_edits_when_read_only_or_disabled', () => {
    for(const property of ['readOnly', 'disabled']) {
      const fixture = makeInput();
      Object.assign(fixture.input, {
        props: {...fixture.input.props, [property]: true}});
      fixture.fields()[0].props.onChange(DateExpression.Mode.DAY_OFFSET);
      fixture.fields(fixture.fields()[1])[0].props.onChange(
        new Beam.Date(2027, 1, 1));
      assert.equal(fixture.commits.length, 0);
      assert.equal(fixture.drafts.length, 0);
      assert.equal(fixture.input.state.value.mode, DateExpression.Mode.FIXED);
    }
  });

  it('resets_fixed_date_segments_for_an_external_clear', () => {
    const fixture = makeInput();
    const key = fixture.fields(fixture.fields()[1])[0].key;
    fixture.replace({mode: DateExpression.Mode.FIXED, date: null});
    assert.notEqual(fixture.fields(fixture.fields()[1])[0].key, key);
    assert.equal(fixture.commits.length, 0);
  });
});
