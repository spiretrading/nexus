import TranslationModel from '../../../../webapp/utils/table-models/translation-model';
import ArrayModel from '../../../../webapp/utils/table-models/array-model';
import DataChangeType from '../../../../webapp/utils/table-models/model/data-change-type';

describe('TranslationModel', function() {
  beforeAll(function() {
    this.translations = {
      row: [2, 0, 4, 1, 3],
      column: [3, 0, 2, 1]
    };
  });

  describe('constructor', function() {
    it('No initial translations', function() {
      let translationModel = initialize(null, null);
      expect(translationModel.getValueAt(2, 2)).toBe(1212);
      expect(translationModel.getValueAt(3, 4)).toBe('Row 4 Column 3');
    });

    it('Supplied initial translations', function() {
      let translationModel = initialize(this.translations.row, this.translations.column);
      expect(translationModel.getValueAt(2, 2)).toBe(1412);
      expect(translationModel.getValueAt(3, 4)).toBe('Row 3 Column 1');
    })
  });

  it('getColumnName', function() {
    let translationModel = initialize(this.translations.row, this.translations.column);
    expect(translationModel.getColumnName(0)).toBe('String 3');
    expect(translationModel.getColumnName(1)).toBe('Number 0');
    expect(translationModel.getColumnName(2)).toBe('Number 2');
    expect(translationModel.getColumnName(3)).toBe('String 1');
  });

  it('getValueAt', function() {
    let translationModel = initialize(this.translations.row, this.translations.column);
    expect(translationModel.getValueAt(0,0)).toBe('Row 2 Column 3');
    expect(translationModel.getValueAt(1,1)).toBe(1010);
    expect(translationModel.getValueAt(2,2)).toBe(1412);
    expect(translationModel.getValueAt(3,4)).toBe('Row 3 Column 1');
  });

  describe('moveRow', function() {
    let translationModel;

    beforeEach(function() {
      translationModel = initialize(this.translations.row, this.translations.column);
    });

    it('From lower to higher', function() {
      translationModel.moveRow(1, 3);
      expect(translationModel.getValueAt(0,0)).toBe('Row 2 Column 3');
      expect(translationModel.getValueAt(1,1)).toBe(1410);
      expect(translationModel.getValueAt(2,2)).toBe(1112);
      expect(translationModel.getValueAt(3,3)).toBe('Row 0 Column 1');
      expect(translationModel.getValueAt(3,4)).toBe('Row 3 Column 1');
    });

    it('From higher to lower', function() {
      translationModel.moveRow(3, 1);
      expect(translationModel.getValueAt(0,0)).toBe('Row 2 Column 3');
      expect(translationModel.getValueAt(1,1)).toBe(1110);
      expect(translationModel.getValueAt(2,2)).toBe(1012);
      expect(translationModel.getValueAt(3,3)).toBe('Row 4 Column 1');
      expect(translationModel.getValueAt(3,4)).toBe('Row 3 Column 1');
    })
  });

  describe('moveColumn', function() {
    let translationModel;

    beforeEach(function() {
      translationModel = initialize(this.translations.row, this.translations.column);
    });

    it('From lower to higher', function() {
      translationModel.moveColumn(1, 3);
      expect(translationModel.getValueAt(0,0)).toBe('Row 2 Column 3');
      expect(translationModel.getValueAt(1,1)).toBe(1012);
      expect(translationModel.getValueAt(2,2)).toBe('Row 4 Column 1');
      expect(translationModel.getValueAt(3,3)).toBe(1110);
      expect(translationModel.getValueAt(3,4)).toBe(1310);
    });

    it('From higher to lower', function() {
      translationModel.moveColumn(3, 1);
      expect(translationModel.getValueAt(0,0)).toBe('Row 2 Column 3');
      expect(translationModel.getValueAt(1,1)).toBe('Row 0 Column 1');
      expect(translationModel.getValueAt(2,2)).toBe(1410);
      expect(translationModel.getValueAt(3,3)).toBe(1112);
      expect(translationModel.getValueAt(3,4)).toBe(1312);
    })
  });

  describe('onDataChange', function() {
    let sourceModel, translationModel;

    beforeEach(function() {
      sourceModel = createSourceModel();
      translationModel = initialize(this.translations.row, this.translations.column, sourceModel);
    });

    it('ADD', function(done) {
      translationModel.addDataChangeListener(function(dataChangeType, payload) {
        expect(dataChangeType).toBe(DataChangeType.ADD);
        expect(payload).toBe(5);

        // checking new row has been added as the last row
        expect(translationModel.getValueAt(0,5)).toBe('Row 5 Column 3');
        expect(translationModel.getValueAt(1,5)).toBe(1510);
        expect(translationModel.getValueAt(2,5)).toBe(1512);
        expect(translationModel.getValueAt(3,5)).toBe('Row 5 Column 1');

        // check new translation mappings
        expect(translationModel.srcToTransRow[0]).toBe(1);
        expect(translationModel.srcToTransRow[1]).toBe(3);
        expect(translationModel.srcToTransRow[2]).toBe(0);
        expect(translationModel.srcToTransRow[3]).toBe(4);
        expect(translationModel.srcToTransRow[4]).toBe(2);
        expect(translationModel.srcToTransRow[5]).toBe(5);

        expect(translationModel.transToSrcRow[0]).toBe(2);
        expect(translationModel.transToSrcRow[1]).toBe(0);
        expect(translationModel.transToSrcRow[2]).toBe(4);
        expect(translationModel.transToSrcRow[3]).toBe(1);
        expect(translationModel.transToSrcRow[4]).toBe(3);
        expect(translationModel.transToSrcRow[5]).toBe(5);

        done();
      });
      sourceModel.addRow([1510, 'Row 5 Column 1', 1512, 'Row 5 Column 3']);
    });

    it('REMOVE', function(done) {
      translationModel.addDataChangeListener(function(dataChangeType, payload) {
        expect(dataChangeType).toBe(DataChangeType.REMOVE);
        expect(payload.index).toBe(0);
        expect(payload.row[0]).toBe('Row 2 Column 3');
        expect(payload.row[1]).toBe(1210);
        expect(payload.row[2]).toBe(1212);
        expect(payload.row[3]).toBe('Row 2 Column 1');

        // check new translation mappings
        expect(translationModel.srcToTransRow[0]).toBe(0);
        expect(translationModel.srcToTransRow[1]).toBe(2);
        expect(translationModel.srcToTransRow[2]).toBe(3);
        expect(translationModel.srcToTransRow[3]).toBe(1);

        expect(translationModel.transToSrcRow[0]).toBe(0);
        expect(translationModel.transToSrcRow[1]).toBe(3);
        expect(translationModel.transToSrcRow[2]).toBe(1);
        expect(translationModel.transToSrcRow[3]).toBe(2);

        done();
      });
      sourceModel.removeRow(2);
    })

    it('UPDATE', function(done) {
      translationModel.addDataChangeListener(function(dataChangeType, payload) {
        expect(dataChangeType).toBe(DataChangeType.UPDATE);
        expect(payload.index).toBe(4);
        expect(payload.original[0]).toBe('Row 3 Column 3');
        expect(payload.original[1]).toBe(1310);
        expect(payload.original[2]).toBe(1312);
        expect(payload.original[3]).toBe('Row 3 Column 1');

        done();
      });
      sourceModel.updateRow(3, [8888, 'Test String 1', 9999, 'Test String 2']);
    })
  });
});

function initialize(rowTranslation, columnTranslation, sourceModel = null) {
  if (sourceModel == null) {
    sourceModel = createSourceModel();
  }
  return new TranslationModel(sourceModel, rowTranslation, columnTranslation);
}

function createSourceModel() {
  let sourceModel = new ArrayModel(['Number 0', 'String 1', 'Number 2', 'String 3']);
  sourceModel.addRow([1010, 'Row 0 Column 1', 1012, 'Row 0 Column 3']);
  sourceModel.addRow([1110, 'Row 1 Column 1', 1112, 'Row 1 Column 3']);
  sourceModel.addRow([1210, 'Row 2 Column 1', 1212, 'Row 2 Column 3']);
  sourceModel.addRow([1310, 'Row 3 Column 1', 1312, 'Row 3 Column 3']);
  sourceModel.addRow([1410, 'Row 4 Column 1', 1412, 'Row 4 Column 3']);
  return sourceModel;
}
