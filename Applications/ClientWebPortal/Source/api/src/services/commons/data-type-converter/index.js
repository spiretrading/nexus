import DataType from '../../../definitions/data-type';
import Money from '../../../definitions/money';

/** Converts data to their respective classes */
class DataTypeConverter {
  toData(data) {
    convertToData.apply(this, [data]);

    function convertToData(object) {
      if (object.constructor == Array) {
        for (let i=0; i<object.length; i++) {
          convertToData.apply(this, [object[i]]);
        }
      } else if (object.which == DataType.MONEY) {
        object.value = object.value.toData();
      }

      else if (typeof object == 'object' && object.which == null) {
        for (let property in object) {
          convertToData.apply(this, [object[property]]);
        }
      }
    }
  }

  fromData(data) {
    convertFromData.apply(this, [data]);

    function convertFromData(object) {
      if (object.constructor == Array) {
        for (let i=0; i<object.length; i++) {
          convertFromData.apply(this, [object[i]]);
        }
      } else if (object.which == DataType.MONEY) {
        object.value = Money.fromRepresentation(object.value);
      }

      else if (typeof object == 'object' && object.which == null) {
        for (let property in object) {
          convertFromData.apply(this, [object[property]]);
        }
      }
    }
  }
}

export default new DataTypeConverter();
