import * as React from 'react';
import {css, StyleSheet} from 'aphrodite';
import { url } from 'inspector';

interface Properties {
 size?: number|string;
}

interface State {
  isExpanded: boolean;
}

export class DropDownButton extends React.Component<Properties, State> {
  constructor(properties: Properties) {
    super(properties);
    this.state = {
      isExpanded: false
    };
    this.onClick = this.onClick.bind(this);
  }

  public render(): JSX.Element {
    const baseStyle = this.ANIMATION.base;
    const source = (() => {
      if (this.state.isExpanded) {
        return (
          'resources/account_page/entitlements_page/icons/arrow-expand.svg'
          );
      } else {
        return (
          'resources/account_page/entitlements_page/icons/arrow-collapse.svg');
      }
    })();
    const sourceShadow = (() => {
      if (this.state.isExpanded) {
        return (
          'resources/account_page/entitlements_page/icons/arrow-collapse.svg'
          );
      } else {
        return (
          'resources/account_page/entitlements_page/icons/arrow-expand.svg'
          );
      }
    })();
    const style = (() => {
      if (this.state.isExpanded) {
        return this.ANIMATION.spinClose;
      } else {
        return  this.ANIMATION.spinOpen;
      }
    })();
    const overlayStyle = (() => {
      if (this.state.isExpanded) {
        return this.ANIMATION.spinCloseFadeIn;
      } else {
        return  this.ANIMATION.spinOpenFadeIn;
      }
    })();

    return (
      <div style={this.STYLE.containerStyle}>
        <img src={source}
          width = {this.props.size}
          height = {this.props.size}
          className = {css(baseStyle, style)}
          onClick={this.onClick}/>
        <img src={sourceShadow}
          width = {this.props.size}
          height = {this.props.size}
          className = {css(baseStyle, overlayStyle)}
          onClick={this.onClick}/>

      </div>);
  }

  private onClick() {
    this.setState({
      isExpanded: !this.state.isExpanded
    });
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
    base:{
      position: 'absolute',
      animationDuration: '300ms',
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
      position: 'relative' as 'relative'
    },
    shadowStyle: {
      position: 'absolute' as 'absolute'
    }
  };
}
