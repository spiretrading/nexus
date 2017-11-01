export default class {
  constructor(value, display, color, backgroundColor, valueComparer) {
    this.value = value;
    this.display = display;
    this.color = color;
    this.backgroundColor = backgroundColor;
    this.valueComparer = valueComparer;
  }

  compare(a, b) {
    let aValue = a.value;
    let bValue = b.value;
    return this.valueComparer.compare(aValue, bValue);
  }
}
