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

    return (
      <div style={this.STYLE.containerStyle}>
        <img src={sourceShadow}
          width = {this.props.size}
          height = {this.props.size}
          style={this.STYLE.shadowStyle}/>
        <img src={source}
          width = {this.props.size}
          height = {this.props.size}
          className = {css(style)}
          onClick = {this.onClick}/>
        <img src={source}
          width = {this.props.size}
          height = {this.props.size}
          className = {css(style)}
          onClick = {this.onClick}/>
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
    '99%' : {
      transform: 'rotate(90deg)',
      opacity: '1'
    },
    '100%' : {
      opacity: '0'
    }
  };
  private readonly close =  {
    '0%' : {
      transform: 'rotate(0deg)',
      opacity: '1'
    },
    '99%' : {
      transform: 'rotate(-90deg)',
      opacity: '1'
    },
    '100%' : {
      opacity: '0'
    }
  };
  private readonly openAndFade =  {
    '0%' : {
      transform: 'rotate(0deg)',
      opacity: '1'
    },
    '99%' : {
      transform: 'rotate(90deg)',
      opacity: '1'
    },
    '100%' : {
      opacity: '0'
    }
  };
  private readonly closeAndFade =  {
    '0%' : {
      transform: 'rotate(0deg)',
      opacity: '1'
    },
    '99%' : {
      transform: 'rotate(-90deg)',
      opacity: '1'
    },
    '100%' : {
      opacity: '0'
    }
  };
  private readonly ANIMATION = StyleSheet.create({
    spinOpen: {
      position: 'absolute',
      background: '#ffffff',
      animationName: this.open,
      animationDuration: '5s',
      animationIterationCount: 1,
      animationFillMode: 'forwards'
    },
    spinClose: {
      position: 'absolute',
      background: '#ffffff',
      animationName: this.close,
      animationDuration: '5s',
      animationIterationCount: 1,
      animationFillMode: 'forwards'
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
