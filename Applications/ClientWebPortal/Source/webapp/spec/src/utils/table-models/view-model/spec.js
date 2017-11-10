import ViewModel from '../../../../webapp/utils/table-models/view-model';
import SignalManager from '../../../../webapp/utils/signal-manager';
import DataChangeType from '../../../../webapp/utils/table-models/model/data-change-type';
import DefaultStyleRule from '../../../../webapp/utils/table-models/default-style-rule';

describe('ViewModel', function() {
  beforeAll(function() {
    this.sourceModel = new MockSourceModel();
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
      sourceModel = new MockSourceModel();
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
      sourceModel.emitDataRemoveSignal(1);
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
      sourceModel.emitDataUpdateSignal(1);
    });
  });
});

class MockSourceModel {
  constructor() {
    this.columnNames = [
      'Number',
      'String',
      'Name'
    ];

    this.data = [
      [
        12345678910,
        'Some String Value 1',
        new MockName('John Doe')
      ],
      [
        5123478,
        'Some String Value 2',
        new MockName('Will Smith')
      ]
    ];

    this.signalManager = new SignalManager();
  }

  getRowCount() {
    return this.data.length;
  }

  getColumnCount() {
    return this.columnNames.length;
  }

  getColumnName(x) {
    return this.columnNames[x];
  }

  getValueAt(x, y) {
    return this.data[y][x];
  }

  addDataChangeListener(listener) {
    return this.signalManager.addListener(listener);
  }

  removeDataChangeListener(subId) {
    this.signalManager.removeListener(subId);
  }

  emitDataRemoveSignal(index) {
    this.signalManager.emitSignal(DataChangeType.REMOVE, {
      index: index,
      row: this.data[index]
    });
  }

  emitDataUpdateSignal(index) {
    this.signalManager.emitSignal(DataChangeType.UPDATE, {
      index: index,
      original: this.data[index]
    });
  }
}

class MockName {
  constructor(name) {
    this.name = name;
  }

  toString() {
    return this.name;
  }
}
