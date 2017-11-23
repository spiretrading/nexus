import ArrayModel from '../../../../webapp/utils/table-models/array-model';
import FilterModel from '../../../../webapp/utils/table-models/filter-model';
import DataChangeType from '../../../../webapp/utils/table-models/model/data-change-type';

describe('FilterModel', function() {
  beforeAll(function() {
    this.filter = [
      {
        index: 2,
        value: 8888
      },
      {
        index: 1,
        value: 'Should Include'
      }
    ];
    this.filterModel = new FilterModel(createSourceModel(), this.filter);
  });

  it('getRowCount', function() {
    expect(this.filterModel.getRowCount()).toBe(2);
  });

  it('getColumnCount', function() {
    expect(this.filterModel.getColumnCount()).toBe(4);
  });

  it('getColumnName', function() {
    expect(this.filterModel.getColumnName(1)).toBe('Column 1');
  });

  it('getValueAt', function() {
    expect(this.filterModel.getValueAt(3, 1)).toBe('Row 4 Column 3');
  });

  describe('onDataChange', function() {
    let sourceModel, filterModel;

    beforeEach(function() {
      sourceModel = createSourceModel();
      filterModel = new FilterModel(sourceModel, this.filter);
    });

    it('Row add - should not be included', function() {
      filterModel.addDataChangeListener(function(dataChangeType, payload) {
        fail();
      });
      sourceModel.addRow([7770, 'Should Include', 7772, 'Row 6 Column 3']);
      expect(filterModel.getRowCount()).toBe(2);
    });

    it('Row add - should be included', function(done) {
      filterModel.addDataChangeListener(function(dataChangeType, payload) {
        expect(dataChangeType).toBe(DataChangeType.ADD);
        expect(payload).toBe(2);
        expect(filterModel.getRowCount()).toBe(3);
        expect(filterModel.getValueAt(0, 0)).toBe(2220);
        expect(filterModel.getValueAt(0, 1)).toBe(5550);
        expect(filterModel.getValueAt(0, 2)).toBe(7770);
        expect(filterModel.projectionToSourceRows[2]).toBe(6);
        expect(filterModel.sourceToProjectionRows[6]).toBe(2);
        done();
      });
      sourceModel.addRow([7770, 'Should Include', 8888, 'Row 6 Column 3']);
    });

    it('Row remove - was not in projection', function() {
      filterModel.addDataChangeListener(function(dataChangeType, payload) {
        fail();
      });
      sourceModel.removeRow(2);
      expect(filterModel.getRowCount()).toBe(2);
    });

    it('Row remove - was in projection', function(done) {
      filterModel.addDataChangeListener(function(dataChangeType, payload) {
        expect(dataChangeType).toBe(DataChangeType.REMOVE);
        expect(payload.index).toBe(0);
        expect(payload.row[0]).toBe(2220);
        expect(filterModel.getRowCount()).toBe(1);
        expect(filterModel.getValueAt(0,0)).toBe(5550);
        expect(filterModel.projectionToSourceRows[0]).toBe(3);
        expect(filterModel.sourceToProjectionRows[3]).toBe(0);
        done();
      });
      sourceModel.removeRow(1);
    });

    it('Row update - was not in projection, should now be included, update head index', function(done) {
      filterModel.addDataChangeListener(function(dataChangeType, payload) {
        expect(dataChangeType).toBe(DataChangeType.ADD);
        expect(payload).toBe(0);
        expect(filterModel.getRowCount()).toBe(3);
        expect(filterModel.getValueAt(0, 0)).toBe(1110);
        expect(filterModel.getValueAt(0, 1)).toBe(2220);
        expect(filterModel.getValueAt(0, 2)).toBe(5550);
        expect(filterModel.projectionToSourceRows[0]).toBe(0);
        expect(filterModel.projectionToSourceRows[1]).toBe(1);
        expect(filterModel.projectionToSourceRows[2]).toBe(4);
        expect(filterModel.sourceToProjectionRows[0]).toBe(0);
        expect(filterModel.sourceToProjectionRows[1]).toBe(1);
        expect(filterModel.sourceToProjectionRows[4]).toBe(2);
        done();
      });
      sourceModel.updateRow(0, [1110, 'Should Include', 8888, 'Row 0 Column 3']);
    });

    it('Row update - was not in projection, should now be included, update mid index', function(done) {
      filterModel.addDataChangeListener(function(dataChangeType, payload) {
        expect(dataChangeType).toBe(DataChangeType.ADD);
        expect(payload).toBe(1);
        expect(filterModel.getRowCount()).toBe(3);
        expect(filterModel.getValueAt(0, 0)).toBe(2220);
        expect(filterModel.getValueAt(0, 1)).toBe(4440);
        expect(filterModel.getValueAt(0, 2)).toBe(5550);
        expect(filterModel.projectionToSourceRows[0]).toBe(1);
        expect(filterModel.projectionToSourceRows[1]).toBe(3);
        expect(filterModel.projectionToSourceRows[2]).toBe(4);
        expect(filterModel.sourceToProjectionRows[0]).toBe(undefined);
        expect(filterModel.sourceToProjectionRows[1]).toBe(0);
        expect(filterModel.sourceToProjectionRows[2]).toBe(undefined);
        expect(filterModel.sourceToProjectionRows[3]).toBe(1);
        expect(filterModel.sourceToProjectionRows[4]).toBe(2);
        expect(filterModel.sourceToProjectionRows[5]).toBe(undefined);
        done();
      });
      sourceModel.updateRow(3, [4440, 'Should Include', 8888, 'Row 3 Column 3']);
    });

    it('Row update - was not in projection, should now be included, update end index', function(done) {
      filterModel.addDataChangeListener(function(dataChangeType, payload) {
        expect(dataChangeType).toBe(DataChangeType.ADD);
        expect(payload).toBe(2);
        expect(filterModel.getRowCount()).toBe(3);
        expect(filterModel.getValueAt(0, 0)).toBe(2220);
        expect(filterModel.getValueAt(0, 1)).toBe(5550);
        expect(filterModel.getValueAt(0, 2)).toBe(6660);
        expect(filterModel.projectionToSourceRows[0]).toBe(1);
        expect(filterModel.projectionToSourceRows[1]).toBe(4);
        expect(filterModel.projectionToSourceRows[2]).toBe(5);
        expect(filterModel.sourceToProjectionRows[0]).toBe(undefined);
        expect(filterModel.sourceToProjectionRows[1]).toBe(0);
        expect(filterModel.sourceToProjectionRows[2]).toBe(undefined);
        expect(filterModel.sourceToProjectionRows[3]).toBe(undefined);
        expect(filterModel.sourceToProjectionRows[4]).toBe(1);
        expect(filterModel.sourceToProjectionRows[5]).toBe(2);
        done();
      });
      sourceModel.updateRow(5, [6660, 'Should Include', 8888, 'Row 5 Column 3']);
    });

    it('Row update - was in projection, now should not be included, update head index', function(done) {
      filterModel.addDataChangeListener(function(dataChangeType, payload) {
        expect(dataChangeType).toBe(DataChangeType.REMOVE);
        expect(payload.index).toBe(0);
        expect(filterModel.getRowCount()).toBe(1);
        expect(filterModel.getValueAt(0, 0)).toBe(5550);
        expect(filterModel.projectionToSourceRows[0]).toBe(4);
        expect(filterModel.sourceToProjectionRows[0]).toBe(undefined);
        expect(filterModel.sourceToProjectionRows[1]).toBe(undefined);
        expect(filterModel.sourceToProjectionRows[2]).toBe(undefined);
        expect(filterModel.sourceToProjectionRows[3]).toBe(undefined);
        expect(filterModel.sourceToProjectionRows[4]).toBe(0);
        expect(filterModel.sourceToProjectionRows[5]).toBe(undefined);
        expect(filterModel.sourceToProjectionRows[6]).toBe(undefined);
        done();
      });
      sourceModel.updateRow(1, [2220, 'Row 1 Column 1', 8888, 'Row 5 Column 3']);
    });

    it('Row update - was in projection, now should not be included, update end index', function(done) {
      filterModel.addDataChangeListener(function(dataChangeType, payload) {
        expect(dataChangeType).toBe(DataChangeType.REMOVE);
        expect(payload.index).toBe(1);
        expect(filterModel.getRowCount()).toBe(1);
        expect(filterModel.getValueAt(0, 0)).toBe(2220);
        expect(filterModel.sourceToProjectionRows[0]).toBe(undefined);
        expect(filterModel.sourceToProjectionRows[1]).toBe(0);
        expect(filterModel.sourceToProjectionRows[2]).toBe(undefined);
        expect(filterModel.sourceToProjectionRows[3]).toBe(undefined);
        expect(filterModel.sourceToProjectionRows[4]).toBe(undefined);
        expect(filterModel.sourceToProjectionRows[5]).toBe(undefined);
        expect(filterModel.sourceToProjectionRows[6]).toBe(undefined);
        done();
      });
      sourceModel.updateRow(4, [5550, 'Row 4 Column 1', 8888, 'Row 5 Column 3']);
    });

    it('Row update - was in projection, still included, other data changes', function(done) {
      filterModel.addDataChangeListener(function(dataChangeType, payload) {
        expect(dataChangeType).toBe(DataChangeType.UPDATE);
        expect(payload.index).toBe(1);
        expect(filterModel.getRowCount()).toBe(2);
        expect(payload.original[0]).toBe(5550);
        expect(payload.original[1]).toBe('Should Include');
        expect(payload.original[2]).toBe(8888);
        expect(payload.original[3]).toBe('Row 4 Column 3');
        done();
      });
      sourceModel.updateRow(4, [5550, 'Should Include', 8888, 'This Changed']);
    });
  });
});

function createSourceModel() {
  let sourceModel = new ArrayModel(['Column 0', 'Column 1', 'Column 2', 'Column 3']);
  sourceModel.addRow([1110, 'Row 0 Column 1', 1112, 'Row 0 Column 3']);
  sourceModel.addRow([2220, 'Should Include', 8888, 'Row 1 Column 3']);
  sourceModel.addRow([3330, 'Should Include', 3332, 'Row 2 Column 3']);
  sourceModel.addRow([4440, 'Row 3 Column 1', 8888, 'Row 3 Column 3']);
  sourceModel.addRow([5550, 'Should Include', 8888, 'Row 4 Column 3']);
  sourceModel.addRow([6660, 'Row 5 Column 1', 6662, 'Row 5 Column 3']);
  return sourceModel;
}
