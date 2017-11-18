import ArrayModel from '../../../../webapp/utils/table-models/array-model';
import SumModel from '../../../../webapp/utils/table-models/sum-model';
import DataChangeType from '../../../../webapp/utils/table-models/model/data-change-type';
import { Money } from 'spire-client';

describe('SumModel', function() {
  beforeAll(function() {
    this.sumModel = new SumModel(createSourceModel());
  });

  it('getRowCount', function() {
    expect(this.sumModel.getRowCount()).toBe(1);
  });

  it('getColumnCount', function() {
    expect(this.sumModel.getColumnCount()).toBe(4);
  });

  it('getColumnName', function() {
    expect(this.sumModel.getColumnName(1)).toBe('Column 2');
  });

  describe('getValueAt', function() {
    it ('Invalid row index', function() {
      let testFunction = function() {
        this.sumModel.getValueAt(0, 100);
      }.bind(this);
      expect(testFunction).toThrowError(RangeError);
    });

    it ('Valid', function() {
      expect(this.sumModel.getValueAt(0, 0)).toBe(16655);
      let expectedMoney = Money.fromNumber(16660);
      expect(this.sumModel.getValueAt(1, 0).compare(expectedMoney)).toBe(0);
      expect(this.sumModel.getValueAt(2, 0)).toBe(16665);
      expectedMoney = Money.fromNumber(16670);
      expect(this.sumModel.getValueAt(3, 0).compare(expectedMoney)).toBe(0);
    });
  });

  describe('onDataChange', function() {
    let sourceModel, sumModel;

    beforeEach(function() {
      sourceModel = createSourceModel();
      sumModel = new SumModel(sourceModel);
    });

    it('Row add', function(done) {
      sumModel.addDataChangeListener(function(dataChangeType, payload) {
        expect(dataChangeType).toBe(DataChangeType.UPDATE);
        expect(payload.index).toBe(5);

        // check payload
        expect(payload.original[0]).toBe(16655);
        let expectedMoney = Money.fromNumber(16660);
        expect(payload.original[1].compare(expectedMoney)).toBe(0);
        expect(payload.original[2]).toBe(16665);
        expectedMoney = Money.fromNumber(16670);
        expect(payload.original[3].compare(expectedMoney)).toBe(0);

        // check current values
        expect(sumModel.getValueAt(0, 0)).toBe(23316);
        expectedMoney = Money.fromNumber(23322);
        expect(sumModel.getValueAt(1, 0).compare(expectedMoney)).toBe(0);
        expect(sumModel.getValueAt(2, 0)).toBe(23328);
        expectedMoney = Money.fromNumber(23334);
        expect(sumModel.getValueAt(3, 0).compare(expectedMoney)).toBe(0);

        done();
      });
      sourceModel.addRow([6661, Money.fromNumber(6662), 6663, Money.fromNumber(6664)]);
    });

    it('Row update', function(done) {
      sumModel.addDataChangeListener(function(dataChangeType, payload) {
        expect(dataChangeType).toBe(DataChangeType.UPDATE);
        expect(payload.index).toBe(0);

        // check payload
        expect(payload.original[0]).toBe(16655);
        let expectedMoney = Money.fromNumber(16660);
        expect(payload.original[1].compare(expectedMoney)).toBe(0);
        expect(payload.original[2]).toBe(16665);
        expectedMoney = Money.fromNumber(16670);
        expect(payload.original[3].compare(expectedMoney)).toBe(0);

        // check current values
        expect(sumModel.getValueAt(0, 0)).toBe(5553);
        expectedMoney = Money.fromNumber(21100);
        expect(sumModel.getValueAt(1, 0).compare(expectedMoney)).toBe(0);
        expect(sumModel.getValueAt(2, 0)).toBe(21105);
        expectedMoney = Money.fromNumber(5562);
        expect(sumModel.getValueAt(3, 0).compare(expectedMoney)).toBe(0);

        done();
      });
      sourceModel.updateRow(2, [-7771, Money.fromNumber(7772), 7773, Money.fromNumber(-7774)]);
    });

    it('Row remove', function(done) {
      sumModel.addDataChangeListener(function(dataChangeType, payload) {
        expect(dataChangeType).toBe(DataChangeType.UPDATE);
        expect(payload.index).toBe(0);

        // check payload
        expect(payload.original[0]).toBe(16655);
        let expectedMoney = Money.fromNumber(16660);
        expect(payload.original[1].compare(expectedMoney)).toBe(0);
        expect(payload.original[2]).toBe(16665);
        expectedMoney = Money.fromNumber(16670);
        expect(payload.original[3].compare(expectedMoney)).toBe(0);

        // check current values
        expect(sumModel.getValueAt(0, 0)).toBe(12214);
        expectedMoney = Money.fromNumber(12218);
        expect(sumModel.getValueAt(1, 0).compare(expectedMoney)).toBe(0);
        expect(sumModel.getValueAt(2, 0)).toBe(12222);
        expectedMoney = Money.fromNumber(12226);
        expect(sumModel.getValueAt(3, 0).compare(expectedMoney)).toBe(0);

        done();
      });
      sourceModel.removeRow(3);
    });
  });
});

function createSourceModel() {
  let sourceModel = new ArrayModel(['Column 1', 'Column 2', 'Column 3', 'Column 4']);
  sourceModel.addRow([1111, Money.fromNumber(1112), 1113, Money.fromNumber(1114)]);
  sourceModel.addRow([2221, Money.fromNumber(2222), 2223, Money.fromNumber(2224)]);
  sourceModel.addRow([3331, Money.fromNumber(3332), 3333, Money.fromNumber(3334)]);
  sourceModel.addRow([4441, Money.fromNumber(4442), 4443, Money.fromNumber(4444)]);
  sourceModel.addRow([5551, Money.fromNumber(5552), 5553, Money.fromNumber(5554)]);
  return sourceModel;
}
