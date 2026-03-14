import { css, StyleSheet } from 'aphrodite';
import { Center, HBoxLayout, Padding, VBoxLayout } from 'dali';
import * as Beam from 'beam';
import * as Nexus from 'nexus';
import * as React from 'react';
import { CurrencySelect, IntegerInput, MoneyInput } from '../../..';

interface Properties {

  /** The parameters to display. */
  parameters: Nexus.RiskParameters;

  /** Used to lookup currency names and symbols. */
  currencyDatabase: Nexus.CurrencyDatabase;

  /**
   * Callback for when the parameters are changed.
   * @param parameters - The new parameters.
   */
  onChange?: (parameters: Nexus.RiskParameters) => void;
}

/** Implements a React component to display a set of RiskParameters. */
export class RiskParametersView extends React.Component<Properties> {
  public render(): JSX.Element {
    const splitTransitionTime = this.props.parameters.transitionTime.split();
    const currencySign = this.props.currencyDatabase.fromCurrency(
      this.props.parameters.currency).sign;
    return (
      <HBoxLayout width='100%'>
        <Padding/>
        <VBoxLayout width='246px'>
          <Label text='Currency'/>
          <Padding size='12px'/>
          <CurrencySelect className={
            css(RiskParametersView.STYLE.dropdownButton)}
            currencyDatabase={this.props.currencyDatabase}
            value={this.props.parameters.currency}
            onChange={this.onCurrencyChange}/>
          <Padding size='30px'/>
          <Label text={`Buying Power (${currencySign})`}/>
          <Padding size='12px'/>
          <MoneyInput
            className={css(RiskParametersView.STYLE.inputBox)}
            value={this.props.parameters.buyingPower}
            min={Nexus.Money.ZERO}
            onChange={this.onBuyingPowerChange}/>
          <Padding size='30px'/>
          <Label text={`Net Loss (${currencySign})`}/>
          <Padding size='12px'/>
          <MoneyInput
            className={css(RiskParametersView.STYLE.inputBox)}
            value={this.props.parameters.netLoss}
            min={Nexus.Money.ZERO}
            onChange={this.onNetLossChange}/>
          <Padding size='30px'/>
          <Label text='Transition Time'/>
          <Padding size='12px'/>
          <HBoxLayout width='100%'>
            <VBoxLayout>
              <IntegerInput min={0} value={splitTransitionTime.hours}
                leadingZeros={2} className={
                  css(RiskParametersView.STYLE.inputBox)}
                onChange={this.onHoursChange}/>
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
              <IntegerInput min={0} max={59} value={
                splitTransitionTime.minutes} leadingZeros={2}
                className={css(RiskParametersView.STYLE.inputBox)}
                onChange={this.onMinutesChange}/>
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
              <IntegerInput min={0} max={59} value={
                splitTransitionTime.seconds} leadingZeros={2}
                className={css(RiskParametersView.STYLE.inputBox)}
                onChange={this.onSecondsChange}/>
              <Padding size='10px'/>
              <span className={
                  css(RiskParametersView.TRANSITION_TIME_STYLE.label)}>
                SECOND
              </span>
            </VBoxLayout>
          </HBoxLayout>
        </VBoxLayout>
        <Padding/>
      </HBoxLayout>);
  }

  private onCurrencyChange = (value: Nexus.Currency) => {
    const newParameters = this.props.parameters.clone();
    newParameters.currency = value;
    this.props.onChange?.(newParameters);
  }

  private onBuyingPowerChange = (value: Nexus.Money) => {
    const newParameters = this.props.parameters.clone();
    newParameters.buyingPower = value;
    this.props.onChange?.(newParameters);
  }

  private onNetLossChange = (value: Nexus.Money) => {
    const newParameters = this.props.parameters.clone();
    newParameters.netLoss = value;
    this.props.onChange?.(newParameters);
  }

  private onHoursChange = (value: number) => {
    this.onTransitionTimeChange({hours: value});
  }

  private onMinutesChange = (value: number) => {
    this.onTransitionTimeChange({minutes: value});
  }

  private onSecondsChange = (value: number) => {
    this.onTransitionTimeChange({seconds: value});
  }

  private onTransitionTimeChange(
      override: Partial<{hours: number; minutes: number; seconds: number}>) {
    const time = {
      ...this.props.parameters.transitionTime.split(),
      ...override
    };
    const newParameters = this.props.parameters.clone();
    newParameters.transitionTime = Beam.Duration.HOUR.multiply(
      time.hours).add(Beam.Duration.MINUTE.multiply(
      time.minutes)).add(Beam.Duration.SECOND.multiply(
      time.seconds));
    this.props.onChange?.(newParameters);
  }

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
        boxShadow: '0px 0px 1px 0px #684BC7'
      }
    }
  });
}

function Label(props: {text: string}): JSX.Element {
  return (
    <HBoxLayout width='100%'>
      <span className={css(LABEL_STYLE.text)}>
        {props.text}
      </span>
      <Padding/>
    </HBoxLayout>);
}

const LABEL_STYLE = StyleSheet.create({
  text: {
    font: '400 14px Roboto',
    color: '#333333',
    whiteSpace: 'nowrap'
  }
});
