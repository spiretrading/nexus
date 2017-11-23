export default class {
  constructor(menuItems) {
    this.menuItems = Object.freeze(clone(menuItems));
  }

  onMenuItemClick(item) {
    throw new TypeError('Method not implemented');
  }

  getMenuItems() {
    return this.menuItems;
  }
}
