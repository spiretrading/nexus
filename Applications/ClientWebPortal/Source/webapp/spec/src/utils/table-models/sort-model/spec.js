import ArrayModel from '../../../../webapp/utils/table-models/array-model';
import SortModel from '../../../../webapp/utils/table-models/sort-model';
import DataChangeType from '../../../../webapp/utils/table-models/model/data-change-type';

describe('SortModel', function() {
  beforeAll(function() {
    this.sortColumns = [
      { index: 2, isAsc: true },
      { index: 1, isAsc: false }
    ];
  });

  // it('getRowCount', function() {
  //   expect(this.sumModel.getRowCount()).toBe(1);
  // });
  //
  // it('getColumnCount', function() {
  //   expect(this.sumModel.getColumnCount()).toBe(5);
  // });
  //
  // it('getColumnName', function() {
  //   expect(this.sumModel.getColumnName(1)).toBe('Column 1');
  // });

  it('constructor', function() {
    // check to see if sorting is done at initialization
    let arrayModel = createSourceModel();
    let sortModel = new SortModel(arrayModel, this.sortColumns);

    expect(sortModel.getValueAt(1, 0)).toBe('Jennifer');
    expect(sortModel.getValueAt(1, 1)).toBe('Tom');
    expect(sortModel.getValueAt(1, 2)).toBe('April');
    expect(sortModel.getValueAt(1, 3)).toBe('April');
    expect(sortModel.getValueAt(1, 4)).toBe('April');
    expect(sortModel.getValueAt(1, 5)).toBe('April');
    expect(sortModel.getValueAt(1, 6)).toBe('April');
    expect(sortModel.getValueAt(1, 7)).toBe('April');
    expect(sortModel.getValueAt(1, 8)).toBe('Josh');
    expect(sortModel.getValueAt(1, 9)).toBe('Katy');
  });

  describe('onDataChange', function(){
    let arrayModel, sortModel;

    beforeEach(function() {
      arrayModel = createSourceModel();
      sortModel = new SortModel(arrayModel, this.sortColumns);
    });

    it('Row add - to be sorted in the middle', function(done) {
      sortModel.addDataChangeListener(function(dataChangeType, payload) {
        expect(dataChangeType).toBe(DataChangeType.ADD);
        expect(payload).toBe(8);
        expect(sortModel.getValueAt(1, 8)).toBe('Alex');
        expect(sortModel.getValueAt(2, 8)).toBe(2351);
        done();
      });

      let newRow = [2341, 'Alex', 2351, 8362];
      arrayModel.addRow(newRow);
      expect(sortModel.getRowCount()).toBe(11);
    });

    it('Row add - to be sorted as head', function(done) {
      sortModel.addDataChangeListener(function(dataChangeType, payload) {
        expect(dataChangeType).toBe(DataChangeType.ADD);
        expect(payload).toBe(0);
        expect(sortModel.getValueAt(1, 0)).toBe('Alex');
        expect(sortModel.getValueAt(2, 0)).toBe(30);
        done();
      });

      let newRow = [2341, 'Alex', 30, 8362];
      arrayModel.addRow(newRow);
      expect(sortModel.getRowCount()).toBe(11);
    });

    it('Row add - to be sorted as tail', function(done) {
      sortModel.addDataChangeListener(function(dataChangeType, payload) {
        expect(dataChangeType).toBe(DataChangeType.ADD);
        expect(payload).toBe(10);
        expect(sortModel.getValueAt(1, 10)).toBe('Alex');
        expect(sortModel.getValueAt(2, 10)).toBe(7234);
        done();
      });

      let newRow = [2341, 'Alex', 7234, 8362];
      arrayModel.addRow(newRow);
      expect(sortModel.getRowCount()).toBe(11);
    });

    it('Row update - to be sorted as head', function(done) {
      let eventCount = 0;
      sortModel.addDataChangeListener(function(dataChangeType, payload) {
        eventCount++;
        if (eventCount == 1) {
          expect(dataChangeType).toBe(DataChangeType.MOVE);
          expect(payload.to).toBe(0);
        } else if (eventCount == 2) {
          expect(dataChangeType).toBe(DataChangeType.UPDATE);
          expect(payload.index).toBe(0);
          expect(sortModel.getValueAt(1, 0)).toBe('Alex');
          expect(sortModel.getValueAt(2, 0)).toBe(30);
          expect(payload.original[1]).toBe('April');
          expect(payload.original[2]).toBe(2351);
          done();
        }
      });

      let newRow = [2341, 'Alex', 30, 8362];
      arrayModel.updateRow(2, newRow);
      expect(sortModel.getRowCount()).toBe(10);
    });

    it('Row update - to be sorted as tail', function(done) {
      let eventCount = 0;
      sortModel.addDataChangeListener(function(dataChangeType, payload) {
        eventCount++;
        if (eventCount == 1) {
          expect(dataChangeType).toBe(DataChangeType.MOVE);
          expect(payload.from).toBe(8);
          expect(payload.to).toBe(9);
        } else if (eventCount == 2) {
          expect(dataChangeType).toBe(DataChangeType.UPDATE);
          expect(payload.index).toBe(9);
          expect(sortModel.getValueAt(1, 9)).toBe('Alex');
          expect(sortModel.getValueAt(2, 9)).toBe(7234);
          expect(payload.original[1]).toBe('Josh');
          expect(payload.original[2]).toBe(3463);
          done();
        }
      });

      let newRow = [2341, 'Alex', 7234, 8362];
      arrayModel.updateRow(0, newRow);
      expect(sortModel.getRowCount()).toBe(10);
    });

    it('Row update - to be sorted as middle', function(done) {
      let eventCount = 0;
      sortModel.addDataChangeListener(function(dataChangeType, payload) {
        eventCount++;
        if (eventCount == 1) {
          expect(dataChangeType).toBe(DataChangeType.MOVE);
          expect(payload.from).toBe(8);
          expect(payload.to).toBe(1);
        } else if (eventCount == 2) {
          expect(dataChangeType).toBe(DataChangeType.UPDATE);
          expect(payload.index).toBe(1);
          expect(sortModel.getValueAt(1, 1)).toBe('Josh');
          expect(sortModel.getValueAt(2, 1)).toBe(1000);
          expect(payload.original[1]).toBe('Josh');
          expect(payload.original[2]).toBe(3463);
          done();
        }
      });

      let newRow = [2341, 'Josh', 1000, 8362];
      arrayModel.updateRow(0, newRow);
      expect(sortModel.getRowCount()).toBe(10);
    });

    it('Row update - no sorting change', function(done) {
      sortModel.addDataChangeListener(function(dataChangeType, payload) {
        expect(dataChangeType).toBe(DataChangeType.UPDATE);
        expect(payload.index).toBe(0);
        expect(sortModel.getValueAt(0, 0)).toBe(2341);
        expect(sortModel.getValueAt(1, 0)).toBe('Jennifer');
        expect(sortModel.getValueAt(2, 0)).toBe(34);
        expect(sortModel.getValueAt(3, 0)).toBe(8362);
        expect(payload.original[0]).toBe(3333);
        expect(payload.original[1]).toBe('Jennifer');
        expect(payload.original[2]).toBe(34);
        expect(payload.original[3]).toBe(3333);
        done();
      });

      let newRow = [2341, 'Jennifer', 34, 8362];
      arrayModel.updateRow(5, newRow);
      expect(sortModel.getRowCount()).toBe(10);
    });

    it('Row remove', function(done) {
      sortModel.addDataChangeListener(function(dataChangeType, payload) {
        expect(dataChangeType).toBe(DataChangeType.REMOVE);
        expect(payload.index).toBe(0);
        expect(payload.row[1]).toBe('Jennifer');
        expect(payload.row[2]).toBe(34);
        done();
      });

      arrayModel.removeRow(5);
      expect(sortModel.getRowCount()).toBe(9);
    });
  });
});

function createSourceModel() {
  let sourceModel = new ArrayModel(['Column 0', 'Column 1', 'Column 2', 'Column 3']);
  sourceModel.addRow([1111, 'Josh', 3463, 1111]);
  sourceModel.addRow([2222, 'April', 2351, 2222]);
  sourceModel.addRow([2222, 'April', 2351, 2222]);
  sourceModel.addRow([2222, 'April', 2351, 2222]);
  sourceModel.addRow([2222, 'April', 2351, 2222]);
  sourceModel.addRow([3333, 'Jennifer', 34, 3333]);
  sourceModel.addRow([2222, 'April', 2351, 2222]);
  sourceModel.addRow([4444, 'Tom', 2351, 4444]);
  sourceModel.addRow([2222, 'April', 2351, 2222]);
  sourceModel.addRow([5555, 'Katy', 7234, 5555]);
  return sourceModel;
}
