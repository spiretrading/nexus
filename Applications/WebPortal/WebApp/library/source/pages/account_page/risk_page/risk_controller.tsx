import * as Beam from 'beam';
import * as Nexus from 'nexus';
import * as React from 'react';
import { LoadingPage, LoadingState } from '../../..';
import { RiskModel } from './risk_model';
import { RiskPage } from './risk_page';

interface Properties {

  /** The database of currencies */
  currencyDatabase: Nexus.CurrencyDatabase;

  /** The model representing an account's risk settings. */
  model: RiskModel;

  /** The account's roles. */
  roles: Nexus.AccountRoles;
}

interface State {
  loadingState: LoadingState;
  comment: string;
  effectiveDate?: Beam.Date;
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
      effectiveDate: Beam.Date.today(),
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
      return <RiskPage comment='' parameters={Nexus.RiskParameters.INVALID}
        currencyDatabase={this.props.currencyDatabase}
        roles={this.props.roles}
        isError status='Server issue'/>;
    }
    const hasDateError = this.state.effectiveDate == null;
    return <RiskPage comment={this.state.comment}
      parameters={this.state.parameters}
      effectiveDate={this.state.effectiveDate}
      currencyDatabase={this.props.currencyDatabase}
      roles={this.props.roles}
      canSubmit={this.state.canSubmit && !hasDateError}
      dateError={hasDateError}
      isError={this.state.hasSubmissionError}
      status={this.state.status} onComment={this.onComment}
      onEffectiveDate={this.onEffectiveDate}
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
    this.setState({comment});
  }

  private onEffectiveDate = (effectiveDate?: Beam.Date) => {
    this.setState({effectiveDate});
  }

  private onParameters = (parameters: Nexus.RiskParameters) => {
    this.setState({
      canSubmit: true,
      parameters
    });
  }

  private onSubmit = async () => {
    try {
      this.setState({
        canSubmit: false,
        hasSubmissionError: false,
        status: ''
      });
      await this.props.model.submit(this.state.comment,
        this.state.parameters, this.state.effectiveDate ?? Beam.Date.today());
      this.setState({
        status: 'Saved'
      });
    } catch(e) {
      this.setState({
        canSubmit: true,
        hasSubmissionError: true,
        status: 'Server issue'
      });
    }
  }
}
