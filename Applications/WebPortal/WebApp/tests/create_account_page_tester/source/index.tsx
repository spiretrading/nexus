import * as Beam from 'beam';
import * as Nexus from 'nexus';
import * as React from 'react';
import * as ReactDOM from 'react-dom';
import * as Router from 'react-router-dom';
import * as WebPortal from 'web_portal';

const countryDatabase = Nexus.buildCountryDatabase();

const DIRECTORY = Beam.DirectoryEntry.Type.DIRECTORY;

const GROUPS = [
  new Beam.DirectoryEntry(DIRECTORY, 80, 'Lonely Mountain'),
  new Beam.DirectoryEntry(DIRECTORY, 41, 'Hobbits of Buckland'),
  new Beam.DirectoryEntry(DIRECTORY, 42, 'Mountains of Mirkwood'),
  new Beam.DirectoryEntry(DIRECTORY, 43, 'Hobbits of Shire'),
  new Beam.DirectoryEntry(DIRECTORY, 45, 'Mountains of Angmar'),
  new Beam.DirectoryEntry(DIRECTORY, 46, 'Hobbits of Bree'),
  new Beam.DirectoryEntry(DIRECTORY, 301, 'spire ontario'),
  new Beam.DirectoryEntry(DIRECTORY, 31, 'Spire United Kingdom'),
  new Beam.DirectoryEntry(DIRECTORY, 32, 'Spire China'),
  new Beam.DirectoryEntry(DIRECTORY, 33, 'Spire Japan'),
  new Beam.DirectoryEntry(DIRECTORY, 34, 'Spire Spain'),
  new Beam.DirectoryEntry(DIRECTORY, 35, 'Spire Singapore'),
  new Beam.DirectoryEntry(DIRECTORY, 302, 'spire quebec'),
  new Beam.DirectoryEntry(DIRECTORY, 37, 'Spire France'),
  new Beam.DirectoryEntry(DIRECTORY, 38, 'Spire Fiji'),
  new Beam.DirectoryEntry(DIRECTORY, 39, 'Spire Sweden'),
  new Beam.DirectoryEntry(DIRECTORY, 303, 'spire alberta'),
  new Beam.DirectoryEntry(DIRECTORY, 304, 'spire yukon'),
  new Beam.DirectoryEntry(DIRECTORY, 305, 'spire manitoba'),
  new Beam.DirectoryEntry(DIRECTORY, 306, 'spire nova scotia'),
  new Beam.DirectoryEntry(DIRECTORY, 307, 'spire new brunswick'),
  new Beam.DirectoryEntry(DIRECTORY, 308, 'spire british columbia'),
  new Beam.DirectoryEntry(DIRECTORY, 309, 'spire saskatchewan')
];

const CONFLICT = 409;

enum TestBehavior {
  SUCCEED,
  DUPLICATE_NAME,
  UNAVAILABLE,
  HANG
}

class TestModel extends WebPortal.CreateAccountModel {
  constructor(delay: number) {
    super();
    this.delay = delay;
    this.behavior = TestBehavior.SUCCEED;
  }

  public setBehavior(behavior: TestBehavior): void {
    this.behavior = behavior;
  }

  public async createAccount(username: string, groups: Beam.DirectoryEntry,
      identity: Nexus.AccountIdentity,
      roles: Nexus.AccountRoles): Promise<void> {
    if(this.behavior === TestBehavior.HANG) {
      await new Promise<void>(() => {});
    }
    await new Promise(resolve => setTimeout(resolve, this.delay));
    if(this.behavior === TestBehavior.DUPLICATE_NAME) {
      throw new Beam.ServiceError('Account already exists.', CONFLICT);
    } else if(this.behavior === TestBehavior.UNAVAILABLE) {
      throw new Beam.ServiceError('Server unreachable.');
    }
  }

  private delay: number;
  private behavior: TestBehavior;
}

const model = new TestModel(1000);
const groupSuggestionModel = new WebPortal.LocalGroupSuggestionModel(GROUPS);

interface Properties {
  displaySize: WebPortal.DisplaySize;
}

interface State {
  behavior: TestBehavior;
}

/**  Displays and tests the CreateAccountPage. */
class TestApp extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {
      behavior: TestBehavior.SUCCEED
    };
  }

  public render(): JSX.Element {
    return (
      <Router.BrowserRouter>
        <div style={STYLE.wrapper}>
          <div style={STYLE.toolbar}>
            <span style={STYLE.toolbarLabel}>Behavior:</span>
            {this.renderBehaviorButton('Succeed', TestBehavior.SUCCEED)}
            {this.renderBehaviorButton(
              'Duplicate name', TestBehavior.DUPLICATE_NAME)}
            {this.renderBehaviorButton('Unavailable', TestBehavior.UNAVAILABLE)}
            {this.renderBehaviorButton('Hang', TestBehavior.HANG)}
          </div>
          <Router.Switch>
            <Router.Route exact path='/account_directory'>
              <div style={STYLE.done}>
                Account created. Go back to submit another.
              </div>
            </Router.Route>
            <Router.Route>
              <WebPortal.CreateAccountController
                displaySize={this.props.displaySize}
                countryDatabase={countryDatabase}
                createAccountModel={model}
                groupSuggestionModel={groupSuggestionModel}/>
            </Router.Route>
          </Router.Switch>
        </div>
      </Router.BrowserRouter>);
  }

  private renderBehaviorButton(label: string,
      behavior: TestBehavior): JSX.Element {
    const isActive = this.state.behavior === behavior;
    return (
      <button key={label}
          style={{...STYLE.button, ...(isActive && STYLE.buttonActive)}}
          onClick={() => {
            model.setBehavior(behavior);
            this.setState({behavior});
          }}>
        {label}
      </button>);
  }
}

const STYLE: Record<string, React.CSSProperties> = {
  wrapper: {
    width: '100%',
    height: '100%',
    display: 'flex',
    flexDirection: 'column'
  },
  toolbar: {
    display: 'flex',
    alignItems: 'center',
    gap: '6px',
    padding: '8px 12px',
    backgroundColor: '#F0F0F0',
    borderBottom: '1px solid #DDD'
  },
  toolbarLabel: {
    fontSize: '12px',
    fontWeight: 600,
    fontFamily: 'monospace'
  },
  button: {
    fontSize: '11px',
    fontFamily: 'monospace',
    padding: '2px 6px',
    borderWidth: '1px',
    borderStyle: 'solid',
    borderColor: '#ccc',
    borderRadius: '3px',
    backgroundColor: '#fff',
    cursor: 'pointer'
  },
  buttonActive: {
    backgroundColor: '#684BC7',
    color: '#fff',
    borderColor: '#684BC7'
  },
  done: {
    padding: '18px',
    font: '400 14px Roboto'
  }
};

const ResponsivePage = WebPortal.displaySizeRenderer(TestApp);
ReactDOM.render(<ResponsivePage/>, document.getElementById('main'));
