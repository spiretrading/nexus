import * as React from 'react';
import { DisplaySize } from './display_size';

export interface DisplaySizeProperties {
  displaySize?: DisplaySize;
}

interface State {
  displaySize: DisplaySize;
}

export function displaySizeRenderer(
    WrappedComponent: React.ComponentType<DisplaySizeProperties>) {
  return class extends React.Component<{}, State> {
    constructor(props: {}) {
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
      return <WrappedComponent {...this.props}
        displaySize={this.state.displaySize}/>;
    }
  };
}
