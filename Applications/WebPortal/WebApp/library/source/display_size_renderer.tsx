import * as React from 'react';
import { DisplaySize } from './display_size';

type Omit<T, K> = Pick<T, Exclude<keyof T, K>>;
type Subtract<T, K> = Omit<T, keyof K>;

/** The properties that the BaseComponent needs to have. */
interface DisplaySizeProperties {
  displaySize: DisplaySize;
}

interface State {
  displaySize: DisplaySize;
}

/** Returns a component that monitors the resize event.
 * @param BaseComponent - A component that has a DisplaySize that needs
 *  to be updated on a resize.
 */
export function displaySizeRenderer<T extends DisplaySizeProperties>(
    BaseComponent: React.ComponentType<T>) {
  return class extends React.Component<Subtract<T, DisplaySizeProperties>,
      State> {
    constructor(props: T) {
      super(props);
      this.state = {
        displaySize: this.getDisplaySize()
      };
      this.onScreenResize = this.onScreenResize.bind(this);
      this.getDisplaySize = this.getDisplaySize.bind(this);
    }

    public componentDidMount() {
      window.addEventListener('resize', this.onScreenResize);
    }

    public componentWillUnmount() {
      window.removeEventListener('resize', this.onScreenResize);
    }

    public onScreenResize() {
      const newDisplaySize = this.getDisplaySize();
      if (newDisplaySize !== this.state.displaySize) {
        this.setState({ displaySize: newDisplaySize });
      }
    }

    public getDisplaySize(): DisplaySize {
      const screenWidth = window.innerWidth ||
        document.documentElement.clientWidth ||
        document.getElementsByTagName('body')[0].clientWidth;
      if (screenWidth <= 767) {
        return DisplaySize.SMALL;
      } else if (screenWidth > 767 && screenWidth <= 1035) {
        return DisplaySize.MEDIUM;
      } else {
        return DisplaySize.LARGE;
      }
    }

    public render() {
      const C = BaseComponent as any;
      return <C {...this.props} displaySize={this.state.displaySize}/>;
    }
  };
}
