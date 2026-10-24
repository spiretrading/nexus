import { StyleSheetTestUtils } from 'aphrodite/no-important';
import * as assert from 'node:assert/strict';
import { after, before, describe, it } from 'node:test';
import * as Beam from 'beam';
import * as React from 'react';
import { DurationInput } from '../../source/components/duration_input';

function makeInput(value: Beam.Duration, onChange: (value: Beam.Duration) =>
    void): DurationInput {
  const input = new DurationInput({value, onChange});
  input.setState = (update: any, callback?: () => void) => {
    if(typeof update === 'function') {
      update = update(input.state, input.props);
    }
    Object.assign(input.state, update,
      DurationInput.getDerivedStateFromProps(input.props, input.state));
    callback?.();
  };
  return input;
}

function change(input: DurationInput, label: string, value: number): void {
  const field = React.Children.toArray(input.render().props.children).
    find(child => React.isValidElement(child) &&
      child.props['aria-label'] === label) as React.ReactElement;
  field.props.onChange(value);
}

function receive(input: DurationInput, value: Beam.Duration): void {
  Object.assign(input.props, {value});
  Object.assign(input.state,
    DurationInput.getDerivedStateFromProps(input.props, input.state));
}

describe('DurationInput', () => {
  before(() => StyleSheetTestUtils.suppressStyleInjection());
  after(() => StyleSheetTestUtils.clearBufferAndResumeStyleInjection());

  it('completes_empty_input_in_any_order', () => {
    const values: Record<string, number> = {Hours: 12, Minutes: 34, Seconds: 56};
    for(const order of [
        ['Hours', 'Minutes', 'Seconds'], ['Hours', 'Seconds', 'Minutes'],
        ['Minutes', 'Hours', 'Seconds'], ['Minutes', 'Seconds', 'Hours'],
        ['Seconds', 'Hours', 'Minutes'], ['Seconds', 'Minutes', 'Hours']]) {
      const changes: Beam.Duration[] = [];
      const input = makeInput(null, value => changes.push(value));
      change(input, order[0], values[order[0]]);
      receive(input, null);
      change(input, order[1], values[order[1]]);
      assert.equal(changes.length, 0);
      change(input, order[2], values[order[2]]);
      assert.equal(changes.length, 1);
      assert.deepEqual(changes[0].split(), {hours: 12, minutes: 34, seconds: 56});
    }
  });

  it('clears_and_reenters_zero', () => {
    const changes: Beam.Duration[] = [];
    const input = makeInput(Beam.Duration.HOUR, value => {
      changes.push(value);
      receive(input, value);
    });
    for(const label of ['Hours', 'Minutes', 'Seconds']) {
      change(input, label, undefined);
    }
    assert.deepEqual(changes, [undefined]);
    for(const label of ['Hours', 'Minutes', 'Seconds']) {
      change(input, label, 0);
    }
    assert.equal(changes.length, 2);
    assert.ok(changes[1].equals(new Beam.Duration(0)));
  });

  it('synchronizes_external_values_without_discarding_drafts_on_rerender', () => {
    const changes: Beam.Duration[] = [];
    const input = makeInput(Beam.Duration.HOUR, value => changes.push(value));
    change(input, 'Minutes', undefined);
    receive(input, Beam.Duration.HOUR.multiply(1));
    change(input, 'Seconds', 30);
    assert.equal(changes.length, 0);
    receive(input, Beam.Duration.HOUR.multiply(2));
    change(input, 'Minutes', 15);
    assert.deepEqual(changes[0].split(), {hours: 2, minutes: 15, seconds: 0});
    receive(input, null);
    change(input, 'Hours', 9);
    change(input, 'Minutes', 8);
    assert.equal(changes.length, 1);
    change(input, 'Seconds', 7);
    assert.deepEqual(changes[1].split(), {hours: 9, minutes: 8, seconds: 7});
  });
});
