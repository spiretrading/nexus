/** Lists the types of displays used to determine a component's layout. */
export enum DisplaySize {

  /** Smartphone size, typically ranging in width up to 767px.  */
  SMALL,

  /** Tablet size, typically ranging in width up to 1023px.  */
  MEDIUM,

  /** Desktop size for widths greater than 1023px. */
  LARGE
}

export namespace DisplaySize {

  /** Returns the default display size to use for the current resolution. */
  export function getDisplaySize(): DisplaySize {
    const screenWidth = window.innerWidth ||
      document.documentElement.clientWidth ||
      document.getElementsByTagName('body')[0].clientWidth;
    if(screenWidth <= 767) {
      return DisplaySize.SMALL;
    } else if(screenWidth > 767 && screenWidth <= 1035) {
      return DisplaySize.MEDIUM;
    } else {
      return DisplaySize.LARGE;
    }
  }
}
