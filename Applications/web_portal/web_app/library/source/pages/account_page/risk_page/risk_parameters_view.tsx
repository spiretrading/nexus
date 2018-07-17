import {css, StyleSheet} from 'aphrodite';
import * as Beam from 'beam';
import * as Nexus from 'nexus';
import * as React from 'react';
import {Center, CurrencySelectionBox, HBoxLayout, MoneyInputBox, Padding,
  VBoxLayout} from '../../..';

interface Properties {

  /** The parameters to display. */
  parameters: Nexus.RiskParameters;

  /** Used to lookup currency names and symbols. */
  currencyDatabase: Nexus.CurrencyDatabase;

  /** Called on an update of the currency. */
  onCurrencyChange?: (currency: Nexus.Currency) => void;

  /** Called on an update of the buying power. */
  onBuyingPowerChange?: (value: Nexus.Money) => void;

  /** Called on an update of the net loss. */
  onNetLossChange?: (value: Nexus.Money) => void;

  /** Called on an update of the transition time. */
  onTransitionTimeChange?: (duration: number, field: string) => void;
}

interface FooterProperties {
  isAdmin: boolean;
}

interface MoneyInputBoxProperties {
  label: string;
  onChange: (value: number) => void;
}

interface State {
  breakpoint: Breakpoint;
}

enum Breakpoint {
  SMALL,
  MEDIUM,
  LARGE
}

interface LabelProperties {
  text: string;
}

interface TransistionTimeProperties {
  onTransitionTimeUpdate: (duration: number, field: string) => void;
}
/** Implements a React component to display a set of RiskParameters. */
export class RiskParametersView extends React.Component<Properties, State> {

  public constructor(props: Properties) {
    super(props);
    this.state = {
      breakpoint: RiskParametersView.getBreakpoint()
    };
    this.onScreenResize = this.onScreenResize.bind(this);
  }

  public componentDidMount() {
    window.addEventListener('resize', this.onScreenResize);
  }

  public componentWillUnmount(): void {
    window.removeEventListener('resize', this.onScreenResize);
  }

  public render(): JSX.Element {
    const containerClassName = (() => {
      switch(this.state.breakpoint) {
        case Breakpoint.SMALL:
          return css(RiskParametersView.CONTAINERS.small);
        case Breakpoint.MEDIUM:
          return css(RiskParametersView.CONTAINERS.medium);
        case Breakpoint.LARGE:
          return css(RiskParametersView.CONTAINERS.large);
        default:
          return css(RiskParametersView.CONTAINERS.medium);
      }
    })();
    return (
        <HBoxLayout width='100%' height='100%'>
          <Padding/>
          <VBoxLayout className={containerClassName}>
            <Padding size='30px'/>
            <VBoxLayout width='246px'>
            <Label text='Currency'/>
            <Padding size='12px'/>
            <CurrencySelectionBox currencyDatabase={
              this.props.currencyDatabase} onChange={
              this.props.onCurrencyChange}/>
            <Padding size='30px'/>
            <Label text='Buying Power ($)'/>
            <Padding size='12px'/>
            <MoneyInputBox
              onChange={this.props.onBuyingPowerChange}/>
            <Padding size='30px'/>
            <Label text='Net Loss ($)'/>
            <Padding size='12px'/>
            <MoneyInputBox
              onChange={this.props.onNetLossChange}/>
            </VBoxLayout>
          </VBoxLayout>
          <Padding/>
        </HBoxLayout>
      );
  }

  private static getBreakpoint(): Breakpoint {
    const screenWidth = window.innerWidth ||
      document.documentElement.clientWidth ||
      document.getElementsByTagName('body')[0].clientWidth;
    if(screenWidth <= 767) {
      return Breakpoint.SMALL;
    } else if(screenWidth > 767 && screenWidth <= 1035) {
      return Breakpoint.MEDIUM;
    } else {
      return Breakpoint.LARGE;
    }
  }

  private onScreenResize(): void {
    const newBreakpoint = RiskParametersView.getBreakpoint();
    if(newBreakpoint !== this.state.breakpoint) {
      this.setState({breakpoint: newBreakpoint});
    }
  }

  private static defaultProps = {
    onCurrencyChange: (currency: Nexus.Currency) => {},
    onBuyingPowerChange: (value: Nexus.Money) => {},
    onNetLossChange: (value: Nexus.Money) => {},
    onTransitionTimeChange: (duration: number, field: string) => {}
  }
  private static CONTAINERS = StyleSheet.create({
    small: {
      width: '60%',
      minWidth: '320px',
      maxWidth: '460px'
    },
    medium: {
      width: '768px'
    },
    large: {
      width: '1036px'
    }
  });
}
class Label extends React.Component<LabelProperties> {
  public render(): JSX.Element {
    return (
      <HBoxLayout width='100%'>
        <span className={css(Label.STYLE.text)}>
          {this.props.text}
        </span>
        <Padding/>
      </HBoxLayout>);
  }

  private static STYLE = StyleSheet.create({
    text: {
      font: '400 14px Roboto',
      color: '#333333',
      whiteSpace: 'nowrap'
    }
  });
}

class TransistionTime extends React.Component<TransistionTimeProperties> {

}

class Footer extends React.Component<FooterProperties> {

}
