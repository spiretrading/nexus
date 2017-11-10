import IndexedModel from '../../../../webapp/utils/table-models/indexed-model';
import DataChangeType from '../../../../webapp/utils/table-models/notify-change-model/data-change-type';

describe('IndexedModel', function() {
  beforeAll(function() {
    this.columnNames = ['Column A', 'Column B', 'Column C'];
  });

  describe('constructor', function(){
    it('Indices out of bounds', function() {
      let testConstructor = function() {
        new IndexedModel([-1, 2], this.columnNames);
      }.bind(this);
      let errorMessage = 'Index -1 is out of bounds';
      expect(testConstructor).toThrow(new RangeError(errorMessage));

      testConstructor = function() {
        new IndexedModel([0, 100], this.columnNames);
      }.bind(this);
      errorMessage = 'Index 100 is out of bounds';
      expect(testConstructor).toThrow(new RangeError(errorMessage));
    });

    it('Valid', function() {
      new IndexedModel([0, 2], this.columnNames);
    });
  });

  describe('update', function() {
    let indexedModel;

    beforeEach(function() {
      indexedModel = new IndexedModel([2,1], this.columnNames);
    });

    it('Non-existent values should add', function(done) {
      indexedModel.addDataChangeListener(function(dataChangeType, rowIndex) {
        expect(dataChangeType).toBe(DataChangeType.ADD);
        expect(rowIndex).toBe(0);
        let cellValue = indexedModel.getValueAt(1, 0);
        expect(cellValue).toBe(1);
        done();
      });

      let values = [0, 1, 2];
      indexedModel.update(values);
      let rowCount = indexedModel.getRowCount();
      expect(rowCount).toBe(1);
      expect(indexedModel.getValueAt(1, 0)).toBe(1);
    });

    it('Existing values should update', function(done) {
      let values = [0, 1, 2];
      indexedModel.update(values);  // add initial values
      indexedModel.addDataChangeListener(function(dataChangeType, payload) {
        expect(dataChangeType).toBe(DataChangeType.UPDATE);
        expect(payload.index).toBe(0);
        expect(payload.original[1]).toBe(1);
        done();
      });
      let newData = ['0', '1', '2'];
      indexedModel.update(newData);
      let rowCount = indexedModel.getRowCount();
      let cellValue = indexedModel.getValueAt(1, 0);

      expect(rowCount).toBe(1);
      expect(cellValue).toBe('1');
    });
  })

  describe('removeRow', function() {
    let indexedModel;

    beforeEach(function() {
      indexedModel = new IndexedModel([2,1], this.columnNames);
      indexedModel.update(['r0c0', 'r0c1', 'r0c2'])
    });

    it('Valid remove', function(done) {
      indexedModel.addDataChangeListener(function(dataChangeType, payload) {
        expect(dataChangeType).toBe(DataChangeType.REMOVE);
        expect(payload.index).toBe(0);
        expect(payload.row[1]).toBe('r0c1');
        done();
      });
      indexedModel.removeRow(['r0c0', 'r0c1', 'r0c2']);
      let rowCount = indexedModel.getRowCount();

      expect(rowCount).toBe(0);
    });

    it('Non-existent values', function() {
      indexedModel.removeRow(['r0c0', 'rrrr', 'r0c2']);
      let rowCount = indexedModel.getRowCount();
      expect(rowCount).toBe(1);
    });
  });

  describe('getRowCount', function() {
    it('Valid', function() {
      let indexedModel = new IndexedModel([2,1], this.columnNames);
      indexedModel.update(['r0c0', 'r0c1', 'r0c2']);
      let rowCount = indexedModel.getRowCount();
      expect(rowCount).toBe(1);
    });
  });

  describe('getColumnCount', function() {
    it('Valid', function() {
      let indexedModel = new IndexedModel([2,1], this.columnNames);
      let columnCount = indexedModel.getColumnCount();
      expect(columnCount).toBe(3);
    });
  });

  describe('getColumnName', function() {
    let indexedModel;

    beforeEach(function() {
      indexedModel = new IndexedModel([2,1], this.columnNames);
    });

    it('Valid', function() {
      let name = indexedModel.getColumnName(1);
      expect(name).toBe('Column B');
    });
  });

  describe('getValueAt', function() {
    let indexedModel;

    beforeEach(function() {
      indexedModel = new IndexedModel([2,1], this.columnNames);
      let newData = ['r0c0', 'r0c1', 'r0c2'];
      indexedModel.update(newData);
    });

    it('Valid', function() {
      let value = indexedModel.getValueAt(2, 0);
      expect(value).toBe('r0c2');
    });
  });

  describe('addDataChangeListener', function() {
    it('Valid', function(done) {
      let indexedModel = new IndexedModel([2,1], this.columnNames);
      indexedModel.addDataChangeListener(function(dataChangeType, rowIndex) {
        expect(dataChangeType).toBe(DataChangeType.ADD);
        expect(rowIndex).toBe(0);
        done();
      });
      indexedModel.update(['r0c0', 'r0c1', 'r0c2']);
    });
  });

  describe('removeDataChangeListener', function() {
    it('Valid', function() {
      let indexedModel = new IndexedModel([2,1], this.columnNames);
      let subId = indexedModel.addDataChangeListener(function(dataChangeType, rowIndex) {
        fail();
      });
      indexedModel.removeDataChangeListener(subId);
      indexedModel.update(['r0c0', 'r0c1', 'r0c2']);
    });
  });
});
