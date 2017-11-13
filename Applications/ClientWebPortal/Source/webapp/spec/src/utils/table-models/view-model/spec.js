import ViewModel from '../../../../webapp/utils/table-models/view-model';
import ArrayModel from '../../../../webapp/utils/table-models/array-model';
import SignalManager from '../../../../webapp/utils/signal-manager';
import DataChangeType from '../../../../webapp/utils/table-models/model/data-change-type';
import DefaultStyleRule from '../../../../webapp/utils/table-models/default-style-rule';

describe('ViewModel', function() {
  beforeAll(function() {
    this.columnNames = ['Number', 'String', 'Name'];
    this.sourceModel = new ArrayModel(this.columnNames);
    initializeSourceModel(this.sourceModel);
    this.viewModel = new ViewModel(this.sourceModel);
  });

  it('getRowCount', function() {
    expect(this.viewModel.getRowCount()).toBe(2);
  });

  it('getColumnCount', function() {
    expect(this.viewModel.getColumnCount()).toBe(3);
  });

  it('getColumnName', function() {
    expect(this.viewModel.getColumnName(2)).toBe('Name');
  });

  describe('getValueAt', function(){
    it('Number - even number row', function() {
      let value = this.viewModel.getValueAt(0,0);
      expect(value.value).toBe(12345678910);
      expect(value.display).toBe('12,345,678,910');
    });

    it('String - even number row', function() {
      let value = this.viewModel.getValueAt(1,0);
      expect(value.value).toBe('Some String Value 1');
      expect(value.display).toBe('Some String Value 1');
    });

    it('All others', function() {
      let value = this.viewModel.getValueAt(2,0);
      expect(value.value.name).toBe('John Doe');
      expect(value.display).toBe('John Doe');
    });
  });

  describe('onDataChange data transformation', function() {
    let sourceModel, viewModel;

    beforeEach(function() {
      sourceModel = new ArrayModel(this.columnNames);
      initializeSourceModel(sourceModel);
      viewModel = new ViewModel(sourceModel);
    });

    it('Row remove', function(done) {
      viewModel.addDataChangeListener(function(dataChangeType, payload) {
        expect(dataChangeType).toBe(DataChangeType.REMOVE);
        expect(payload.index).toBe(1);
        expect(payload.row[0].value).toBe(5123478);
        expect(payload.row[0].display).toBe('5,123,478');
        expect(payload.row[0].style instanceof DefaultStyleRule).toBe(true);
        done();
      });
      sourceModel.removeRow(1);
    });

    it('Row update', function(done) {
      viewModel.addDataChangeListener(function(dataChangeType, payload) {
        expect(dataChangeType).toBe(DataChangeType.UPDATE);
        expect(payload.index).toBe(1);
        expect(payload.original[2].value instanceof MockName).toBe(true);
        expect(payload.original[2].display).toBe('Will Smith');
        expect(payload.original[2].style instanceof DefaultStyleRule).toBe(true);
        done();
      });
      sourceModel.updateRow(1, [
        111111111,
        'Some String Value 3',
        new MockName('Test Name')
      ]);
    });
  });
});

function initializeSourceModel(sourceModel) {
  sourceModel.addRow([
    12345678910,
    'Some String Value 1',
    new MockName('John Doe')
  ]);

  sourceModel.addRow([
    5123478,
    'Some String Value 2',
    new MockName('Will Smith')
  ]);
}

class MockName {
  constructor(name) {
    this.name = name;
  }

  toString() {
    return this.name;
  }
}
