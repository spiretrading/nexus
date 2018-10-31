import * as React from 'react';
import {css, StyleSheet} from 'aphrodite';

interface Properties {
 size?: number|string;
}

interface State {
  isExpanded: boolean;
  isFirstTime: boolean;
}

export class DropDownButton extends React.Component<Properties, State> {
  constructor(properties: Properties) {
    super(properties);
    this.state = {
      isExpanded: false,
      isFirstTime: true
    };
    this.onClick = this.onClick.bind(this);
  }

  public render(): JSX.Element {
    const baseStyle = this.ANIMATION.base;
    const endSource = (() => {
      if (this.state.isExpanded) {
        return (
          'resources/account_page/entitlements_page/icons/arrow-collapse.svg');
      } else {
        return (
          'resources/account_page/entitlements_page/icons/arrow-expand.svg');
      }
    })();
    const startSource = (() => {
      if (this.state.isExpanded) {
        return (
          'resources/account_page/entitlements_page/icons/arrow-expand.svg');
      } else {
        return (
          'resources/account_page/entitlements_page/icons/arrow-collapse.svg');
      }
    })();
    const startStyle = (() => {
      if(this.state.isFirstTime) {
        return this.ANIMATION.noAnimationHidden;
      } else {
        if (this.state.isExpanded) {
          return  this.ANIMATION.spinOpen;
        } else {
          return this.ANIMATION.spinClose;
        }
      }
    })();
    const endStyle = (() => {
      if(this.state.isFirstTime) {
        return this.ANIMATION.noAnimation;
      } else {
        if (this.state.isExpanded) {
          return  this.ANIMATION.spinOpenFadeIn;
        } else {
          return this.ANIMATION.spinCloseFadeIn;
        }
      }
    })();
    return (
      <div style={this.STYLE.containerStyle}>
        <img src={endSource}
          width = {this.props.size}
          height = {this.props.size}
          className = {css(baseStyle, startStyle)}
          onClick={this.onClick}/>
        <img src={startSource}
          width = {this.props.size}
          height = {this.props.size}
          className = {css(baseStyle, endStyle)}
          onClick={this.onClick}/>
      </div>);
  }

  private onClick() {
    this.setState({
      isExpanded: !this.state.isExpanded
    });
    if(this.state.isFirstTime) {
      this.setState({
      isFirstTime: false
      });
    }
  }

  private readonly open =  {
    '0%' : {
      transform: 'rotate(0deg)',
      opacity: '1'
    },
    '100%' : {
      transform: 'rotate(90deg)',
      opacity: '0'
    }
  };
  private readonly close =  {
    '0%' : {
      transform: 'rotate(0deg)',
      opacity: '1'
    },
    '100%' : {
      transform: 'rotate(-90deg)',
      opacity: '0'
    }
  };
  private readonly openAndFade =  {
    '0%' : {
      transform: 'rotate(-90deg)',
      opacity: '0'
    },
    '100%' : {
      transform: 'rotate(0deg)',
      opacity: '1'
    }
  };
  private readonly closeAndFade =  {
    '0%' : {
      transform: 'rotate(90deg)',
      opacity: '0'
    },
    '100%' : {
      transform: 'rotate(0deg)',
      opacity: '1'
    }
  };
  private readonly ANIMATION = StyleSheet.create({
    noAnimation: {
      position: 'absolute'
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
      animationName: this.open
    },
    spinClose: {
      animationName: this.close
    },
    spinOpenFadeIn: {
      animationName: this.openAndFade
    },
    spinCloseFadeIn:{
      animationName: this.closeAndFade
    }
  });

  public readonly STYLE= {
    containerStyle: {
      position: 'relative' as 'relative',
      cursor: 'pointer'
    }
  };
}
