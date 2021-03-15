import * as React from 'react';
import * as Router from 'react-router-dom';
import { DisplaySize, LoadingPage, PageNotFoundPage } from '../..';
import { GroupInfoController } from './group_info_page';
import { GroupModel } from './group_model';
import { GroupPage } from './group_page';
import { GroupSubPage } from './group_sub_page';

interface Properties {

  /** Determines the layout to use based on the display device. */
  displaySize: DisplaySize;

  /** The model representing the group to display. */
  model: GroupModel;
}

interface State {
  isLoaded: boolean;
  cannotLoad: boolean;
  redirect: string;
}

/** Implements a controller for the GroupPage. */
export class GroupController extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {
      isLoaded: false,
      cannotLoad: false,
      redirect: null
    };
  }

  public render(): JSX.Element {
    if(this.state.redirect) {
      return <Router.Redirect push to={this.state.redirect}/>;
    }
    if(this.state.cannotLoad) {
      return <PageNotFoundPage displaySize={this.props.displaySize}/>;
    }
    if(!this.state.isLoaded) {
      return <LoadingPage/>;
    }
    const subPage = (() => {
      if(window.location.pathname.endsWith('/group_info')) {
        return GroupSubPage.GROUP;
      } else if(window.location.pathname.endsWith('/compliance')) {
        return GroupSubPage.COMPLIANCE;
      } else if(window.location.pathname.endsWith('/profit_loss')) {
        return GroupSubPage.PROFIT_LOSS;
      }
      return GroupSubPage.NONE;
    })();
    return (
      <GroupPage displaySize={this.props.displaySize}
          subPage={subPage} group={this.props.model.group}
          onMenuClick={this.onMenuClick}>
        <Router.Switch>
          <Router.Route path='/group/:id(\d+)?/group_info'
            render={this.renderGroupInfoPage}/>
          <Router.Route path='/group/:id(\d+)?/compliance'
            render={this.renderCompliancePage}/>
          <Router.Route path='/group/:id(\d+)?/profit_loss'
            render={this.renderProfitLossPage}/>
          <Router.Route path='/group/:id(\d+)?'
            render={({match}: any) => {
              const url = `/group/${match.params.id}/group_info`;
              return <Router.Redirect to={url}/>;
            }}/>
        </Router.Switch>
      </GroupPage>);
  }

  public async componentDidMount(): Promise<void> {
    try {
      await this.props.model.load();
      this.setState({isLoaded: true});
    } catch {
      this.setState({cannotLoad: true});
    }
  }

  public componentDidUpdate(): void {
    if(this.state.redirect) {
      this.setState({redirect: null});
    }
  }

  private parseUrlPrefix = (): string => {
    const url = window.location.pathname;
    const prefix = url.substr(0, url.lastIndexOf('/'));
    if(prefix === '') {
      return url;
    }
    return prefix;
  }

  private renderGroupInfoPage = () => {
    return (
      <GroupInfoController
        displaySize={this.props.displaySize}
        accounts={this.props.model.accounts}/>);
  }

  private renderCompliancePage = () => {
    return <div/>;
  }

  private renderProfitLossPage = () => {
    return <div/>;
  }

  private onMenuClick = (subPage: GroupSubPage) => {
    const urlPrefix = this.parseUrlPrefix();
    if(subPage === GroupSubPage.GROUP) {
      this.setState({redirect: `${urlPrefix}/group_info`});
    } else if(subPage === GroupSubPage.COMPLIANCE) {
      this.setState({redirect: `${urlPrefix}/compliance`});
    } else if(subPage === GroupSubPage.PROFIT_LOSS) {
      this.setState({redirect: `${urlPrefix}/profit_loss`});
    }
  }
}
