import * as React from 'react';
import { css, StyleSheet } from 'aphrodite/no-important';

interface Properties {

  /* Determines the size of the element. */
  size?: number | string;

  /** The onClick event handler. */
  onClick?: (event?: React.MouseEvent<any>) => void;

  /** Determines if the button is collapsed or expanded. */
  isExpanded: boolean;
}

interface State {
  isFirstTime: boolean;
}

export class DropDownButton extends React.Component<Properties, State> {
  public static readonly defaultProps = {
    onClick: () => {},
    size: '16px'
  }

  constructor(properties: Properties) {
    super(properties);
    this.state = {
      isFirstTime: true
    };
  }

  public render(): JSX.Element {
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
        return DropDownButton.ANIMATION.noAnimation;
      } else if(this.props.isExpanded) {
        return DropDownButton.ANIMATION.spinOpenFadeIn;
      } else {
        return DropDownButton.ANIMATION.spinCloseFadeIn;
      }
    })();
    const startStyle = (() => {
      if(this.state.isFirstTime) {
        return DropDownButton.ANIMATION.noAnimationHidden;
      } else if(this.props.isExpanded) {
        return DropDownButton.ANIMATION.spinOpen;
      } else {
        return DropDownButton.ANIMATION.spinClose;
      }
    })();
    return (
      <div style={DropDownButton.STYLE.componentWrapper}
          onClick={this.props.onClick}>
        <div style={DropDownButton.STYLE.imageWrapper}>
          <img src={endSource}
            width={this.props.size}
            height={this.props.size}
            className={css(DropDownButton.ANIMATION.base, startStyle)}/>
          <img src={startSource}
            width={this.props.size}
            height={this.props.size}
            className={css(DropDownButton.ANIMATION.base, endStyle)}/>
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
  private static readonly CLOSE_AND_FADE_IN = {
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
      animationName: DropDownButton.OPEN_AND_FADEOUT
    },
    spinClose: {
      animationName: DropDownButton.CLOSE_AND_FADEOUT,
      animationDuration: '200ms'
    },
    spinOpenFadeIn: {
      position: 'static',
      animationName: DropDownButton.OPEN_AND_FADEIN
    },
    spinCloseFadeIn:{
      position: 'static',
      animationName: DropDownButton.CLOSE_AND_FADE_IN,
      animationDuration: '200ms'
    }
  });
  public static readonly STYLE = {
    imageWrapper: {
      position: 'relative',
      display: 'flex',
      justifyContent: 'center',
      alignItems: 'center',
      width: '20px',
      height: '20px',
      cursor: 'pointer'
    } as React.CSSProperties,
    componentWrapper: {
      width: '20px',
      height: '20px'
    } as React.CSSProperties
  };
}
