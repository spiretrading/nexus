import ArrayModel from '../../../../webapp/utils/chainable-models/array-model';
import DataChangeType from '../../../../webapp/utils/chainable-models/data-change-type';

describe('ArrayModel', function() {
  beforeAll(function() {
    this.columnNames = ['Column A', 'Column B', 'Column C'];
  });

  it('Construtor - Null column names', function() {
    let testConstructor = function() {
      new ArrayModel(null);
    };
    let errorMessage = 'Column names cannot be null';
    expect(testConstructor).toThrow(new TypeError(errorMessage));
  });

  describe('addRow', function() {
    let arrayModel;

    beforeEach(function() {
      arrayModel = new ArrayModel(this.columnNames);
    });

    it('Invalid column length data', function() {
      let testMethod = function() {
        arrayModel.addRow(['r0c0', 'r0c1'])
      };
      let errorMessage = 'Mismatching data length. Expecting: 3, Actual: 2';

      expect(testMethod).toThrow(new RangeError(errorMessage));
    });

    it('Valid', function(done) {
      arrayModel.addDataChangeListener(function(dataChangeType, rowIndex) {
        expect(dataChangeType).toBe(DataChangeType.ADD);
        expect(rowIndex).toBe(0);
        done();
      });

      arrayModel.addRow(['r0c0', 'r0c1', 'r0c2']);
      let rowCount = arrayModel.getRowCount();
      let valueAtZeroOne = arrayModel.getValueAt(1, 0);

      expect(rowCount).toBe(1);
      expect(valueAtZeroOne).toBe('r0c1');
    });
  })

  describe('removeRow', function() {
    let arrayModel;

    beforeEach(function() {
      arrayModel = new ArrayModel(this.columnNames);
      arrayModel.addRow(['r0c0', 'r0c1', 'r0c2'])
    });

    it('Valid index', function(done) {
      arrayModel.addDataChangeListener(function(dataChangeType, payload) {
        expect(dataChangeType).toBe(DataChangeType.REMOVE);
        expect(payload.index).toBe(0);
        expect(payload.row[1]).toBe('r0c1');
        let manipulateRowArray = function() {
          payload.row.push('hello');
        }
        expect(manipulateRowArray).toThrow(new TypeError('Cannot add property 3, object is not extensible'));
        done();
      });
      arrayModel.removeRow(0);
      let rowCount = arrayModel.getRowCount();

      expect(rowCount).toBe(0);
    });

    it('Invalid index', function() {
      let testMethod = function() {
        arrayModel.removeRow(1);
      };
      expect(testMethod).toThrow(new RangeError('Invalid row index. Max: 0, Actual: 1'));
    });
  });

  describe('updateRow', function() {
    let arrayModel;

    beforeEach(function() {
      arrayModel = new ArrayModel(this.columnNames);
      arrayModel.addRow(['r0c0', 'r0c1', 'r0c2'])
    });

    it('Valid', function(done) {
      arrayModel.addDataChangeListener(function(dataChangeType, payload) {
        expect(dataChangeType).toBe(DataChangeType.UPDATE);
        expect(payload.index).toBe(0);
        expect(payload.original[1]).toBe('r0c1');
        let manipulateOriginalArray = function() {
          payload.original.push('hello');
        }
        expect(manipulateOriginalArray).toThrow(new TypeError('Cannot add property 3, object is not extensible'));
        done();
      });
      let newData = ['r0c0c0', 'r0c1c1', 'r0c2c2'];
      arrayModel.updateRow(0, newData);
      let rowCount = arrayModel.getRowCount();
      let cellValue = arrayModel.getValueAt(1, 0);

      expect(rowCount).toBe(1);
      expect(cellValue).toBe('r0c1c1');
    });

    it('Invalid index', function() {
      let testMethod = function() {
        let newData = ['r0c0c0', 'r0c1c1', 'r0c2c2'];
        arrayModel.updateRow(100, newData);
      };
      expect(testMethod).toThrow(new RangeError('Invalid row index. Max: 0, Actual: 100'));
    });
  });

  describe('getRowCount', function() {
    it('Valid', function() {
      let arrayModel = new ArrayModel(this.columnNames);
      arrayModel.addRow(['r0c0', 'r0c1', 'r0c2']);
      let rowCount = arrayModel.getRowCount();
      expect(rowCount).toBe(1);
    });
  });

  describe('getColumnCount', function() {
    it('Valid', function() {
      let arrayModel = new ArrayModel(this.columnNames);
      let columnCount = arrayModel.getColumnCount();
      expect(columnCount).toBe(3);
    });
  });

  describe('getColumnName', function() {
    let arrayModel;

    beforeEach(function() {
      arrayModel = new ArrayModel(this.columnNames);
    });

    it('Valid', function() {
      let name = arrayModel.getColumnName(1);
      expect(name).toBe('Column B');
    });

    it('Invalid index', function() {
      let testMethod = function() {
        arrayModel.getColumnName(100);
      };
      expect(testMethod).toThrow(new RangeError('Invalid column index. Max: 2, Actual: 100'));
    });
  });

  describe('getValueAt', function() {
    let arrayModel;

    beforeEach(function() {
      arrayModel = new ArrayModel(this.columnNames);
      let newData = ['r0c0', 'r0c1', 'r0c2'];
      arrayModel.addRow(newData);
    });

    it('Valid', function() {
      let value = arrayModel.getValueAt(2, 0);
      expect(value).toBe('r0c2');
    });

    it('Invalid x index', function() {
      let testMethod = function() {
        arrayModel.getValueAt(100, 0);
      };
      expect(testMethod).toThrow(new RangeError('Invalid column index. Max: 2, Actual: 100'));
    });

    it('Invalid y index', function() {
      let testMethod = function() {
        arrayModel.getValueAt(1, 100);
      };
      expect(testMethod).toThrow(new RangeError('Invalid row index. Max: 0, Actual: 100'));
    });
  });

  describe('addDataChangeListener', function() {
    it('Valid', function(done) {
      let arrayModel = new ArrayModel(this.columnNames);
      arrayModel.addDataChangeListener(function(dataChangeType, rowIndex) {
        expect(dataChangeType).toBe(DataChangeType.ADD);
        expect(rowIndex).toBe(0);
        done();
      });
      arrayModel.addRow(['r0c0', 'r0c1', 'r0c2']);
    });
  });

  describe('removeDataChangeListener', function() {
    it('Valid', function() {
      let arrayModel = new ArrayModel(this.columnNames);
      let subId = arrayModel.addDataChangeListener(function(dataChangeType, rowIndex) {
        fail();
      });
      arrayModel.removeDataChangeListener(subId);
      arrayModel.addRow(['r0c0', 'r0c1', 'r0c2']);
    });
  });
});
