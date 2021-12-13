import * as React from 'react';
import { DisplaySize } from '../../..';
import { LoadingPage, LoadingState } from '../..';
import { ComplianceModel } from './compliance_model';
import { CompliancePage } from './compliance_page';
import { ComplianceService } from './compliance_service';

interface Properties {

  /** The size at which the component should be displayed at. */
  displaySize: DisplaySize;

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
    } else if(this.state.loadingState.state === LoadingState.State.ERROR) {
      return <div/>;
    }
    return <CompliancePage displaySize={this.props.displaySize}
      model={this.state.model}/>;
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
