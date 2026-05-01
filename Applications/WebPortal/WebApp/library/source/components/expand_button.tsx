import * as React from 'react';
import { css, StyleSheet } from 'aphrodite/no-important';

interface Properties {

  /** Determines the size of the element. */
  size?: number | string;

  /** Disables the button and sets cursor to default. */
  disabled?: boolean;

  /** The onClick event handler. */
  onClick?: (event?: React.MouseEvent<HTMLDivElement>) => void;

  /** Determines if the button is collapsed or expanded. */
  isExpanded: boolean;
}

interface State {
  isFirstTime: boolean;
}

/** An animated button that toggles between a collapsed and expanded state. */
export class ExpandButton extends React.Component<Properties, State> {
  constructor(properties: Properties) {
    super(properties);
    this.state = {
      isFirstTime: true
    };
  }

  public render(): JSX.Element {
    const size = this.props.size ?? '16px';
    const endSource = (() => {
      if(this.props.isExpanded) {
        return 'resources/arrow-collapse.svg';
      } else {
        return 'resources/arrow-expand.svg';
      }
    })();
    const startSource = (() => {
      if(this.props.isExpanded) {
        return 'resources/arrow-expand.svg';
      } else {
        return 'resources/arrow-collapse.svg';
      }
    })();
    const endStyle = (() => {
      if(this.state.isFirstTime) {
        return ExpandButton.ANIMATION.noAnimation;
      } else if(this.props.isExpanded) {
        return ExpandButton.ANIMATION.spinOpenFadeIn;
      } else {
        return ExpandButton.ANIMATION.spinCloseFadeIn;
      }
    })();
    const startStyle = (() => {
      if(this.state.isFirstTime) {
        return ExpandButton.ANIMATION.noAnimationHidden;
      } else if(this.props.isExpanded) {
        return ExpandButton.ANIMATION.spinOpen;
      } else {
        return ExpandButton.ANIMATION.spinClose;
      }
    })();
    const imageWrapperStyle = this.props.disabled ?
      {...ExpandButton.STYLE.imageWrapper, cursor: 'default' as const} :
      ExpandButton.STYLE.imageWrapper;
    return (
      <div style={ExpandButton.STYLE.componentWrapper}
          onClick={this.props.disabled ? undefined : this.props.onClick}>
        <div style={imageWrapperStyle}>
          <img src={endSource}
            width={size}
            height={size}
            className={css(ExpandButton.ANIMATION.base, startStyle)}/>
          <img src={startSource}
            width={size}
            height={size}
            className={css(ExpandButton.ANIMATION.base, endStyle)}/>
        </div>
      </div>);
  }

  public componentDidUpdate(): void {
    if(this.state.isFirstTime && this.props.isExpanded) {
      this.setState({isFirstTime: false});
    }
  }

  private static readonly OPEN_AND_FADEOUT = {
    '0%': {
      transform: 'rotate(0deg)',
      opacity: '1'
    },
    '100%': {
      transform: 'rotate(90deg)',
      opacity: '0'
    }
  };
  private static readonly CLOSE_AND_FADEOUT = {
    '0%': {
      transform: 'rotate(0deg)',
      opacity: '1'
    },
    '100%': {
      transform: 'rotate(-90deg)',
      opacity: '0'
    }
  };
  private static readonly OPEN_AND_FADEIN = {
    '0%': {
      transform: 'rotate(-90deg)',
      opacity: '0'
    },
    '100%': {
      transform: 'rotate(0deg)',
      opacity: '1'
    }
  };
  private static readonly CLOSE_AND_FADEIN = {
    '0%' : {
      transform: 'rotate(90deg)',
      opacity: '0'
    },
    '100%' : {
      transform: 'rotate(0deg)',
      opacity: '1'
    }
  };
  private static readonly ANIMATION = StyleSheet.create({
    noAnimation: {
      position: 'static'
    },
    noAnimationHidden: {
      position: 'absolute',
      visibility: 'hidden'
    },
    base:{
      position: 'absolute',
      animationDuration: '200ms',
      animationIterationCount: 1,
      animationFillMode: 'forwards'
    },
    spinOpen: {
      animationName: ExpandButton.OPEN_AND_FADEOUT
    },
    spinClose: {
      animationName: ExpandButton.CLOSE_AND_FADEOUT,
      animationDuration: '200ms'
    },
    spinOpenFadeIn: {
      position: 'static',
      animationName: ExpandButton.OPEN_AND_FADEIN
    },
    spinCloseFadeIn:{
      position: 'static',
      animationName: ExpandButton.CLOSE_AND_FADEIN,
      animationDuration: '200ms'
    }
  });
  private static readonly STYLE: Record<string, React.CSSProperties> = {
    imageWrapper: {
      position: 'relative',
      display: 'flex',
      justifyContent: 'center',
      alignItems: 'center',
      width: '20px',
      height: '20px',
      cursor: 'pointer'
    },
    componentWrapper: {
      width: '20px',
      height: '20px'
    }
  };
}
