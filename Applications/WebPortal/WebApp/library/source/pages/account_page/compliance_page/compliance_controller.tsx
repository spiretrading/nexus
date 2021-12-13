import * as React from 'react';
import { LoadingPage, LoadingState } from '../..';
import { ComplianceModel } from './compliance_model';
import { ComplianceService } from './compliance_service';

interface Properties {

  /** The services used by the CompliancePage. */
  service: ComplianceService;
}

interface State {
  loadingState: LoadingState;
  model: ComplianceModel;
}

/** Implements the controller for the CompliancePage. */
export class ComplianceController extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {
      loadingState: new LoadingState(),
      model: null
    }
  }

  public render(): JSX.Element {
    if(this.state.loadingState.isLoading()) {
      return <LoadingPage/>;
    }
    return <div/>;
  }

  public async componentDidMount(): Promise<void> {
    try {
      const model = await this.props.service.load();
      this.setState((state) => {
        return {
          loadingState: state.loadingState.succeed(),
          model
        };
      });
    } catch(error) {
      this.setState((state) => {
        return {
          loadingState: state.loadingState.fail(error.toString())
        };
      });
    }
  }
}
