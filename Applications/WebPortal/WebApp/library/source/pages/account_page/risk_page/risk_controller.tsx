import * as Nexus from 'nexus';
import * as React from 'react';
import { DisplaySize, LoadingPage, LoadingState } from '../../..';
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
  loadingState: LoadingState;
  comment: string;
  parameters: Nexus.RiskParameters;
  canSubmit: boolean;
  hasSubmissionError: boolean;
  status: string;
}

/** Implements a controller for the RiskPage. */
export class RiskController extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {
      loadingState: new LoadingState(),
      comment: '',
      parameters: null,
      canSubmit: false,
      hasSubmissionError: false,
      status: ''
    };
  }

  public render(): JSX.Element {
    if(this.state.loadingState.isLoading()) {
      return <LoadingPage/>;
    } else if(this.state.loadingState.state === LoadingState.State.ERROR) {
      return <div/>;
    }
    return <RiskPage comment={this.state.comment}
      parameters={this.state.parameters}
      currencyDatabase={this.props.currencyDatabase}
      displaySize={this.props.displaySize} roles={this.props.roles}
      canSubmit={this.state.canSubmit} isError={this.state.hasSubmissionError}
      status={this.state.status} onComment={this.onComment}
      onParameters={this.onParameters} onSubmit={this.onSubmit}/>;  
  }

  public async componentDidMount(): Promise<void> {
    try {
      await this.props.model.load();
      this.setState(state => {
        return {
          loadingState: state.loadingState.succeed(),
          comment: '',
          parameters: this.props.model.riskParameters.clone()
        };
      });
    } catch(error) {
      this.setState(state => {
        return {
          loadingState: state.loadingState.fail(error.toString())
        };
      });
    }
  }

  private onComment = (comment: string) => {
    this.setState({
      canSubmit: true,
      comment
    });
  }

  private onParameters = (parameters: Nexus.RiskParameters) => {
    this.setState({
      canSubmit: true,
      parameters
    });
  }

  private onSubmit = async (
      comment: string, parameters: Nexus.RiskParameters) => {
    try {
      this.setState({
        canSubmit: false,
        hasSubmissionError: false,
        status: ''
      });
      await this.props.model.submit(comment, parameters);
      this.setState({
        status: 'Saved.'
      });
    } catch(e) {
      this.setState({
        hasSubmissionError: true,
        status: e.toString()
      });
    }
  }
}
