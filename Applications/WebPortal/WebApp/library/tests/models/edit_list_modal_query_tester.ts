import { StyleSheetTestUtils } from 'aphrodite/no-important';
import * as assert from 'node:assert/strict';
import { after, before, describe, it } from 'node:test';
import * as React from 'react';
import { ComboBox } from '../../source/components/combo_box';
import { EditListModal } from
  '../../source/components/edit_list_modal/edit_list_modal';
import { SuggestionsWindow } from '../../source/components/suggestions_window';
import { LocalQueryModel, QueryModel } from '../../source/models';

function find(element: React.ReactElement, predicate: (props: any) => boolean):
    React.ReactElement {
  if(predicate(element.props)) {
    return element;
  }
  for(const child of React.Children.toArray(element.props.children)) {
    if(React.isValidElement(child)) {
      const match = find(child, predicate);
      if(match) {
        return match;
      }
    }
  }
  return null;
}

function makeSynchronous<C extends React.Component<any, any>>(component: C): C {
  component.setState = update => {
    if(typeof update === 'function') {
      Object.assign(component.state, update(component.state, component.props));
    } else {
      Object.assign(component.state, update);
    }
  };
  return component;
}

function makeModal<T>(model: QueryModel<T>, selected: T[]) {
  return makeSynchronous(new EditListModal({model, selected, getLabel: String,
    title: 'Edit', listHeading: 'Selected'}));
}

describe('EditListModal queries', () => {
  before(() => StyleSheetTestUtils.suppressStyleInjection());
  after(() => StyleSheetTestUtils.clearBufferAndResumeStyleInjection());

  it('opaque_suggestions', async () => {
    let pending: () => Promise<void>;
    const queries: string[] = [];
    const originalWindow = globalThis.window;
    globalThis.window = {
      setTimeout: (callback: () => Promise<void>) => {
        pending = callback;
        return 1;
      },
      clearTimeout: () => { pending = null; }
    } as unknown as Window & typeof globalThis;
    try {
      const model: QueryModel<string> = {
        parse: async () => null,
        submit: async query => {
          queries.push(query);
          return ['Zulu', 'Selected', 'Alpha', 'Zulu'];
        }
      };
      const modal = makeModal(model, ['Selected']);
      const editor = find(modal.render(), props => props.model !== undefined);
      const combo = makeSynchronous(new ComboBox<string>(editor.props));
      const input = find(combo.render(), props => props.role === 'combobox');
      for(const query of ['  id:42 !  ', '   ']) {
        input.props.onChange({target: {value: query}});
        const callback = pending;
        pending = null;
        await callback();
        const window = find(combo.render(), props => props.items !== undefined);
        const suggestions = find(new SuggestionsWindow(window.props).render(),
          props => props.role === 'listbox');
        const labels = React.Children.toArray(suggestions.props.children).
          filter(React.isValidElement<any>).
          filter(child => child.props.role === 'option').
          map(child => child.props.children);
        assert.deepEqual(labels, ['Zulu', 'Alpha']);
      }
      input.props.onChange({target: {value: ''}});
      assert.equal(pending, null);
      assert.deepEqual(queries, ['  id:42 !  ', '   ']);
    } finally {
      globalThis.window = originalWindow;
    }
  });

  it('csv_uses_parse_and_preserves_atomicity', async () => {
    const originalReader = globalThis.FileReader;
    let content: string;
    let imported: Promise<void>;
    globalThis.FileReader = class {
      public readAsText(): void {
        this.result = content;
        imported = this.onload();
      }

      public result: string;
      public onload: () => Promise<void>;
    } as unknown as typeof FileReader;
    try {
      const model = new LocalQueryModel<number>(String);
      model.add('Zero alias', 0);
      model.add('One alias', 1);
      const modal = makeModal(model, [2]);
      const upload = find(modal.render(), props => props.type === 'file');
      content = 'Zero alias,One alias';
      upload.props.onChange({target: {files: [{}], value: ''}});
      await imported;
      assert.deepEqual(modal.state.selected, [2, 0, 1]);
      model.add('Three alias', 3);
      content = 'Three alias,Unknown';
      upload.props.onChange({target: {files: [{}], value: ''}});
      await imported;
      assert.deepEqual(modal.state.selected, [2, 0, 1]);
      assert.match(modal.state.error, /Unknown or ambiguous item: Unknown/);
      assert.equal(modal.state.importing, false);
    } finally {
      globalThis.FileReader = originalReader;
    }
  });
});
