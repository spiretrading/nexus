import {css, StyleSheet} from 'aphrodite';
import * as Beam from 'beam';
import * as Nexus from 'nexus';
import * as React from 'react';
import {Center, CurrencySelectionBox, HBoxLayout, IntegerInputBox, 
  MoneyInputBox, Padding, VBoxLayout} from '../../..';

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
  buttonIsDisabled: boolean;
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
  onTransitionTimeUpdate?: (duration: number, field: string) => void;
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
          return css([RiskParametersView.CONTAINER_STYLE.small,
            RiskParametersView.CONTAINER_STYLE.base]);
        default:
          return css(RiskParametersView.CONTAINER_STYLE.base);
      }
    })();
    const containerWidth = (() => {
      switch(this.state.breakpoint) {
        case Breakpoint.SMALL:
          return '60%'
        case Breakpoint.MEDIUM:
          return '768px';
        case Breakpoint.LARGE:
          return '1036px';
        default:
          return '768px';
      }
    })();
    const containerPaddingWidth = (() => {
      switch(this.state.breakpoint) {
        case Breakpoint.SMALL:
          return '20%';
      }
    })();
    return (
        <VBoxLayout width='100%' height='100%'>
          <Padding size='30px'/>
          <HBoxLayout width='100%'>
            <Padding/>
            <VBoxLayout width={containerWidth} className={containerClassName}>
              <VBoxLayout width='100%' className={css(RiskParametersView.STYLE.controlsContainer)}>
                <HBoxLayout width='100%'>
                  <Padding/>
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
                    <Padding size='30px'/>
                    <Label text='Transition Time'/>
                    <Padding size='12px'/>
                    <TransistionTime/>
                  </VBoxLayout>
                  <Padding/>
                </HBoxLayout>
                <Padding size='30px'/>
                <div className={css(RiskParametersView.STYLE.divider)}/>
                <Padding size='30px'/>
                <Footer isAdmin={false} buttonIsDisabled={false}/>
              </VBoxLayout>
            </VBoxLayout>
            <Padding/>
          </HBoxLayout>
        </VBoxLayout>
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
  private static CONTAINER_STYLE = StyleSheet.create({
    small: {
      minWidth: '320px',
      maxWidth: '460px'
    },
    base: {
      position: 'relative' as 'relative'
    }
  });
  private static STYLE = StyleSheet.create({
    divider: {
      width: '100%',
      height: '1px',
      backgroundColor: '#E6E6E6'
    },
    controlsContainer: {
      position: 'absolute' as 'absolute'
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
  public render(): JSX.Element {
    return (
      <HBoxLayout width='100%'>
        <VBoxLayout>
          <IntegerInputBox min={0}/>
          <Padding size='10px'/>
          <span className={css(TransistionTime.STYLE.label)}>
            HOUR
          </span>
        </VBoxLayout>
        <Padding size='10px'/>
        <Center height='34px' className={css(TransistionTime.STYLE.colon)}>
          :
        </Center>
        <Padding size='10px'/>
        <VBoxLayout>
          <IntegerInputBox min={0} max={59}/>
          <Padding size='10px'/>
          <span className={css(TransistionTime.STYLE.label)}>
            MINUTE
          </span>
        </VBoxLayout>
        <Padding size='10px'/>
        <Center height='34px' className={css(TransistionTime.STYLE.colon)}>
          :
        </Center>
        <Padding size='10px'/>
        <VBoxLayout>
          <IntegerInputBox min={0} max={59}/>
          <Padding size='10px'/>
          <span className={css(TransistionTime.STYLE.label)}>
            SECOND
          </span>
        </VBoxLayout>
      </HBoxLayout>)
  }

  private static STYLE = StyleSheet.create({
    label: {
      font: '400 10px Roboto',
      color: '#8C8C8C',
      width: '100%',
      textAlign: 'center'
    },
    colon: {
      font: '400 16px Roboto'
    }
  });
}

class Footer extends React.Component<FooterProperties> {
  public render(): JSX.Element {
    const buttonText = (() => {
      if(this.props.isAdmin) {
        return 'Save Changes';
      }
      return 'Submit Request';
    })();
    const message = (() => {
      return <span className={css([Footer.MESSAGE_STYLE.base,
        Footer.MESSAGE_STYLE.validMessage])}>Saved</span>
    })();
    return (
      
        <HBoxLayout width='100%'>
          <Padding/>
            <VBoxLayout>
            <button disabled={this.props.buttonIsDisabled}
                className={css(Footer.STYLE.button)}>
              {buttonText}
            </button>
            <Padding size='18px'/>
            <HBoxLayout>
              <Padding/>
                {message}
              <Padding/>
            </HBoxLayout>
            </VBoxLayout>
          <Padding/>
      </HBoxLayout>)
  }

  private static STYLE = StyleSheet.create({
    button: {
      width: '246px',
      height: '34px',
      backgroundColor: '#684BC7',
      font: '400 14px Roboto',
      color: '#FFFFFF',
      border: 'none',
      outline: 0,
      ':active': {
        backgroundColor: '#4B23A0'
      },
      ':disabled': {
        backgroundColor: '#F8F8F8',
        color: '#8C8C8C'
      }
    }
  });
  private static MESSAGE_STYLE = StyleSheet.create({
    base: {
      font: '400 14px Roboto'
    },
    validMessage: {
      color: '#36BB55'
    }
  });
}
