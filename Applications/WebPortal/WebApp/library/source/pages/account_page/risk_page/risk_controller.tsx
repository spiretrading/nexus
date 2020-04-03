import * as Nexus from 'nexus';
import * as React from 'react';
import { DisplaySize, LoadingPage } from '../../..';
import { RiskModel } from './risk_model';
import { RiskPage } from './risk_page';

interface Properties {
  
  /** The database of currencies */
  currencyDatabase: Nexus.CurrencyDatabase;

  /** The size at which the component should be displayed at. */
  displaySize: DisplaySize;

  /** The model representing an account's risk settings. */
  model: RiskModel;

  /** The account's roles. */
  roles: Nexus.AccountRoles;
}

interface State {
  isError: boolean;
  isLoaded: boolean;
  status: string;
}

/** Implements a controller for the RiskPage. */
export class RiskController extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {
      isError: false,
      isLoaded: false,
      status: ''
    }
    this.onSubmit = this.onSubmit.bind(this);
  }

  public render(): JSX.Element {
    if(!this.state.isLoaded) {
      return <LoadingPage/>;
    }
    return <RiskPage
      currencyDatabase={this.props.currencyDatabase}
      displaySize={this.props.displaySize}
      parameters={this.props.model.riskParameters}
      roles={this.props.roles}
      isError={this.state.isError}
      status={this.state.status}
      onSubmit={this.onSubmit}/>;  
  }

  public componentDidMount(): void {
    this.props.model.load().then(
      () => {
        this.setState({
          isLoaded: true
      });
    })
  }
  

  private async onSubmit(comment: string, parameters: Nexus.RiskParameters) {
    try {
      this.setState({
        isError: false,
        status: ''
      });
      await this.props.model.submit(comment, parameters);
      this.setState({
        status: 'Saved.'
      });
    } catch(e) {
      this.setState({
        isError: true,
        status: e.toString()
      });
    }
  }
}
