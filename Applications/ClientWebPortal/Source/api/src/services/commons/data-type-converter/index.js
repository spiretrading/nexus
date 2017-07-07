import DataType from '../../../definitions/data-type';
import Money from '../../../definitions/money';
import Security from '../../../definitions/security';

/** Converts data to their respective classes */
class DataTypeConverter {
  constructor() {
    this.toData = this.toData.bind(this);
  }

  toData(data) {
    convertToData.call(this, data);

    function convertToData(object) {
      if (object.constructor == Array) {
        for (let i=0; i<object.length; i++) {
          convertToData.call(this, object[i]);
        }
      } else if (object.which == DataType.MONEY) {
        object.value = object.value.toData();
      } else if (object.which == DataType.SECURITY) {
        object.value = object.value.toData();
      } else if (object.which == DataType.LIST) {
        convertToData.call(this, object.value);
      } else if (typeof object == 'object' && object.which == null) {
        for (let property in object) {
          convertToData.call(this, object[property]);
        }
      }
    }
  }

  fromData(data) {
    convertFromData.call(this, data);

    function convertFromData(object) {
      if (object.constructor == Array) {
        for (let i=0; i<object.length; i++) {
          convertFromData.call(this, object[i]);
        }
      } else if (object.which == DataType.MONEY) {
        object.value = Money.fromRepresentation(object.value);
      } else if (object.which == DataType.SECURITY) {
        object.value = Security.fromData(object.value);
      } else if (object.which == DataType.LIST) {
        convertFromData.call(this, object.value);
      } else if (typeof object == 'object' && object.which == null) {
        for (let property in object) {
          convertFromData.call(this, object[property]);
        }
      }
    }
  }
}

export default new DataTypeConverter();
