import * as assert from 'node:assert/strict';
import { describe, it } from 'node:test';
import { FilteredQueryModel, QueryModel } from '../../source/models';

describe('FilteredQueryModel', () => {
  it('filters_latest_selection_and_preserves_queries', async () => {
    const queries: string[] = [];
    let resolve: (values: readonly string[]) => void;
    let excluded = ['Alpha'];
    const source: QueryModel<string> = {
      parse: async query => {
        queries.push(query);
        return 'Beta';
      },
      submit: query => {
        queries.push(query);
        return new Promise(result => { resolve = result; });
      }
    };
    const model = new FilteredQueryModel(source,
      value => !excluded.includes(value));
    const pending = model.submit('  id:42 !  ');
    excluded = ['Beta'];
    const values = ['Gamma', 'Beta', 'Alpha'];
    resolve(values);
    assert.deepEqual(await pending, ['Gamma', 'Alpha']);
    assert.deepEqual(values, ['Gamma', 'Beta', 'Alpha']);
    assert.equal(await model.parse('  Beta alias  '), 'Beta');
    assert.deepEqual(queries, ['  id:42 !  ', '  Beta alias  ']);
  });
});
