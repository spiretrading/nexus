import * as Nexus from 'nexus';
import * as React from 'react';
import * as ReactDOM from 'react-dom';
import * as WebPortal from 'web_portal';

interface State {
  displaySize: WebPortal.DisplaySize;
  lastNameValue: string;
  someRoles: Nexus.AccountRoles;
}

/**  Displays a testing application. */
class TestApp extends React.Component<{}, State> {
  constructor(props: {}) {
    super(props);
    this.state = {
      displaySize: WebPortal.DisplaySize.getDisplaySize(),
      lastNameValue: 'Grey',
      someRoles: new Nexus.AccountRoles()
    };
    this.onScreenResize = this.onScreenResize.bind(this);
    this.onTextInput = this.onTextInput.bind(this);
    this.onRoleClick = this.onRoleClick.bind(this);
  }

  public render(): JSX.Element {
    const orientation = (() => {
      if(this.state.displaySize === WebPortal.DisplaySize.SMALL) {
        return WebPortal.FormEntry.Orientation.VERTICAL;
      } else {
        return WebPortal.FormEntry.Orientation.HORIZONTAL;
      }
    })();
    return (
    <WebPortal.HBoxLayout>
      <WebPortal.Padding size='12px'/>
      <WebPortal.VBoxLayout>
        <WebPortal.Padding size='30px'/>
        <WebPortal.FormEntry name='First Name'
            readonly
            orientation={orientation}>
          <WebPortal.TextField
            value = 'Gandalf'
            displaySize={this.state.displaySize}
            disabled/>
        </WebPortal.FormEntry>
        <WebPortal.Padding size='30px'/>
        <WebPortal.FormEntry name='Last Name'
            orientation={orientation}>
          <WebPortal.TextField
            displaySize={this.state.displaySize}
            value={this.state.lastNameValue}
            onInput={this.onTextInput}/>
        </WebPortal.FormEntry>
        <WebPortal.Padding size='30px'/>
        <WebPortal.RolesField roles={this.state.someRoles}
          onClick={this.onRoleClick}/>
        <WebPortal.Padding size='30px'/>
        <WebPortal.FormEntry name='Nickname'
            orientation={orientation}>
          <WebPortal.TextField
            displaySize={this.state.displaySize}
            value='Stormcrow'
            disabled/>
        </WebPortal.FormEntry>
      </WebPortal.VBoxLayout>
      <WebPortal.Padding/>
    </WebPortal.HBoxLayout>);
  }

  public componentDidMount(): void {
    window.addEventListener('resize', this.onScreenResize);
  }

  public componentWillUnmount(): void {
    window.removeEventListener('resize', this.onScreenResize);
  }

  private onScreenResize() {
    const newDisplaySize = WebPortal.DisplaySize.getDisplaySize();
    if(newDisplaySize !== this.state.displaySize) {
      this.setState({ displaySize: newDisplaySize });
    }
  }

  private onTextInput(value: string) {
    this.setState({
      lastNameValue: value
    });
  }

  private onRoleClick(role: Nexus.AccountRoles.Role) {
    if(this.state.someRoles.test(role)) {
      this.state.someRoles.unset(role);
    } else {
      this.state.someRoles.set(role);
    }
    this.forceUpdate();
  }
}

ReactDOM.render(<TestApp/>, document.getElementById('main'));
