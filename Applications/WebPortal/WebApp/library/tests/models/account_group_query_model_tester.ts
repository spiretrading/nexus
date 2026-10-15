import * as Beam from 'beam';
import * as Nexus from 'nexus';
import * as assert from 'node:assert/strict';
import { describe, it } from 'node:test';
import { HttpAccountGroupQueryModel, LocalAccountGroupQueryModel } from
  '../../source/components/edit_account_group_modal';
import { QueryModel } from '../../source/models/query_model';

describe('AccountGroupQueryModel', () => {
  it('names_and_prefixes', async () => {
    const account = Beam.DirectoryEntry.makeAccount(1, 'Alice');
    const group = Beam.DirectoryEntry.makeDirectory(2, 'Alpha');
    const model: QueryModel<Beam.DirectoryEntry> =
      new LocalAccountGroupQueryModel([account, group]);
    assert.deepEqual(await model.submit(' aL '), [account, group]);
    assert.deepEqual(await model.submit('lic'), []);
    assert.deepEqual(await model.submit(''), []);
    assert.equal(await model.parse(' ALICE '), account);
    assert.equal(await model.parse('Al'), null);
    assert.equal(await model.parse('missing'), null);
  });

  it('ambiguous_names_and_duplicate_ids', async () => {
    const account = Beam.DirectoryEntry.makeAccount(1, 'Shared');
    const group = Beam.DirectoryEntry.makeDirectory(2, 'Shared');
    const model = new LocalAccountGroupQueryModel([account, group]);
    assert.deepEqual(await model.submit('shared'), [account, group]);
    assert.equal(await model.parse('shared'), null);
    const duplicate = Beam.DirectoryEntry.makeAccount(1, 'Shared');
    const duplicates = new LocalAccountGroupQueryModel([account, duplicate]);
    assert.ok((await duplicates.parse('shared')).equals(account));
  });

  it('http_lookup', async () => {
    const account = Beam.DirectoryEntry.makeAccount(1, 'Alice');
    const group = Beam.DirectoryEntry.makeDirectory(2, 'Alpha');
    const queries: string[] = [];
    const client = {
      searchAccounts: async (query: string) => {
        queries.push(query);
        return [[group, account], [group, group], [group, account]];
      }
    } as unknown as Nexus.AdministrationClient;
    const model = new HttpAccountGroupQueryModel(client);
    assert.deepEqual(await model.submit(' Al '), [account, group, account]);
    assert.ok((await model.parse('Alice')).equals(account));
    assert.equal(await model.parse('Al'), null);
    assert.deepEqual(await model.submit(' '), []);
    assert.deepEqual(queries, ['Al', 'Alice', 'Al']);
  });

  it('http_errors', async () => {
    const error = new Error('Lookup failed.');
    const client = {
      searchAccounts: async () => { throw error; }
    } as unknown as Nexus.AdministrationClient;
    const model = new HttpAccountGroupQueryModel(client);
    await assert.rejects(model.submit('Alice'), error);
    await assert.rejects(model.parse('Alice'), error);
  });
});
