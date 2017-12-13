import ArrayModel from '../../../../webapp/utils/table-models/array-model';
import SortModel from '../../../../webapp/utils/table-models/sort-model';
import DataChangeType from '../../../../webapp/utils/table-models/model/data-change-type';

describe('SortModel', function() {
  beforeAll(function() {
    this.sortColumns = [
      { index: 2, isAsc: true },
      { index: 1, isAsc: false }
    ];
    let arrayModel = createSourceModel();
    this.sortModel = new SortModel(arrayModel, this.sortColumns);
  });

  it('getRowCount', function() {
    expect(this.sortModel.getRowCount()).toBe(10);
  });

  it('getColumnCount', function() {
    expect(this.sortModel.getColumnCount()).toBe(4);
  });

  it('getColumnName', function() {
    expect(this.sortModel.getColumnName(1)).toBe('Column 1');
  });

  describe('constructor', function() {
    it('constructor - with sort orders', function() {
      let arrayModel = createSourceModel();
      let sortModel = new SortModel(arrayModel, this.sortColumns);

      expect(sortModel.getValueAt(1, 0).value).toBe('Jennifer');
      expect(sortModel.getValueAt(1, 1).value).toBe('Tom');
      expect(sortModel.getValueAt(1, 2).value).toBe('April');
      expect(sortModel.getValueAt(1, 3).value).toBe('April');
      expect(sortModel.getValueAt(1, 4).value).toBe('April');
      expect(sortModel.getValueAt(1, 5).value).toBe('April');
      expect(sortModel.getValueAt(1, 6).value).toBe('April');
      expect(sortModel.getValueAt(1, 7).value).toBe('April');
      expect(sortModel.getValueAt(1, 8).value).toBe('Josh');
      expect(sortModel.getValueAt(1, 9).value).toBe('Katy');
    });

    it('constructor - without sort orders', function() {
      let arrayModel = createSourceModel();
      let sortModel = new SortModel(arrayModel);

      expect(sortModel.getValueAt(1, 0).value).toBe('Josh');
      expect(sortModel.getValueAt(1, 1).value).toBe('April');
      expect(sortModel.getValueAt(1, 2).value).toBe('April');
      expect(sortModel.getValueAt(1, 3).value).toBe('April');
      expect(sortModel.getValueAt(1, 4).value).toBe('April');
      expect(sortModel.getValueAt(1, 5).value).toBe('April');
      expect(sortModel.getValueAt(1, 6).value).toBe('April');
      expect(sortModel.getValueAt(1, 7).value).toBe('Jennifer');
      expect(sortModel.getValueAt(1, 8).value).toBe('Tom');
      expect(sortModel.getValueAt(1, 9).value).toBe('Katy');
    });
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
        expect(sortModel.getValueAt(1, 8).value).toBe('Alex');
        expect(sortModel.getValueAt(2, 8).value).toBe(2351);
        done();
      });

      let newRow = [{value:2341}, {value:'Alex'}, {value:2351}, {value:8362}];
      arrayModel.addRow(newRow);
      expect(sortModel.getRowCount()).toBe(11);
    });

    it('Row add - to be sorted as head', function(done) {
      sortModel.addDataChangeListener(function(dataChangeType, payload) {
        expect(dataChangeType).toBe(DataChangeType.ADD);
        expect(payload).toBe(0);
        expect(sortModel.getValueAt(1, 0).value).toBe('Alex');
        expect(sortModel.getValueAt(2, 0).value).toBe(30);
        done();
      });

      let newRow = [{value:2341}, {value:'Alex'}, {value:30}, {value:8362}];
      arrayModel.addRow(newRow);
      expect(sortModel.getRowCount()).toBe(11);
    });

    it('Row add - to be sorted as tail', function(done) {
      sortModel.addDataChangeListener(function(dataChangeType, payload) {
        expect(dataChangeType).toBe(DataChangeType.ADD);
        expect(payload).toBe(10);
        expect(sortModel.getValueAt(1, 10).value).toBe('Alex');
        expect(sortModel.getValueAt(2, 10).value).toBe(7234);
        done();
      });

      let newRow = [{value:2341}, {value:'Alex'}, {value:7234}, {value:8362}];
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
          expect(sortModel.getValueAt(1, 0).value).toBe('Alex');
          expect(sortModel.getValueAt(2, 0).value).toBe(30);
          expect(payload.original[1].value).toBe('April');
          expect(payload.original[2].value).toBe(2351);
          done();
        }
      });

      let newRow = [{value:2341}, {value:'Alex'}, {value:30}, {value:8362}];
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
          expect(sortModel.getValueAt(1, 9).value).toBe('Alex');
          expect(sortModel.getValueAt(2, 9).value).toBe(7234);
          expect(payload.original[1].value).toBe('Josh');
          expect(payload.original[2].value).toBe(3463);
          done();
        }
      });

      let newRow = [{value:2341}, {value:'Alex'}, {value:7234}, {value:8362}];
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
          expect(sortModel.getValueAt(1, 1).value).toBe('Josh');
          expect(sortModel.getValueAt(2, 1).value).toBe(1000);
          expect(payload.original[1].value).toBe('Josh');
          expect(payload.original[2].value).toBe(3463);
          done();
        }
      });

      let newRow = [{value:2341}, {value:'Josh'}, {value:1000}, {value:8362}];
      arrayModel.updateRow(0, newRow);
      expect(sortModel.getRowCount()).toBe(10);
    });

    it('Row update - no sorting change', function(done) {
      sortModel.addDataChangeListener(function(dataChangeType, payload) {
        expect(dataChangeType).toBe(DataChangeType.UPDATE);
        expect(payload.index).toBe(0);
        expect(sortModel.getValueAt(0, 0).value).toBe(2341);
        expect(sortModel.getValueAt(1, 0).value).toBe('Jennifer');
        expect(sortModel.getValueAt(2, 0).value).toBe(34);
        expect(sortModel.getValueAt(3, 0).value).toBe(8362);
        expect(payload.original[0].value).toBe(3333);
        expect(payload.original[1].value).toBe('Jennifer');
        expect(payload.original[2].value).toBe(34);
        expect(payload.original[3].value).toBe(3333);
        done();
      });

      let newRow = [{value:2341}, {value:'Jennifer'}, {value:34}, {value:8362}];
      arrayModel.updateRow(5, newRow);
      expect(sortModel.getRowCount()).toBe(10);
    });

    it('Row remove', function(done) {
      sortModel.addDataChangeListener(function(dataChangeType, payload) {
        expect(dataChangeType).toBe(DataChangeType.REMOVE);
        expect(payload.index).toBe(0);
        expect(payload.row[1].value).toBe('Jennifer');
        expect(payload.row[2].value).toBe(34);
        done();
      });

      arrayModel.removeRow(5);
      expect(sortModel.getRowCount()).toBe(9);
    });
  });
});

function createSourceModel() {
  let sourceModel = new ArrayModel(['Column 0', 'Column 1', 'Column 2', 'Column 3']);
  sourceModel.addRow([{value:1111}, {value:'Josh'}, {value:3463}, {value:1111}]);
  sourceModel.addRow([{value:2222}, {value:'April'}, {value:2351}, {value:2222}]);
  sourceModel.addRow([{value:2222}, {value:'April'}, {value:2351}, {value:2222}]);
  sourceModel.addRow([{value:2222}, {value:'April'}, {value:2351}, {value:2222}]);
  sourceModel.addRow([{value:2222}, {value:'April'}, {value:2351}, {value:2222}]);
  sourceModel.addRow([{value:3333}, {value:'Jennifer'}, {value:34}, {value:3333}]);
  sourceModel.addRow([{value:2222}, {value:'April'}, {value:2351}, {value:2222}]);
  sourceModel.addRow([{value:4444}, {value:'Tom'}, {value:2351}, {value:4444}]);
  sourceModel.addRow([{value:2222}, {value:'April'}, {value:2351}, {value:2222}]);
  sourceModel.addRow([{value:5555}, {value:'Katy'}, {value:7234}, {value:5555}]);
  return sourceModel;
}
