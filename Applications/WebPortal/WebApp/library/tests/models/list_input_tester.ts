import { StyleSheetTestUtils } from 'aphrodite/no-important';
import * as assert from 'node:assert/strict';
import { after, before, describe, it } from 'node:test';
import { ListInput } from '../../source/components/list_input';
import { LocalQueryModel } from '../../source/models';

function makeInput(overrides = {}): any {
  const input = new ListInput<string>({value: ['Alpha'],
    model: new LocalQueryModel(String), getLabel: String,
    title: 'Edit List', listHeading: 'Selected', ...overrides});
  input.setState = (update: any, callback?: () => void) => {
    Object.assign(input, {state: {...input.state, ...update}});
    callback?.();
  };
  return input;
}

function event(overrides = {}): any {
  return {key: ' ', defaultPrevented: false, repeat: false,
    nativeEvent: {isComposing: false},
    preventDefault() { this.defaultPrevented = true; }, ...overrides};
}

describe('ListInput', () => {
  before(() => StyleSheetTestUtils.suppressStyleInjection());
  after(() => StyleSheetTestUtils.clearBufferAndResumeStyleInjection());

  it('focus_does_not_open_and_space_or_click_opens', () => {
    let focused = 0;
    const input = makeInput({onFocus: () => ++focused});
    const field = input.render().props.children[0].props;
    field.onFocus(event());
    assert.equal(focused, 1);
    assert.equal(input.state.isOpen, false);
    for(const key of ['Tab', 'Enter', 'ArrowDown']) {
      field.onKeyDown(event({key}));
      assert.equal(input.state.isOpen, false);
    }
    field.onKeyDown(event({repeat: true}));
    field.onKeyDown(event({nativeEvent: {isComposing: true}}));
    assert.equal(input.state.isOpen, false);
    const space = event();
    field.onKeyDown(space);
    assert.equal(space.defaultPrevented, true);
    assert.equal(input.state.isOpen, true);
    input.input.current = {focus: () => field.onFocus(event())};
    input.render().props.children[1].props.onClose();
    assert.equal(focused, 2);
    assert.equal(input.state.isOpen, false);
    field.onClick(event());
    assert.equal(input.state.isOpen, true);
  });

  it('honors_caller_event_handlers', () => {
    let keys = 0;
    let clicks = 0;
    const input = makeInput({onKeyDown: (event: any) => {
      ++keys;
      event.preventDefault();
    }, onClick: (event: any) => {
      ++clicks;
      event.preventDefault();
    }});
    const field = input.render().props.children[0].props;
    field.onKeyDown(event());
    field.onClick(event());
    assert.equal(keys, 1);
    assert.equal(clicks, 1);
    assert.equal(input.state.isOpen, false);
  });

  it('disabled_controls_stay_closed_and_readonly_controls_allow_viewing', () => {
    const disabled = makeInput({disabled: true});
    const field = disabled.render().props.children[0].props;
    field.onClick(event());
    field.onKeyDown(event());
    assert.equal(disabled.state.isOpen, false);
    const readonly = makeInput({readOnly: true});
    readonly.render().props.children[0].props.onKeyDown(event());
    assert.equal(readonly.state.isOpen, true);
    assert.equal(readonly.render().props.children[1].props.readOnly, true);
  });
});
