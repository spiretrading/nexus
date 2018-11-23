import * as React from 'react';
import { DashboardModel } from './dashboard_model';
import { DashboardPage } from './dashboard_page';
import { SideMenu } from './side_menu';

interface Properties {

  /** The model to use. */
  model: DashboardModel;

  /** Indicates the user has logged out. */
  onLogout?: () => void;
}

interface State {
  isLoaded: boolean;
}

/** Implements the controller for the DashboardPage. */
export class DashboardController extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {
      isLoaded: false
    };
    this.onSideMenuClick = this.onSideMenuClick.bind(this);
  }

  public render(): JSX.Element {
    if(!this.state.isLoaded) {
      return <div/>;
    }
    return <DashboardPage roles={this.props.model.roles}
      onSideMenuClick={this.onSideMenuClick}/>;
  }

  public componentWillMount(): void {
    this.props.model.load().then(
      () => {
        this.setState({isLoaded: true});
      });
  }

  private onSideMenuClick(item: SideMenu.Item) {
    if(item === SideMenu.Item.SIGN_OUT) {
      this.props.model.logout().then(
        () => {
          this.props.onLogout();
        });
    }
  }
}
