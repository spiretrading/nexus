import * as assert from 'node:assert/strict';
import { describe, it } from 'node:test';
import * as React from 'react';
import { DayOfMonthInput } from '../../source/components/day_of_month_input';
import { DayOfMonth } from '../../source/models/day_of_month';
import { Weekday } from '../../source/models/date_rule';

function makeInput(value: DayOfMonth, echoInput = true) {
  const drafts: DayOfMonth[] = [];
  const commits: DayOfMonth[] = [];
  const input = new DayOfMonthInput({value, onInput: value => {
    drafts.push(value);
    if(echoInput) {
      replace(value);
    }
  }, onChange: value => {
    commits.push(value);
    replace(value);
  }});
  input.setState = update => {
    Object.assign(input, {state: {...input.state, ...update}});
  };
  function replace(value: DayOfMonth) {
    const previous = input.props;
    Object.assign(input, {props: {...previous, value}});
    input.componentDidUpdate(previous);
  }
  function fields(): React.ReactElement[] {
    return React.Children.toArray(input.render().props.children).
      filter(React.isValidElement) as React.ReactElement[];
  }
  return {input, drafts, commits, replace, fields};
}

describe('DayOfMonthInput', () => {
  it('retains_each_mode_value_and_accepts_external_updates', () => {
    const fixture = makeInput(new DayOfMonth(DayOfMonth.Position.SECOND,
      Weekday.TUESDAY));
    fixture.fields()[0].props.onChange(DayOfMonth.Position.DAY);
    assert.equal(fixture.input.props.value.day, 1);
    fixture.fields()[1].props.onChange(23);
    fixture.fields()[0].props.onChange(DayOfMonth.Position.LAST);
    assert.equal(fixture.input.props.value.day, Weekday.TUESDAY);
    fixture.fields()[1].props.onChange(DayOfMonth.DayType.WEEKDAY);
    fixture.fields()[0].props.onChange(DayOfMonth.Position.DAY);
    assert.equal(fixture.input.props.value.day, 23);
    fixture.replace(new DayOfMonth(DayOfMonth.Position.DAY, 17));
    fixture.fields()[0].props.onChange(DayOfMonth.Position.FIRST);
    assert.equal(fixture.input.props.value.day, DayOfMonth.DayType.WEEKDAY);
    fixture.fields()[0].props.onChange(DayOfMonth.Position.DAY);
    assert.equal(fixture.input.props.value.day, 17);
  });

  it('commits_numeric_drafts_with_and_without_parent_echo', () => {
    for(const echo of [false, true]) {
      const fixture = makeInput(new DayOfMonth(DayOfMonth.Position.DAY, 1),
        echo);
      for(const [text, draft, committed] of [
          ['12', 12, 12], ['', null, null], ['99', 99, 31], ['0', 0, 1]]) {
        fixture.fields()[1].props.onInput({currentTarget: {value: text}});
        assert.equal(fixture.drafts.at(-1).day, draft);
        assert.equal(fixture.fields()[1].props.value, draft);
        const count = fixture.commits.length;
        fixture.input.render().props.onBlur({target: {tagName: 'INPUT'}});
        assert.equal(fixture.commits.length, count + 1);
        assert.equal(fixture.commits.at(-1).day, committed);
        assert.equal(fixture.fields()[1].props.value, committed);
        fixture.input.render().props.onBlur({target: {tagName: 'INPUT'}});
        assert.equal(fixture.commits.length, count + 1);
      }
    }
  });

  it('does_not_duplicate_integer_commits_or_commit_external_replacements',
      () => {
    const fixture = makeInput(new DayOfMonth(DayOfMonth.Position.DAY, 1));
    fixture.fields()[1].props.onInput({currentTarget: {value: '99'}});
    fixture.fields()[1].props.onChange(98);
    fixture.input.render().props.onBlur({target: {tagName: 'INPUT'}});
    assert.equal(fixture.commits.length, 1);
    assert.equal(fixture.commits[0].day, 31);
    fixture.fields()[1].props.onInput({currentTarget: {value: '2'}});
    fixture.replace(new DayOfMonth(DayOfMonth.Position.DAY, 15));
    fixture.input.render().props.onBlur({target: {tagName: 'INPUT'}});
    assert.equal(fixture.commits.length, 1);
    assert.equal(fixture.input.props.value.day, 15);
  });

  it('suppresses_edits_when_read_only_or_disabled', () => {
    for(const property of ['readOnly', 'disabled']) {
      const fixture = makeInput(new DayOfMonth(DayOfMonth.Position.DAY, 1));
      Object.assign(fixture.input, {
        props: {...fixture.input.props, [property]: true}});
      fixture.fields()[0].props.onChange(DayOfMonth.Position.LAST);
      fixture.fields()[1].props.onInput({currentTarget: {value: '12'}});
      fixture.fields()[1].props.onChange(12);
      assert.equal(fixture.drafts.length, 0);
      assert.equal(fixture.commits.length, 0);
    }
  });
});
