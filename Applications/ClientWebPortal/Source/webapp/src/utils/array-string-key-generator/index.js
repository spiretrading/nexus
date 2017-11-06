export default class {
  get(values) {
    for (let i=0; i<values.length; i++) {
      if (values[i] == null) {
        throw new TypeError('Values must not contain a null');
      }
    }

    let key = "";
    for (let i=0; i<values.length; i++) {
      key += '(';
      if (values[i].toString != null) {
        key += values[i].toString();
      } else {
        key += values[i];
      }
      key += ')';
    }
    return key;
  }
}
