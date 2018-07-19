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
}

interface State {
  breakpoint: Breakpoint;
}

enum Breakpoint {
  SMALL,
  MEDIUM,
  LARGE
}

enum TimeUnit {
  SECONDS,
  MINUTES,
  HOURS
}

/** Implements a React component to display a set of RiskParameters. */
export class RiskParametersView extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {
      breakpoint: RiskParametersView.getBreakpoint()
    };
    this.onScreenResize = this.onScreenResize.bind(this);
    this.onCurrencyChange = this.onCurrencyChange.bind(this);
    this.onBuyingPowerChange = this.onBuyingPowerChange.bind(this);
    this.onNetLossChange = this.onNetLossChange.bind(this);
    this.onTransitionTimeChange = this.onTransitionTimeChange.bind(this);
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
        case Breakpoint.MEDIUM:
          return css([RiskParametersView.CONTAINER_STYLE.medium,
            RiskParametersView.CONTAINER_STYLE.base]);
        case Breakpoint.LARGE:
          return css([RiskParametersView.CONTAINER_STYLE.large,
            RiskParametersView.CONTAINER_STYLE.base]);
        default:
          return css([RiskParametersView.CONTAINER_STYLE.medium,
              RiskParametersView.CONTAINER_STYLE.base]);
      }
    })();
    const hours = this.props.parameters.transitionTime.split().hours;
    const minutes = this.props.parameters.transitionTime.split().minutes;
    const seconds = this.props.parameters.transitionTime.split().seconds;
    const currencySign = this.props.currencyDatabase.fromCurrency(
      this.props.parameters.currency).sign;
    return (
      <HBoxLayout width='100%'>
        <Padding/>
        <VBoxLayout className={containerClassName}>
          <VBoxLayout width='100%' className={
              css(RiskParametersView.STYLE.innerContainer)}>
            <HBoxLayout width='100%'>
              <Padding/>
              <VBoxLayout width='246px'>
                <Label text='Currency'/>
                <Padding size='12px'/>
                <CurrencySelectionBox className={
                  css(RiskParametersView.STYLE.dropdownButton)}
                  currencyDatabase={this.props.currencyDatabase}
                  value={this.props.parameters.currency} onChange={
                  this.onCurrencyChange}/>
                <Padding size='30px'/>
                <Label text={`Buying Power (${currencySign})`}/>
                <Padding size='12px'/>
                <MoneyInputBox
                  className={css(RiskParametersView.STYLE.inputBox)}
                  value={this.props.parameters.buyingPower}
                  onChange={this.onBuyingPowerChange}/>
                <Padding size='30px'/>
                <Label text={`Net Loss (${currencySign})`}/>
                <Padding size='12px'/>
                <MoneyInputBox
                  className={css(RiskParametersView.STYLE.inputBox)}
                  value={this.props.parameters.netLoss}
                  onChange={this.onNetLossChange}/>
                <Padding size='30px'/>
                <Label text='Transition Time'/>
                <Padding size='12px'/>
                <HBoxLayout width='100%'>
                  <VBoxLayout>
                    <IntegerInputBox min={0} value={hours} padding={2}
                      className={css(RiskParametersView.STYLE.inputBox)}
                      onChange={(value) => this.onTransitionTimeChange(
                        value, TimeUnit.HOURS)}/>
                    <Padding size='10px'/>
                    <span className={
                        css(RiskParametersView.
                          TRANSITION_TIME_STYLE.label)}>
                      HOUR
                    </span>
                  </VBoxLayout>
                  <Padding size='10px'/>
                  <Center height='34px' className={
                      css(RiskParametersView.TRANSITION_TIME_STYLE.colon)}>
                    :
                  </Center>
                  <Padding size='10px'/>
                  <VBoxLayout>
                    <IntegerInputBox min={0} max={59} value={minutes}
                      padding={2}
                      className={css(RiskParametersView.STYLE.inputBox)}
                      onChange={(value) => this.onTransitionTimeChange(
                        value, TimeUnit.MINUTES)}/>
                    <Padding size='10px'/>
                    <span className={
                        css(RiskParametersView.
                          TRANSITION_TIME_STYLE.label)}>
                      MINUTE
                    </span>
                  </VBoxLayout>
                  <Padding size='10px'/>
                  <Center height='34px' className={
                      css(RiskParametersView.TRANSITION_TIME_STYLE.colon)}>
                    :
                  </Center>
                  <Padding size='10px'/>
                  <VBoxLayout>
                    <IntegerInputBox min={0} max={59} value={seconds}
                      padding={2}
                      className={css(RiskParametersView.STYLE.inputBox)}
                      onChange={(value) => this.onTransitionTimeChange(
                        value, TimeUnit.SECONDS)}/>
                    <Padding size='10px'/>
                    <span className={
                        css(RiskParametersView.
                          TRANSITION_TIME_STYLE.label)}>
                      SECOND
                    </span>
                  </VBoxLayout>
                </HBoxLayout>
              </VBoxLayout>
              <Padding/>
            </HBoxLayout>
          </VBoxLayout>
        </VBoxLayout>
        <Padding/>
      </HBoxLayout>);
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

  private onCurrencyChange(value: Nexus.Currency) {
    this.props.parameters.currency = value;
    this.forceUpdate();
  }

  private onBuyingPowerChange(value: Nexus.Money) {
    this.props.parameters.buyingPower = value;
  }

  private onNetLossChange(value: Nexus.Money) {
    this.props.parameters.netLoss = value;
  }

  private onTransitionTimeChange(value: number, timeUnit: TimeUnit) {
    const timeJSON = this.props.parameters.transitionTime.split();
    const newTimeJSON = (() => {
      switch (timeUnit) {
        case TimeUnit.HOURS:
          return {
            hours: value,
            minutes: timeJSON.minutes,
            seconds: timeJSON.seconds
          };
        case TimeUnit.MINUTES:
          return {
            hours: timeJSON.hours,
            minutes: value,
            seconds: timeJSON.seconds
          };
        case TimeUnit.SECONDS:
          return {
            hours: timeJSON.hours,
            minutes: timeJSON.minutes,
            seconds: value
          };
        }
    })();
    this.props.parameters.transitionTime = Beam.Duration.HOUR.multiply(
      newTimeJSON.hours).add(Beam.Duration.MINUTE.multiply(
        newTimeJSON.minutes)).add(Beam.Duration.SECOND.multiply(
        newTimeJSON.seconds));
  }

  private static CONTAINER_STYLE = StyleSheet.create({
    base: {
      position: 'relative' as 'relative'
    },
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
  private static TRANSITION_TIME_STYLE = StyleSheet.create({
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
  private static STYLE = StyleSheet.create({
    innerContainer: {
      position: 'absolute' as 'absolute'
    },
    dropdownButton: {
      backgroundColor: '#F8F8F8',
      border: '1px solid #C8C8C8',
      outline: 0
    },
    inputBox: {
      outline: 0,
      color: '#333333',
      border: '1px solid #C8C8C8',
      ':focus': {
        border: '1px solid #684BC7',
        '-webkit-box-shadow': '0px 0px 1px 0px #684BC7',
        '-moz-box-shadow': '0px 0px 1px 0px #684BC7',
        boxShadow: '0px 0px 1px 0px #684BC7'
      }
    }
  });
}

interface LabelProperties {
  text: string;
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
