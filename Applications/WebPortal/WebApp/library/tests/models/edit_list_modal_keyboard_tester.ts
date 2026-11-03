import { StyleSheetTestUtils } from 'aphrodite/no-important';
import * as assert from 'node:assert/strict';
import { after, before, describe, it } from 'node:test';
import * as React from 'react';
import { EditListModal } from
  '../../source/components/edit_list_modal/edit_list_modal';
import { LocalQueryModel } from '../../source/models';

function findList(element: React.ReactElement): React.ReactElement {
  if(element.props.items && element.props.onRemove) {
    return element;
  }
  for(const child of React.Children.toArray(element.props.children)) {
    if(React.isValidElement(child)) {
      const found = findList(child);
      if(found) {
        return found;
      }
    }
  }
  return null;
}

function makeList(readOnly = false): any {
  const modal: any = new EditListModal({selected: ['Alpha', 'Beta', 'Gamma'],
    model: new LocalQueryModel(String), getLabel: String,
    title: 'Edit Items', listHeading: 'Added Items', readOnly});
  const element: any = findList(modal.render());
  const list = new element.type(element.props);
  const result = {modal, list, focused: -1};
  modal.setState = (update: any, callback?: () => void) => {
    if(typeof update === 'function') {
      update = update(modal.state);
    }
    modal.state = {...modal.state, ...update};
    list.props = {...list.props, items: modal.state.selected,
      selection: modal.state.removal};
    callback?.();
  };
  modal.list.current = list;
  list.element.current = {
    querySelectorAll: () => list.props.items.map((_: any, index: number) => ({
      focus: () => {
        result.focused = index;
        list.props.onSelect(index);
      }
    })),
    focus: () => { result.focused = -1; }
  };
  return result;
}

function key(value: string, composing = false): any {
  return {key: value, defaultPrevented: false,
    nativeEvent: {isComposing: composing},
    preventDefault() { this.defaultPrevented = true; }};
}

describe('EditListModal keyboard', () => {
  before(() => StyleSheetTestUtils.suppressStyleInjection());
  after(() => StyleSheetTestUtils.clearBufferAndResumeStyleInjection());

  it('arrows_select_rows_and_delete_keeps_focus_in_the_list', () => {
    const state = makeList();
    const {list, modal} = state;
    list.focus(0);
    const up = key('ArrowUp');
    list.onKeyDown(up, 0);
    assert.equal(up.defaultPrevented, true);
    assert.equal(state.focused, 0);
    list.onKeyDown(key('ArrowDown'), 0);
    assert.equal(state.focused, 1);
    assert.equal(modal.state.removal, 1);
    const remove = key('Delete');
    list.onKeyDown(remove, 1);
    assert.equal(remove.defaultPrevented, true);
    assert.deepEqual(modal.state.selected, ['Alpha', 'Gamma']);
    assert.equal(state.focused, 1);
    list.onKeyDown(key('ArrowDown'), 1);
    assert.equal(state.focused, 1);
    list.onKeyDown(key('Delete'), 1);
    assert.deepEqual(modal.state.selected, ['Alpha']);
    assert.equal(state.focused, 0);
    list.onKeyDown(key('Delete'), 0);
    assert.deepEqual(modal.state.selected, []);
    assert.equal(modal.state.removal, -1);
    assert.equal(state.focused, -1);
  });

  it('readonly_and_composition_do_not_remove_items', () => {
    const readonly = makeList(true);
    readonly.list.onKeyDown(key('Delete'), 1);
    readonly.modal.remove(1);
    assert.equal(readonly.modal.state.selected.length, 3);
    const editable = makeList();
    editable.list.onKeyDown(key('Delete', true), 1);
    assert.equal(editable.modal.state.selected.length, 3);
    editable.list.onKeyDown(key('Tab'), 1);
    assert.equal(editable.focused, -1);
  });
});
