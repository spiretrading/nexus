import {css, StyleSheet} from 'aphrodite';
import * as Beam from 'beam';
import * as Nexus from 'nexus';
import * as React from 'react';
import * as ReactDOM from 'react-dom';
import * as WebPortal from 'web_portal';

interface State {
  isAdmin: boolean;
  footerMessage: FOOTER_MESSAGE;
  parameters: Nexus.RiskParameters;
}

interface Properties {

}

enum FOOTER_MESSAGE {
  SAVED,
  SUBMITTED,
  SERVER_ISSUE,
  NONE
}

class TestApp extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {
      isAdmin: false,
      footerMessage: FOOTER_MESSAGE.NONE,
      parameters: new Nexus.RiskParameters(
        Nexus.DefaultCurrencies.CAD,
        Nexus.Money.ONE.multiply(100000),
        new Nexus.RiskState(Nexus.RiskState.Type.ACTIVE),
        Nexus.Money.ONE.multiply(1000), 100,
        Beam.Duration.HOUR.multiply(5).add(Beam.Duration.MINUTE.multiply(30)).add(
        Beam.Duration.SECOND.multiply(15)))
    };
    this.onChangeFooterMessage = this.onChangeFooterMessage.bind(this);
    this.onToggleIsAdmin = this.onToggleIsAdmin.bind(this);
    this.onCurrencyChange = this.onCurrencyChange.bind(this);
    this.onBuyingPowerChange = this.onBuyingPowerChange.bind(this);
    this.onNetLossChange = this.onNetLossChange.bind(this);
    this.onTransitionTimeChange = this.onTransitionTimeChange.bind(this);
    this.onSubmitRequest = this.onSubmitRequest.bind(this);
    this.onSaveChanges = this.onSaveChanges.bind(this);
  }

  public render(): JSX.Element {
    const adminButtonMessage = (() => {
      if(this.state.isAdmin) {
        return 'Is Admin';
      }
      return 'Not Admin';
    })()
    return (
      <WebPortal.VBoxLayout width='100%' height='100%'>
        <WebPortal.HBoxLayout className={css(TestApp.STYLE.controlPanel)}>
          <button className={css(TestApp.STYLE.controlButton)} onClick={
              this.onChangeFooterMessage}>
            Change Footer Message
          </button>
          <WebPortal.Padding size='30px'/>
          <button className={css(TestApp.STYLE.controlButton)} onClick={
              this.onToggleIsAdmin}>
            {adminButtonMessage}
          </button>
        </WebPortal.HBoxLayout>
        <WebPortal.RiskParametersView parameters={this.state.parameters}
          currencyDatabase={Nexus.buildDefaultCurrencyDatabase()}
          footerMessage={this.state.footerMessage}
          isUserAdmin={this.state.isAdmin}
          onCurrencyChange={this.onCurrencyChange}
          onBuyingPowerChange={this.onBuyingPowerChange}
          onNetLossChange={this.onNetLossChange}
          onTransitionTimeChange={this.onTransitionTimeChange}
          onSubmitRequest={this.onSubmitRequest}
          onSaveChanges={this.onSaveChanges}/>
      </WebPortal.VBoxLayout>);
  }

  private onToggleIsAdmin() {
    this.setState({isAdmin: !this.state.isAdmin});
  }

  private onChangeFooterMessage() {
    const next = (() => {
      switch(this.state.footerMessage) {
      case FOOTER_MESSAGE.SAVED:
        return FOOTER_MESSAGE.SERVER_ISSUE;
      case FOOTER_MESSAGE.SERVER_ISSUE:
        return FOOTER_MESSAGE.SUBMITTED;
      case FOOTER_MESSAGE.SUBMITTED:
        return FOOTER_MESSAGE.NONE;
      case FOOTER_MESSAGE.NONE:
        return FOOTER_MESSAGE.SAVED;
      }
    })();
    this.setState({footerMessage: next});
  }

  private onCurrencyChange(value: Nexus.Currency) {
    const newParameters = this.state.parameters;
    newParameters.currency = value;
    this.setState({parameters: newParameters})
  }

  private onBuyingPowerChange(value: Nexus.Money) {
    const newParameters = this.state.parameters;
    newParameters.buyingPower = value;
    this.setState({parameters: newParameters})
  }

  private onNetLossChange(value: Nexus.Money) {
    const newParameters = this.state.parameters;
    newParameters.netLoss = value;
    this.setState({parameters: newParameters})
  }

  private onTransitionTimeChange(value: Beam.Duration) {
    const newParameters = this.state.parameters;
    newParameters.transitionTime = value;
    this.setState({parameters: newParameters})
  }

  private onSubmitRequest(comment: string) {
    console.log('submitting with request with comment: ',
      comment, ' parameters: ', this.state.parameters);
  }

  private onSaveChanges() {
    console.log('saving changes with parameters: ',
      this.state.parameters);
  }

  private static STYLE = StyleSheet.create({
    controlPanel: {
      position: 'absolute' as 'absolute',
      top: 700,
      left: 0,
      width: '100%',
    },
    controlButton: {
      width: '100px',
      height: '30px'
    }
  });
}

ReactDOM.render(<TestApp/>, document.getElementById('main'));
