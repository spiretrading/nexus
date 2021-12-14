import * as Nexus from 'nexus';
import * as React from 'react';
import { DisplaySize } from '../../..';
import { LoadingPage, LoadingState } from '../..';
import { ComplianceModel } from './compliance_model';
import { CompliancePage } from './compliance_page';
import { ComplianceService } from './compliance_service';

interface Properties {

  /** The size at which the component should be displayed at. */
  displaySize: DisplaySize;

  /** The roles belonging to the account viewing this page. */
  roles: Nexus.AccountRoles;

  /** The services used by the CompliancePage. */
  service: ComplianceService;
}

interface State {
  loadingState: LoadingState;
  model: ComplianceModel;
  hasChange: boolean;
  hasError: boolean;
  status: string;
}

/** Implements the controller for the CompliancePage. */
export class ComplianceController extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {
      loadingState: new LoadingState(),
      model: null,
      hasChange: false,
      hasError: false,
      status: ''
    }
  }

  public render(): JSX.Element {
    if(this.state.loadingState.isLoading()) {
      return <LoadingPage/>;
    } else if(this.state.loadingState.state === LoadingState.State.ERROR) {
      return <div/>;
    }
    return <CompliancePage displaySize={this.props.displaySize}
      roles={this.props.roles} model={this.state.model}
      isError={this.state.hasError} status={this.state.status}
      hasChange={this.state.hasChange} onRuleAdd={this.onRuleAdd}
      onRuleChange={this.onRuleChange} onSubmit={this.onSubmit}/>;
  }

  public async componentDidMount(): Promise<void> {
    try {
      const model = await this.props.service.load();
      this.setState(state => {
        return {
          loadingState: state.loadingState.succeed(),
          model
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

  private onRuleAdd = (newSchema: Nexus.ComplianceRuleSchema) => {
    this.setState(state => {
      state.model.add(newSchema);
      return {
        model: state.model,
        hasChange: true
      };
    });
  }

  private onRuleChange = (updatedRule: Nexus.ComplianceRuleEntry) => {
    this.setState(state => {
      state.model.update(updatedRule);
      return {
        model: state.model,
        hasChange: true
      };
    });
  }

  private onSubmit = async () => {
    try {
      const model = await this.props.service.submit(this.state.model);
      this.setState(
        {
          hasError: false,
          status: '',
          model
        });
    } catch(error) {
      this.setState(
        {
          hasError: true,
          status: error.toString()
        });
    }
  }
}
