import * as assert from 'node:assert/strict';
import { describe, it } from 'node:test';
import { LocalQueryModel } from '../../source/models/local_query_model';
import { QueryModel } from '../../source/models/query_model';

describe('LocalQueryModel', () => {
  it('empty', async () => {
    const model: QueryModel<string> = new LocalQueryModel(value => value);
    assert.equal(await model.parse(''), null);
    assert.equal(await model.parse('missing'), null);
    assert.deepEqual(await model.submit(''), []);
    assert.deepEqual(await model.submit('missing'), []);
  });

  it('exact_names_and_prefixes', async () => {
    const model = new LocalQueryModel<string>(value => value);
    model.add('Alpha');
    model.add('Alpine');
    model.add('Beta');
    assert.equal(await model.parse('aLPHa'), 'Alpha');
    assert.equal(await model.parse('Al'), null);
    assert.deepEqual(await model.submit('aL'), ['Alpha', 'Alpine']);
    assert.deepEqual(await model.submit('lph'), []);
    assert.deepEqual(await model.submit(''), ['Alpha', 'Alpine', 'Beta']);
    assert.equal(await model.parse(' Alpha '), null);
    assert.deepEqual(await model.submit(' Al'), []);
  });

  it('aliases', async () => {
    const value = {id: 1, name: 'First'};
    const model = new LocalQueryModel<typeof value>(entry => entry.name);
    model.add(value);
    model.add('Alias', value);
    assert.equal(await model.parse('first'), value);
    assert.equal(await model.parse('ALIAS'), value);
    assert.deepEqual(await model.submit('a'), [value]);
    assert.deepEqual(await model.submit(''), [value]);
  });

  it('replace_mapping', async () => {
    const model = new LocalQueryModel<number>(String);
    model.add('Name', 1);
    model.add('Other', 2);
    model.add('NAME', 3);
    assert.equal(await model.parse('name'), 3);
    assert.deepEqual(await model.submit(''), [3, 2]);
  });

  it('value_identity', async () => {
    const first = {id: 1};
    const duplicate = {id: 1};
    const second = {id: 2};
    const model = new LocalQueryModel<typeof first>(entry => String(entry.id),
      (a, b) => a.id === b.id);
    model.add('First', first);
    model.add('Alias', duplicate);
    model.add('Second', second);
    assert.deepEqual(await model.submit(''), [first, second]);
    assert.equal(await model.parse('Alias'), duplicate);
  });

  it('falsy_values', async () => {
    const model = new LocalQueryModel<number | boolean | string>(String);
    model.add(0);
    model.add(false);
    model.add('');
    assert.equal(await model.parse('0'), 0);
    assert.equal(await model.parse('FALSE'), false);
    assert.equal(await model.parse(''), '');
    assert.deepEqual(await model.submit(''), [0, false, '']);
  });
});
