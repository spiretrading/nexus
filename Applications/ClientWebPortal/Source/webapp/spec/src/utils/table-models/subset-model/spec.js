import ArrayModel from '../../../../webapp/utils/table-models/array-model';
import SubsetModel from '../../../../webapp/utils/table-models/subset-model';
import DataChangeType from '../../../../webapp/utils/table-models/model/data-change-type';

describe('SubsetModel', function() {
  beforeAll(function() {
    this.subsetModel = new SubsetModel(createSourceModel(), [0, 2])
  });

  it('getRowCount', function() {
    expect(this.subsetModel.getRowCount()).toBe(5);
  });

  it('getColumnCount', function() {
    expect(this.subsetModel.getColumnCount()).toBe(2);
  });

  it('getColumnName', function() {
    expect(this.subsetModel.getColumnName(1)).toBe('String 3');
  });

  it('getValueAt', function() {
    expect(this.subsetModel.getValueAt(1, 2)).toBe('Row 2 Column 3');
  });

  describe('onDataChange', function() {
    let sourceModel, subsetModel;

    beforeEach(function() {
      sourceModel = createSourceModel();
      subsetModel = new SubsetModel(sourceModel, [0, 3]);
    });

    it('Row remove', function(done) {
      subsetModel.addDataChangeListener(function(dataChangeType, payload) {
        expect(dataChangeType).toBe(DataChangeType.REMOVE);
        expect(payload.index).toBe(1);
        expect(payload.row.length).toBe(2);
        expect(payload.row[0]).toBe('Row 1 Column 1');
        expect(payload.row[1]).toBe(1112);
        done();
      });
      sourceModel.removeRow(1);
    });

    it('Row update', function(done) {
      subsetModel.addDataChangeListener(function(dataChangeType, payload) {
        expect(dataChangeType).toBe(DataChangeType.UPDATE);
        expect(payload.index).toBe(2);
        expect(payload.original.length).toBe(2);
        expect(payload.original[0]).toBe('Row 2 Column 1');
        expect(payload.original[1]).toBe(1212);
        done();
      });
      sourceModel.updateRow(2, [2222, 'Test 2', 2323, 'Test 3']);
    });
  });
});

function createSourceModel() {
  let sourceModel = new ArrayModel(['Number 0', 'String 1', 'Number 2', 'String 3']);
  sourceModel.addRow([1010, 'Row 0 Column 1', 1012, 'Row 0 Column 3']);
  sourceModel.addRow([1110, 'Row 1 Column 1', 1112, 'Row 1 Column 3']);
  sourceModel.addRow([1210, 'Row 2 Column 1', 1212, 'Row 2 Column 3']);
  sourceModel.addRow([1310, 'Row 3 Column 1', 1312, 'Row 3 Column 3']);
  sourceModel.addRow([1410, 'Row 4 Column 1', 1412, 'Row 4 Column 3']);
  return sourceModel;
}
